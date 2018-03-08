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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "geometry/region_meta.hpp"
#include "geometry/base_region.hpp"
#include "core/input_parameter_memo.hpp"
#include "geometry/geometry_manager.hpp"
#include "core/input_help.hpp"
#include "core/input_base_reader.hpp"

// Manual includes
#include "core/input_error.hpp"
#include "core/strngs.hpp"
// -
namespace geometry {

// Main ctor.
region_meta::region_meta(boost::shared_ptr< geometry_manager > man)
: input_base_meta( core::strngs::fsregn(), true, false ) // multiple + not required
, manager_( man )
, type_to_object_()
, label_()
, type_name_()
, parameter_set_()
{}

region_meta::~region_meta()
{

}

// Add a new region type definition.
//
// \pre not has_definition(defn.type_name_)
void region_meta::add_definition(std::unique_ptr< region_definition > & defn)
{
  UTILITY_REQUIRE( not this->has_definition( defn->label() ), "Can not add two region definitions with the same name." );
  this->type_to_object_.push_back( defn.release() );

}

// Get a list of definition labels/keys
std::vector< std::string > region_meta::definition_key_list()
{
  std::vector< std::string > result;
  for( auto const& entry : this->type_to_object_ )
  {
    result.push_back( entry.label() );
  }
  return result;

}

void region_meta::publish_help(core::input_help & helper) const
{
  const std::string rgnlabel( this->section_label() );
  // ----------------------------------------
  // add section
  // ----------------------------------------
  
  //   Specie input section definition
  helper.add_section( { rgnlabel,
      "Region input section definition.  In addition to the listed options, each individual "
      "region subtype may have its own specific options." } );
  
  // ----------------------------------------
  // add parameters
  // ----------------------------------------
  
  //  name : [required] region label
  {
    const std::string description( "Label for a region. Must be unique in a simulation as it can be referred to from other parts of the simulation." );
    helper.get_section( rgnlabel ).add_entry( { core::strngs::fsname(), "text label", "unique for all regions", "required", description } );
  }
  
  // Subtype parameter documentation.
  for( auto iter = this->type_to_object_.begin(); iter != this->type_to_object_.end(); ++iter )
  {
    iter->publish_help( helper, rgnlabel );
  }

}

// Do we have a region with this typename?
bool region_meta::has_definition(std::string type_name)
{
  for (auto iter = this->type_to_object_.begin(); iter != this->type_to_object_.end(); ++iter )
  {
    if (iter->label() == type_name )
    {
      return true;
    }
  }
  return false;

}

//Read a name/value entry in the input file. Return true if the entry was processed.
//
//throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool region_meta::do_read_entry(core::input_base_reader & reader)
{
  if( reader.name() == core::strngs::fstype() )
  {
    // --------------------
    // Region type
    reader.duplicate_test( this->type_name_.empty(), this->section_label() );
    std::vector< std::string > keylist{ this->definition_key_list() };
    const std::size_t idx = reader.get_key( "Region subtype", this->section_label(), keylist );
    this->type_name_ = keylist.at( idx );
  }
  else if( reader.name() == core::strngs::fsname() )
  {
    // --------------------
    // Region label
    reader.duplicate_test( this->label_.empty(), this->section_label() );
    this->label_ = reader.get_text( "Region label", this->section_label() );
  }
  else
  {
    // --------------------
    // Region subtype specific parameters
    const std::string lname{ reader.name() };
    reader.duplicate_test( 0 == std::count_if( this->parameter_set_.begin(), this->parameter_set_.end(), [&lname](const core::input_parameter_memo& param){ return lname == param.name(); } ), this->section_label() );
    this->parameter_set_.push_back( core::input_parameter_memo( reader ) );
  }
  return true;

}

// Perform checks at the end of reading a section and create/add result to
// simulations. If multiple sections may be read, this method must also
// reset the meta object state.
void region_meta::do_read_end(const core::input_base_reader & reader)
{
  {
    std::string bad_params;
    // check for required parameters.
    if( this->type_name_.empty() ) bad_params = core::strngs::fstype();
    if( this->label_.empty() ) bad_params += (bad_params.empty() ? "": " ") +  core::strngs::fsname();
    if( not bad_params.empty() ) reader.missing_parameters_error( "Geometry", core::strngs::fsregn(), bad_params );
  }
  // We know from having a type name and do_read_entry that a
  // type object must exist.
  for( auto const& defn : this->type_to_object_ )
  {
    if( defn.label() == this->type_name_ )
    {
      // check parameters.
      for( auto const& entry : this->parameter_set_ )
      {
        // check for required parameters.
        if( not defn.has_definition( entry.name() ) )
        {
          throw core::input_error::invalid_parameter_subtype_error( this->section_label(), this->type_name_, entry );
        }
      }
      // push "end" tag onto parameter set.
      this->parameter_set_.push_back( core::input_parameter_memo( reader ) );
      // build object and add to manager.
      this->manager_->add_region( defn( this->label_, this->parameter_set_ ) );
      return;
    }
  }
  UTILITY_CHECK( false, "Earlier checks for region type should mean we never get here." );

}

// Reset the object state on entry into read_seaction. This
// provides a stronger exception safety if the delegate is
// reused after raising an exception in read_section.
void region_meta::do_reset()
{
  this->label_.clear();
  this->type_name_.clear();
  this->parameter_set_.clear();

}


} // namespace geometry
