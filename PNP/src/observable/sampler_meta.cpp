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

#include "observable/sampler_meta.hpp"
#include "observable/base_observable.hpp"
#include "core/input_parameter_memo.hpp"
#include "observable/report_manager.hpp"
#include "core/input_base_reader.hpp"

// Manual incls
#include <sstream>
#include <bitset>
#include "core/strngs.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
#include "utility/utility.hpp"
namespace observable {

sampler_meta::sampler_meta(boost::shared_ptr< report_manager > manager) 
: input_base_meta( core::strngs::sampler_label(), true, true )
, parameter_set_()
, manager_( manager )
, type_to_sample_()
, type_to_tracked_()
, type_()
{}

sampler_meta::~sampler_meta() 
{}

// Add ctor for input "trial" section with "type = [trial_label]" 
//
// \pre not has_trial_type( defn.label )
// \post has_trial_type( defn.label )
void sampler_meta::add_sampler_type(std::unique_ptr< sampler_definition > & defn)
{
  UTILITY_REQUIRE( not this->has_type( defn->label() ), ("Attempt to add more than one chooser factory for type \""+defn->label()+"\".") );
  this->type_to_sample_.push_back( defn.release() );

}

// Add ctor for input "trial" section with "type = [trial_label]" 
//
// \pre not has_trial_type( defn.label )
// \post has_trial_type( defn.label )
void sampler_meta::add_tracked_type(std::unique_ptr< tracked_definition > & defn)
{
  UTILITY_REQUIRE( not this->has_type( defn->label() ), ("Attempt to add more than one chooser factory for type \""+defn->label()+"\".") );
  this->type_to_tracked_.push_back( defn.release() );

}

// Get a list of definition labels/keys
std::vector< std::string > sampler_meta::definition_key_list()
{
  std::vector< std::string > result;
  for( auto const& entry : this->type_to_sample_ )
  {
    result.push_back( entry.label() );
  }
  for( auto const& entry : this->type_to_tracked_ )
  {
    result.push_back( entry.label() );
  }
  return result;

}

// Is there a ctor to match input "sampler" section with
// "type = [label]"
bool sampler_meta::has_type(std::string label)
{
  if( not this->type_to_sample_.empty() )
  {
    for( auto const& defn : this->type_to_sample_ )
    {
      if( defn.label() == label )
      {
        return true;
      }
    }
  }
  if( not this->type_to_tracked_.empty() )
  {
    for( auto const& defn : this->type_to_tracked_ )
    {
      if( defn.label() == label )
      {
        return true;
      }
    }
  }
  return false;
  

}

void sampler_meta::publish_help(core::input_help & helper) const
{
  const std::string seclabel( core::strngs::sampler_label() );
  // ----------------------------------------
  // add section
  // ----------------------------------------
  
  //   Specie input section definition
  core::input_help::exemplar().add_section( { seclabel,
      "Sampler input section definition.  Samplers collect data from the running simulation." } );
  
  // ----------------------------------------
  // add parameters
  // ----------------------------------------
  
  // Subtype parameter documentation.
  for( auto const& iter : this->type_to_sample_ )
  {
    iter.publish_help( helper, seclabel );
  }
  for( auto const& iter : this->type_to_tracked_ )
  {
    iter.publish_help( helper, seclabel );
  }
  
  

}

//Read an entry in the input file. Return true if the entry was processed.
//
//throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool sampler_meta::do_read_entry(core::input_base_reader & reader)
{
  if( reader.name().find( core::strngs::fstype() ) == 0 )
  {
    // --------------------
    // Sampler type
    reader.duplicate_test( this->type_.empty(), this->section_label() );
    std::vector< std::string > keys{ this->definition_key_list() };
    const std::size_t idx = reader.get_key( "Sampler subtype", this->section_label(), keys );
    this->type_ = keys.at( idx );
  }
  else
  {
    // --------------------
    // Sampler specific parameters
    const std::string lname{ reader.name() };
    reader.duplicate_test( 0 == std::count_if( this->parameter_set_.begin(), this->parameter_set_.end(), [&lname](const core::input_parameter_memo& param){ return lname == param.name(); } ), this->section_label() );
    this->parameter_set_.push_back( { reader } );
  }
  return true;
  

}

// Perform checks at the end of reading a section.
void sampler_meta::do_read_end(const core::input_base_reader & reader)
{
  if( this->type_.empty() )
  {
    reader.missing_parameters_error( "Data sampler",  this->section_label(), core::strngs::fstype() );
  }
  // Call functor method to create sampler and add to simulator
  for( const auto& defn : this->type_to_sample_ )
  {
    if( defn.label() == this->type_ )
    { 
      // check parameters.
      for( auto const& entry : this->parameter_set_ )
      {
        if( not defn.has_definition( entry.name() ) )
        {
          throw core::input_error::invalid_parameter_subtype_error( this->section_label(), this->type_, entry );
        }
      }
      // push "end" tag onto parameter set.
      this->parameter_set_.push_back( { reader } );
      this->manager_->add_sample( defn( this->parameter_set_ ) );
      return;
    }
  }
  for( const auto& defn : this->type_to_tracked_ )
  {
    if( defn.label() == this->type_ )
    { 
      // check parameters.
      for( auto const& entry : this->parameter_set_ )
      {
        if( not defn.has_definition( entry.name() ) )
        {
          throw core::input_error::invalid_parameter_subtype_error( this->section_label(), this->type_, entry );
        }
      }
      // push "end" tag onto parameter set.
      this->parameter_set_.push_back( { reader } );
      this->manager_->add_tracked( defn( this->parameter_set_ ) );
      return;
    }
  }
  // error to get here
  UTILITY_CHECK( false, "should never reach this point in the code." );

}

// Reset internal data.
void sampler_meta::do_reset()
{
  // Reset meta data
  this->parameter_set_.clear();
  this->type_.clear();

}


} // namespace observable
