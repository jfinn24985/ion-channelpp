#ifndef IONCH_EVALUATOR_EVALUATOR_META_HPP
#define IONCH_EVALUATOR_EVALUATOR_META_HPP

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
#include "core/input_base_meta.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <cstddef>

namespace evaluator { class base_evaluator; } 
namespace core { class input_parameter_memo; } 
namespace evaluator { class evaluator_manager; } 
namespace core { class input_help; } 
namespace core { class input_base_reader; } 

namespace evaluator {

class evaluator_definition : public core::input_definition
 {
   public:
    typedef boost::function1<std::unique_ptr< base_evaluator >, std::vector< core::input_parameter_memo > const&> evaluator_generator_fn;


   private:
    // A method for generating evaluator objects.
    evaluator_generator_fn factory_;


   public:
    // Main Ctor
    //
    // \param label : name of the evaluator subtype.
    evaluator_definition(std::string label, std::string desc, evaluator_generator_fn fn)
    : input_definition( label, desc )
    , factory_( fn )
    {}


   private:
    evaluator_definition() = delete;


   public:
    virtual ~evaluator_definition() {}


   private:
    evaluator_definition(evaluator_definition && source) = delete;

    evaluator_definition(const evaluator_definition & source) = delete;

    evaluator_definition & operator=(const evaluator_definition & source) = delete;


   public:
    // Generate a sampler from the given label and paramneters and
    // add to the report manager.
    
    std::unique_ptr< base_evaluator > operator()(const std::vector< core::input_parameter_memo >& params) const
    {
      return this->factory_( params );
    }

using input_definition::add_definition;
using input_definition::empty;
using input_definition::has_definition;
using input_definition::label;
using input_definition::publish_help;
using input_definition::size;

};
class evaluator_meta : public core::input_base_meta
 {
   private:
    // Where to put the evaluator objects generated from the input file.
    boost::shared_ptr< evaluator_manager > manager_;

    //The type specific name/value pairs
    std::vector<core::input_parameter_memo> parameter_set_;

    // Evaluator factories
    boost::ptr_vector< evaluator_definition > type_to_object_;

    //The type-label of the evaluator to construct
    std::string type_;


   public:
    evaluator_meta(boost::shared_ptr< evaluator_manager >& eman);

    virtual ~evaluator_meta();

    // Add ctor for input "evaluator" section with "type = [defn.label]" 
    //
    // \pre not has_definition( defn.label )
    // \post has_definition( defn.label )
    void add_definition(std::unique_ptr< evaluator_definition > & defn);

    // Add ctor for input "evaluator" section with "type = [defn.label]" 
    //
    // \pre not has_definition( defn.label )
    // \post has_definition( defn.label )
    void add_definition(std::unique_ptr< evaluator_definition > && defn);

    // Get a list of definition labels/keys
    std::vector< std::string > definition_key_list();

    // Ary there no evaluator definitions?
    bool empty() const { return this->type_to_object_.empty(); }
    

    // Get definition of evaluator that matches the label
    const evaluator_definition& get_definition(std::string label) const;

    // Is there a ctor to match input "sampler" section with
    // "type = [label]"
    bool has_definition(std::string label) const;

    void publish_help(core::input_help & helper) const override;


   private:
    //Read an entry in the input file. Return true if the entry was processed.
    //
    //throw an error if input file is incorrect (using UTILITY_INPUT macro)
    virtual bool do_read_entry(core::input_base_reader & reader) override;

    // Perform checks at the end of reading a section.
    virtual void do_read_end(const core::input_base_reader & reader) override;

    // Reset internal data.
    virtual void do_reset() override;


   public:
    // The number of evaluator definitions
    std::size_t size() const { return this->type_to_object_.size(); }
    


};

} // namespace evaluator
#endif
