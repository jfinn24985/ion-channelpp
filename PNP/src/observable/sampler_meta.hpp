#ifndef IONCH_OBSERVABLE_SAMPLER_META_HPP
#define IONCH_OBSERVABLE_SAMPLER_META_HPP

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

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <string>
#include "core/input_base_meta.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "utility/unique_ptr.hpp"
#include <cstddef>

namespace observable { class base_observable; } 
namespace core { class input_parameter_memo; } 
namespace observable { class tracked_observable; } 
namespace observable { class report_manager; } 
namespace core { class input_help; } 
namespace core { class input_base_reader; } 

namespace observable {

// Information about a sample type to be used to instantiate
// a sample from an input file.
class sampler_definition : private core::input_definition
 {
   public:
    typedef boost::function1<boost::shared_ptr< observable::base_observable >, std::vector< core::input_parameter_memo > const&> sampler_generator_fn;


   private:
    // A method for generating sampler objects.
    sampler_generator_fn factory_;


   public:
    // Main Ctor
    //
    // \param label : name of the sampler subtype.
    // \param desc : description of the sampler subtype.
    // \param fn : factory/ctor for the subtype.
    sampler_definition(std::string label, std::string desc, sampler_generator_fn fn)
    : input_definition( label, desc )
    , factory_( fn )
    {}


   private:
    sampler_definition() = delete;


   public:
    virtual ~sampler_definition() {}


   private:
    sampler_definition(sampler_definition && source) = delete;

    sampler_definition(const sampler_definition & source) = delete;

    sampler_definition & operator=(const sampler_definition & source) = delete;


   public:
    // Generate a sampler from the given label and paramneters and
    // add to the report manager.
    
    boost::shared_ptr< base_observable > operator()(const std::vector< core::input_parameter_memo > & params) const
    {
      return this->factory_( params );
    }

using input_definition::add_definition;
using input_definition::empty;
using input_definition::has_definition;
using input_definition::label;
using input_definition::description;
using input_definition::publish_help;
using input_definition::size;

};
// Information about a sample type to be used to instantiate
// a sample from an input file.
class tracked_definition : private core::input_definition
 {
   public:
    typedef boost::function1<boost::shared_ptr< observable::tracked_observable >, std::vector< core::input_parameter_memo > const&> tracked_generator_fn;


   private:
    // A method for generating sampler objects.
    tracked_generator_fn factory_;


   public:
    // Main Ctor
    //
    // \param label : name of the region subtype.
    tracked_definition(std::string label, std::string desc, tracked_generator_fn fn)
    : input_definition( label, desc )
    , factory_( fn )
    {}


   private:
    tracked_definition() = delete;


   public:
    virtual ~tracked_definition() {}


   private:
    tracked_definition(tracked_definition && source) = delete;

    tracked_definition(const tracked_definition & source) = delete;

    tracked_definition & operator=(const tracked_definition & source) = delete;


   public:
    // Generate a sampler from the given label and paramneters and
    // add to the report manager.
    
    boost::shared_ptr< tracked_observable > operator()(const std::vector< core::input_parameter_memo > & params) const
    {
      return this->factory_( params );
    }

using input_definition::add_definition;
using input_definition::empty;
using input_definition::has_definition;
using input_definition::label;
using input_definition::description;
using input_definition::publish_help;
using input_definition::size;

};
class sampler_meta : public core::input_base_meta
 {
   private:
    std::vector<core::input_parameter_memo> parameter_set_;

    boost::shared_ptr< report_manager > manager_;

    // Map input file sampler type label to corresponding object
    // factories
    boost::ptr_vector< sampler_definition > type_to_sample_;

    boost::ptr_vector< tracked_definition > type_to_tracked_;

    //The type label of the current observable
    std::string type_;


   public:
    sampler_meta(boost::shared_ptr< report_manager > manager);

    virtual ~sampler_meta();

    // Add ctor for input "trial" section with "type = [trial_label]" 
    //
    // \pre not has_trial_type( defn.label )
    // \post has_trial_type( defn.label )
    void add_sampler_type(std::unique_ptr< sampler_definition > & defn);

    // Add ctor for input "trial" section with "type = [trial_label]" 
    //
    // \pre not has_trial_type( defn.label )
    // \post has_trial_type( defn.label )
    void add_tracked_type(std::unique_ptr< tracked_definition > & defn);

    // Get a list of definition labels/keys
    std::vector< std::string > definition_key_list();

    // Ary there no evaluator definitions?
    bool empty() const { return this->type_to_sample_.empty() and this->type_to_tracked_.empty(); }
    

    // Is there a ctor to match input "sampler" section with
    // "type = [label]"
    bool has_type(std::string label);

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
    std::size_t size() const { return this->type_to_sample_.size() + this->type_to_tracked_.size(); }
    


};

} // namespace observable
#endif
