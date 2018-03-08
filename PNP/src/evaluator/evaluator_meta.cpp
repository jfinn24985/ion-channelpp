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

#include "evaluator/evaluator_meta.hpp"
#include "evaluator/base_evaluator.hpp"
#include "core/input_parameter_memo.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "core/input_base_reader.hpp"

// Manual incls
#include <sstream>
#include <bitset>
#include "core/strngs.hpp"
#include "core/input_help.hpp"
#include "core/input_error.hpp"
#include "utility/utility.hpp"
namespace evaluator {

evaluator_meta::evaluator_meta(boost::shared_ptr< evaluator_manager >& eman) 
: input_base_meta (core::strngs::evaluator_label(), true, true)
, manager_( eman )
, parameter_set_()
, type_()
{}

evaluator_meta::~evaluator_meta()
{}

// Add ctor for input "evaluator" section with "type = [defn.label]" 
//
// \pre not has_definition( defn.label )
// \post has_definition( defn.label )
void evaluator_meta::add_definition(std::unique_ptr< evaluator_definition > & defn)
{
  UTILITY_REQUIRE( 0 == this->has_definition( defn->label() ), ( "Attempt to add more than one evaluator factory for type \"" + defn->label() + "\"." ) );
  this->type_to_object_.push_back( defn.release() );

}

// Add ctor for input "evaluator" section with "type = [defn.label]" 
//
// \pre not has_definition( defn.label )
// \post has_definition( defn.label )
void evaluator_meta::add_definition(std::unique_ptr< evaluator_definition > && defn)
{
  UTILITY_REQUIRE( 0 == this->has_definition( defn->label() ), ( "Attempt to add more than one evaluator factory for type \"" + defn->label() + "\"." ) );
  this->type_to_object_.push_back( defn.release() );

}

// Get a list of definition labels/keys
std::vector< std::string > evaluator_meta::definition_key_list()
{
  std::vector< std::string > result;
  for( auto const& entry : this->type_to_object_ )
  {
    result.push_back( entry.label() );
  }
  return result;

}

// Get definition of evaluator that matches the label
const evaluator_definition& evaluator_meta::get_definition(std::string label) const
{
  for( auto const& defn : this->type_to_object_ )
  {
    if( defn.label() == label )
    {
      return defn;
    }
  }
  constexpr bool label_found = false;
  UTILITY_REQUIRE( label_found, "No definition with the given label" );
  return this->type_to_object_.front();

}

// Is there a ctor to match input "sampler" section with
// "type = [label]"
bool evaluator_meta::has_definition(std::string label) const
{
  for( auto const& defn : this->type_to_object_ )
  {
    if( defn.label() == label )
    {
      return true;
    }
  }
  return false;

}

void evaluator_meta::publish_help(core::input_help & helper) const
{
  const std::string seclabel( core::strngs::evaluator_label() );
  // ----------------------------------------
  // add section
  // ----------------------------------------
  
  //   Evaluator input section definition
  helper.add_section( { seclabel, "Evaluator input sections specify a potential energy functions and its parameters. The input file must include an evaluator section for each energy function to be used in the simulation." } );
  
  // ----------------------------------------
  // add parameters
  // ----------------------------------------
  
  // Subtype parameter documentation.
  for( auto const& iter : this->type_to_object_ )
  {
    iter.publish_help( helper, seclabel );
  }

}

//Read an entry in the input file. Return true if the entry was processed.
//
//throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool evaluator_meta::do_read_entry(core::input_base_reader & reader)
{
  if( reader.name().find( core::strngs::fstype() ) == 0 )
  {
    // --------------------
    // Evaluator type
    reader.duplicate_test( this->type_.empty(), this->section_label() );
    const std::vector< std::string > keys{ this->definition_key_list() };
    std::size_t idx = reader.get_key( "Evaluator selection", this->section_label(), keys );
    this->type_ = keys.at( idx );
  }
  else
  {
    // --------------------
    // Evaluator specific parameters
    const std::string lname{ reader.name() };
    reader.duplicate_test( 0 == std::count_if( this->parameter_set_.begin(), this->parameter_set_.end(), [&lname](const core::input_parameter_memo& param){ return lname == param.name(); } ), this->section_label() );
    this->parameter_set_.push_back( { reader } );
  }
  return true;
  

}

// Perform checks at the end of reading a section.
void evaluator_meta::do_read_end(const core::input_base_reader & reader)
{
  if( this->type_.empty() )
  {
    reader.missing_parameters_error( "Energy function",  this->section_label(), core::strngs::fstype() );
  }
  // Call functor method to create evaluator
  for( const auto& defn : this->type_to_object_ )
  {
    if( defn.label() == this->type_ )
    { 
      // check parameters are in definition
      for( const auto& entry : this->parameter_set_ )
      {
        if( not defn.has_definition( entry.name() ) )
        {
          throw core::input_error::invalid_parameter_subtype_error( this->section_label(), this->type_, entry );
        }
      }
      // push "end" tag onto parameter set.
      this->parameter_set_.push_back( { reader } );
      this->manager_->add_evaluator( defn( this->parameter_set_ ) );
      return;
    }
  }
  // error to get here
  UTILITY_CHECK( false, "should never reach this point in the code." );

}

// Reset internal data.
void evaluator_meta::do_reset()
{
  // Reset meta data
  this->parameter_set_.clear();
  this->type_.clear();

}


} // namespace evaluator
