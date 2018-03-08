#ifndef IONCH_PLATFORM_STORAGE_META_HPP
#define IONCH_PLATFORM_STORAGE_META_HPP

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
#include <string>
#include "core/input_base_meta.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <boost/serialization/vector.hpp>
#include "utility/unique_ptr.hpp"
#include <cstddef>

namespace platform { class storage_manager; } 
namespace core { class input_parameter_memo; } 
namespace core { class input_base_reader; } 
namespace core { class input_help; } 

namespace platform {

// Provide information useful for interpreting the input file. This includes
// a storage_manager generator function, a type name and any optional
// parameters (with help descriptions).
class storage_definition : private core::input_definition
{
   public:
    // Main Ctor
    //
    // \param label : name of the storage manager subtype.
    storage_definition(std::string label, std::string desc)
    : input_definition( label, desc )
    {}


using input_definition::add_definition;
using input_definition::empty;
using input_definition::has_definition;
using input_definition::label;
using input_definition::description;
using input_definition::publish_help;
using input_definition::size;
    virtual ~storage_definition() {}


   private:
    storage_definition() = delete;

    storage_definition(storage_definition && source) = delete;

    storage_definition(const storage_definition & source) = delete;

    storage_definition & operator=(const storage_definition & source) = delete;


};
//Meta class between the input file and derived classes
//of type 'storage_manager'
//
//multiple = false
//
//Implementation Notes: The simulator object must always
//have a viable storage manager object.  In fact it must be
//fully realised before the input file is read.  The options
//in the input file therefore must override the existing
//parameters. Some of these can not be changed from the
//input file and should match existing values or be ignored.
//
//Parameters:
//
// * checkname : set from input
//
// * inputname : set from command line or default, input
// file value must match existing value or issue warning
//
// * outputdir : set from input
//
// * outputname : set from input
//
// * runnum : set from command line or default, input file
// value matches existing value or issue warning
//
// * type : set from command line or default, input file
// value must match existing value or raise error
//
// ** type specific params : passed to storage manager as
// name:val map.  See manager for validity of parameters.

class storage_meta : public core::input_base_meta
 {
   private:
    boost::shared_ptr< storage_manager > fstype_;

    // Set of valid storage manager type definitions.
    boost::ptr_vector< storage_definition > types_;

    std::vector<core::input_parameter_memo> parameter_set_;

    //The type label of the current observable
    std::string type_;

    // Read an entry in the input file
    //
    // throw an error if input file is incorrect (using UTILITY_INPUT macro)
    bool do_read_entry(core::input_base_reader & reader) override;

    // Check for completeness of input at end of section. It does not
    // reset the meta data as 'multiple' is false.
    void do_read_end(const core::input_base_reader & reader) override;


   public:
    // Get a list of definition labels/keys
    std::vector< std::string > definition_key_list();

    // Add ctor for input "simulation" section
    //
    // \pre not has_type( ctor.type_name_ )
    // \post has_type( ctor.type_name_ )
    void add_type(std::unique_ptr< storage_definition >& ctor);

    // Is there a ctor to match input "simulation" section with
    // "type = [label]"
    bool has_type(std::string label);

    // The number of storage type definitions
    std::size_t size() const { return this->types_.size(); }
    

    // Ary there no storage type definitions?
    bool empty() const { return this->types_.empty(); }
    

    storage_meta(boost::shared_ptr< storage_manager > fstype);

    virtual ~storage_meta();


   private:
    // Reset the object
    void do_reset() override;


   public:
    void publish_help(core::input_help & helper) const override;

    // Name of checkpoint name parameter in input file
    static std::string checkpoint_name_label();

    // Name of checkpoint name parameter in input file
    static std::string output_name_label();

    // The name of the storage section in the input file.
    static std::string storage_label();


};

} // namespace platform
#endif
