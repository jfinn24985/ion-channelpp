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

#include "trial/choice_meta.hpp"
#include "trial/base_chooser.hpp"
#include "trial/choice_manager.hpp"
#include "core/input_base_reader.hpp"

// Manual incls
#include <sstream>
#include <bitset>
#include "core/strngs.hpp"
#include "core/input_help.hpp"
#include "utility/utility.hpp"
namespace trial {

// 
choice_meta::choice_meta(boost::shared_ptr< choice_manager > man) 
: input_base_meta( core::strngs::fstry(), true, true )
, manager_( man )
, parameter_set_()
, missing_required_tags_(std::bitset< CHOICE_TAG_COUNT >( true ))
, rate_()
, type_()
{}


choice_meta::~choice_meta() 
{}

// Add ctor for input "trial" section
//
// \pre not has_trial_type( ctor.type_name_ )
// \post has_trial_type( ctor.type_name_ )
void choice_meta::add_trial_type(std::unique_ptr< choice_definition >& ctor)
{
  UTILITY_REQUIRE( not this->has_trial_type( ctor->label() ), ("Attempt to add more than one chooser factory for type \"" + ctor->label() + "\".") );
  this->type_to_object_.push_back( ctor.release() );
  
  

}

// Get a list of definition labels/keys
std::vector< std::string > choice_meta::definition_key_list()
{
  std::vector< std::string > result;
  for( auto const& entry : this->type_to_object_ )
  {
    result.push_back( entry.label() );
  }
  return result;

}

// Is there a ctor to match input "trial" section with
// "type = [trial_label]"
bool choice_meta::has_trial_type(std::string trial_label)
{
  for( auto const& defn : this->type_to_object_ )
  {
    if( defn.label() == trial_label )
    {
      return true;
    }
  }
  return false;
  

}

// 
void choice_meta::publish_help(core::input_help & helper) const
{
  const std::string seclabel( core::strngs::fstry() );
  // ----------------------------------------
  // add section
  // ----------------------------------------
  
  //   Specie input section definition
  helper.add_section( { seclabel,
      "Define which MC change trials are used in the simulation." } );
  auto &sect = helper.get_section( seclabel );
  // ----------------------------------------
  // add parameters
  // ----------------------------------------
  
  //   rate : [required, number] trial rate
  {
    const std::string description("Trial type relative rate (not necessarily normalized).");
    sect.add_entry( { core::strngs::rate_label(), "number", ">0", "required", description } );
  }
  //  specie : [optional] include/exclude list
  {
    const std::string description("Space separated list of specie labels to include or exclude. Exclude species by preceding the label with '-' (e.g. -Aa). All species are included by default, but you may also include only selected species by putting their label in the list without '-' (optionally preceded by '+'). NOTE: This should only be used in special cases as most trials already restrict which specie types they are applied to. For example \"mobile\" type species are automatically excluded from \"jump\" or Grand Canonical trials. This list does not override these restrictions.");
    sect.add_entry( { core::strngs::fsspec(), "list", "specie labels", "optional", description } );
  }
  // Subtype parameter documentation.
  for( auto iter = this->type_to_object_.begin(); iter != this->type_to_object_.end(); ++iter )
  {
    iter->publish_help( helper, seclabel );
  }
  

}

// Read an entry in the input file. Return true if the entry was processed.
//
// Throws an error if input file is incorrect (using UTILITY_INPUT macro)
bool choice_meta::do_read_entry(core::input_base_reader & reader)
{
  if( reader.name().find( core::strngs::rate_label() ) == 0 )
  {
    // --------------------
    // Choice rate
    reader.duplicate_test( this->missing_required_tags_[ CHOICE_RATE ], this->section_label() );
    this->rate_ = reader.get_float( "Trial likelihood", this->section_label(), true, false );
    this->missing_required_tags_.reset( CHOICE_RATE );
  }
  else if( reader.name().find( core::strngs::fstype() ) == 0 )
  {
    // --------------------
    // Trial type
    reader.duplicate_test( this->missing_required_tags_[ CHOICE_TYPE ], this->section_label() );
    std::vector< std::string > keys{ this->definition_key_list() };
    const std::size_t idx = reader.get_key( "Trial subtype ", this->section_label(), keys );
    this->type_ = keys.at( idx );
    this->missing_required_tags_.reset( CHOICE_TYPE );
  }
  else if( reader.name().find( core::strngs::fsspec() ) == 0 )
  {
    // --------------------
    // Allowed/disallowed specie list
    reader.duplicate_test( this->specie_list_.name().empty(), this->section_label() );
    core::input_parameter_memo tmp( reader );
    tmp.set_value( reader.get_text( "Specie inclusion list", core::strngs::fstry() ) ); 
    this->specie_list_.swap( tmp );
  }
  else
  {
    // --------------------
    // Assume choice specific parameters
    const std::string lname{ reader.name() };
    reader.duplicate_test( 0 == std::count_if( this->parameter_set_.begin(), this->parameter_set_.end(), [&lname](const core::input_parameter_memo& param){ return lname == param.name(); } ), this->section_label() );
    this->parameter_set_.push_back( core::input_parameter_memo( reader ) );
  }
  return true;
  

}

// Perform checks at the end of reading a section. Then create a chooser
// object and add it to the choice manager. Resets the
// meta object state ready for the next 'trial' input section.
//
// Throws an error if input file is incorrect (using UTILITY_INPUT macro)
void choice_meta::do_read_end(const core::input_base_reader & reader)
{
  if( this->missing_required_tags_.any() )
  {
    // Not all required tags were present.
    std::string missing;
    if( this->missing_required_tags_[ CHOICE_RATE ] ) missing = core::strngs::rate_label();
    if( this->missing_required_tags_[ CHOICE_TYPE ] ) missing += ( missing.empty() ? "" : " " ) + core::strngs::fstype();
    reader.missing_parameters_error( "Trial definition", this->section_label(), missing );
  }
  // find definition
  std::size_t idx = 0;
  for( idx = 0; idx != this->type_to_object_.size(); ++idx )
  {
    if( this->type_to_object_[ idx ].label() == this->type_ )
    {
      break;
    }
  }
  UTILITY_CHECK( idx != this->type_to_object_.size(), "Check in do_read_entry should mean this is never true." );
  auto const& defn = this->type_to_object_[ idx ];
  // use definiton to check parameter names
  if( not this->parameter_set_.empty() )
  {
    std::string bad_params;
    for( auto const& entry : this->parameter_set_ )
    {
      if( not defn.has_definition( entry.name() ) )
      {
        bad_params += (bad_params.empty() ? "": " ") + entry.name();
      }
    }
    if( not bad_params.empty() )
    {
      reader.invalid_parameter_subtype_error( this->section_label(), this->type_, bad_params );
    } 
  }
  // push "end" tag onto parameter set.
  this->parameter_set_.push_back( core::input_parameter_memo( reader ) );
  // Use generator to create the chooser object.
  this->manager_->add_chooser( defn( this->parameter_set_, this->type_, this->specie_list_, this->rate_ ) );

}

// Reset the object state on entry into read_seaction. This
// provides a stronger exception safety if the delegate is
// reused after raising an exception in read_section.
void choice_meta::do_reset()
{
  this->missing_required_tags_.set();
  this->parameter_set_.clear();
  this->rate_ = 0.0;
  core::input_parameter_memo tmp;
  this->specie_list_.swap( tmp );
  this->type_.clear();
  

}


} // namespace trial
