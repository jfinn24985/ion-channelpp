#ifndef IONCH_TRIAL_CHOICE_META_HPP
#define IONCH_TRIAL_CHOICE_META_HPP

//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------

#include "core/input_definition.hpp"
#include <boost/function.hpp>

#include "utility/unique_ptr.hpp"
#include <boost/serialization/vector.hpp>
#include <string>
#include "core/input_parameter_memo.hpp"
#include "core/input_base_meta.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include "utility/bitset.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <cstddef>

namespace trial { class base_chooser; } 
namespace trial { class choice_manager; } 
namespace core { class input_help; } 
namespace core { class input_base_reader; } 

namespace trial {

// Provide information useful for interpreting the input file. This includes
// a chooser generator function, a choice type name and the optional
// parameters (with help descriptions).
class choice_definition : private core::input_definition
{
   public:
    typedef boost::function4<std::unique_ptr< base_chooser >, const std::vector< core::input_parameter_memo > &, std::string, core::input_parameter_memo, double> choice_generator_fn;


   private:
    choice_generator_fn factory_;


   public:
    // Main Ctor
    //
    // \param label : name of the region subtype.
    choice_definition(std::string label, std::string desc, choice_generator_fn fn)
    : input_definition( label, desc )
    , factory_( fn )
    {}


   private:
    choice_definition() = delete;


   public:
    virtual ~choice_definition() {}


   private:
    choice_definition(choice_definition && source) = delete;

    choice_definition(const choice_definition & source) = delete;

    choice_definition & operator=(const choice_definition & source) = delete;


   public:
    // Generate a base_region from the given label 
    // and parameters.
    std::unique_ptr< base_chooser > operator()(const std::vector< core::input_parameter_memo > & params, std::string label, const core::input_parameter_memo & specie_list, double rate) const
    {
      return this->factory_( params, label, specie_list, rate );
    }

using input_definition::add_definition;
using input_definition::empty;
using input_definition::has_definition;
using input_definition::label;
using input_definition::description;
using input_definition::publish_help;
using input_definition::size;

};
// Interpreter of choice definitions in the input file.
//
// Selecting choice types for different simulation types is
// done by setting appropriate functors in the type_to_object
// map through the add_trial_type method.
//
// It is anticipated that only one of these objects will be 
// created. Creating multiple objects of this type is undefined.
class choice_meta : public core::input_base_meta
 {
   public:
    enum
     {
      CHOICE_RATE = 0,// Trial choice relative rate
      CHOICE_TYPE,// Choice type
      CHOICE_TAG_COUNT = 2// The number of required tags  in choice input section.

    };


   private:
    //The trial generation object
    boost::shared_ptr< choice_manager > manager_;

    // Non-standard parameters of the currently processing trial section.
    std::vector< core::input_parameter_memo > parameter_set_;

    // Flags to check all options have been seen in input file. 
    // ("type" and "rate")
    std::bitset<CHOICE_TAG_COUNT> missing_required_tags_;

    //The trial "rate" of the currently processing trial section.
    double rate_;

    // (optional) List of allowed or disallowed species.
    core::input_parameter_memo specie_list_;

    boost::ptr_vector< choice_definition > type_to_object_;

    //The trial "type" parameter value of the currently processing section.
    std::string type_;


   public:
    // 
    choice_meta(boost::shared_ptr< choice_manager > man);

    virtual ~choice_meta();

    // Add ctor for input "trial" section
    //
    // \pre not has_trial_type( ctor.type_name_ )
    // \post has_trial_type( ctor.type_name_ )
    void add_trial_type(std::unique_ptr< choice_definition >& ctor);

    // Get a list of definition labels/keys
    std::vector< std::string > definition_key_list();

    // Ary there no evaluator definitions?
    bool empty() const { return this->type_to_object_.empty(); }
    

    // Is there a ctor to match input "trial" section with
    // "type = [trial_label]"
    bool has_trial_type(std::string trial_label);

    // 
    void publish_help(core::input_help & helper) const override;


   private:
    // Read an entry in the input file. Return true if the entry was processed.
    //
    // Throws an error if input file is incorrect (using UTILITY_INPUT macro)
    virtual bool do_read_entry(core::input_base_reader & reader) override;

    // Perform checks at the end of reading a section. Then create a chooser
    // object and add it to the choice manager. Resets the
    // meta object state ready for the next 'trial' input section.
    //
    // Throws an error if input file is incorrect (using UTILITY_INPUT macro)
    virtual void do_read_end(const core::input_base_reader & reader) override;

    // Reset the object state on entry into read_seaction. This
    // provides a stronger exception safety if the delegate is
    // reused after raising an exception in read_section.
    virtual void do_reset() override;


   public:
    // The number of evaluator definitions
    std::size_t size() const { return this->type_to_object_.size(); }
    


};

} // namespace trial
#endif
