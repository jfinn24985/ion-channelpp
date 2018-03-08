//Authors: Justin Finnerty
//
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

#include "particle/specie.hpp"
#include "core/input_document.hpp"

//--
#include "core/strngs.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
//#include "core/XXinput_reader.hpp"
#include "utility/config.hpp"
#include "utility/fuzzy_equals.hpp"
// -
//#include <bitset>
//#include <boost/lexical_cast.hpp>
//#include <iostream>

namespace particle {

//chemical potential in units of kT : log([conc]) - mu_ex
//
//(mu_ex = excess_potential_)
double specie::chemical_potential() const 
{
  return (utility::feq(0.0, this->concentration_) ? 0.0 : std::log(this->concentration_/core::constants::to_SI()) + this->excess_potential_);

}

//Write a human readable summary of the specie to 'out'
void specie::description(std::ostream & out) const 
{
  out << "specie\n";
  out << "        label :" << this->label_ << "\n";
  out << "       radius :" << this->radius_ << "\n";
  out << "         rate :" << this->rate_ << "\n";
  out << "      valency :" << this->valency_ << "\n";
  out << "  excess c.p. :" << this->excess_potential_ << "\n";
  out << "   chem. pot. :" << this->chemical_potential() << "\n";
  out << " target conc. :" << this->concentration_ << "\n";
  out << "         type :" << type_label(this->type_) << "\n";
  if (not this->parameter_set_.empty())
  {
    out << " - parameters -\n";
    for (auto & keyval : this->parameter_set_)
    {
      out << std::setw(15) << (keyval.name() + " :") << keyval.value() << "\n";
    }
  }

}

// Compare if two species have the same information
bool specie::equivalent(const specie & other) const
{
  return 
    this->parameter_set_ == other.parameter_set_ and
    this->locations_ == other.locations_ and
    this->localize_data_ == other.localize_data_ and
    this->concentration_ == other.concentration_ and
    this->excess_potential_ == other.excess_potential_ and
    this->label_ == other.label_ and
    this->radius_ == other.radius_ and
    this->rate_ == other.rate_ and
    this->valency_ == other.valency_ and
    this->type_ == other.type_;

}

//Test if parameter is present in this specie
bool specie::has_parameter(std::string name) const 
{
  for( auto & param : this->parameter_set_ )
  {
    if( param.name() == name ) return true;
  }
  return false;

}

// Define a partial ordering of species based on type.
//
// The ordering is MOBILE < FLEXIBLE < CHANNEL < SOLUTE
//
// \pre type != INVALID 
bool specie::operator <(const specie & other) const
{
   UTILITY_REQUIRE(this->is_valid() and other.is_valid(), "Can not sort an invalid specie object");
   return this->type_ < other.type_;
}

//Get a parameter value
//
//\pre has_parameter (name)
const core::input_parameter_memo& specie::parameter(std::string name) const
{
  for( auto & param : this->parameter_set_ )
  {
    if( param.name() == name ) return param;
  }
  UTILITY_REQUIRE( false, "Parameter '"+name+"' not in specie '"+this->label_+"'");
  return this->parameter_set_.front(); // suppress warnings 

}

//Convert specie type value into a string
//
//\pre typeval in { MOBILE, FLEXIBLE, CHANNEL_ONLY, SOLUTE or INVALID }
//( INVALID returns non-dictionary string "invalid" which is not recognised by
// the string_to_specie_type companion method )
std::string specie::type_label(std::size_t typeval)
{
  switch(typeval) 
  {
  case MOBILE:
    return core::strngs::fsmobl();
  case FLEXIBLE:
    return core::strngs::fsflxd();
  case CHANNEL_ONLY:
    return core::strngs::fschon();
  case SOLUTE:
    return core::strngs::fsfree();
  default:
    {
      size_t valid_value = MOBILE;
      UTILITY_ALWAYS(typeval == valid_value, "Attempt to get label for invalid specie type");
    }
  case INVALID:
    return "invalid";
  };
  

}

// LIFETIME METHODS
// base constructor
specie::specie() 
: parameter_set_ ()
, locations_()
, localize_data_()
, concentration_ (0.0)
, excess_potential_ (0.0)
, label_ ()
, radius_(0.0)
, rate_(1.0)
, valency_(0.0)
, type_ (INVALID)
{}

specie::specie(const specie & other) 
: parameter_set_ ( other.parameter_set_ )
, locations_ ( other.locations_ )
, localize_data_ ( other.localize_data_ )
, concentration_ ( other.concentration_ )
, excess_potential_ ( other.excess_potential_ )
, label_ ( other.label_ )
, radius_ ( other.radius_ )
, rate_ ( other.rate_ )
, valency_ ( other.valency_ )
, type_ ( other.type_ )
{}


specie::specie(specie && other) 
: parameter_set_( std::move( other.parameter_set_ ) )
, locations_ ( std::move( other.locations_ ) )
, localize_data_ ( std::move( other.localize_data_ ) )
, concentration_( std::move( other.concentration_ ) )
, excess_potential_( std::move( other.excess_potential_ ) )
, label_( std::move( other.label_ ) )
, radius_( std::move( other.radius_ ) )
, rate_( std::move( other.rate_ ) )
, valency_( std::move( other.valency_ ) )
, type_( std::move( other.type_ ) )
{}


void specie::swap(specie & other) 
{
  std::swap(this->parameter_set_, other.parameter_set_);
  std::swap(this->locations_, other.locations_);
  std::swap(this->localize_data_, other.localize_data_);
  std::swap(this->concentration_, other.concentration_);
  std::swap(this->excess_potential_, other.excess_potential_);
  std::swap(this->label_, other.label_);
  std::swap(this->radius_, other.radius_);
  std::swap(this->rate_, other.rate_);
  std::swap(this->valency_, other.valency_);
  std::swap(this->type_, other.type_);

}

//Set a parameter value
//
//\pre not has_parameter( name )
//\post has_parameter( name )
void specie::set_parameter(core::input_parameter_memo param)
{
  UTILITY_REQUIRE( not this->has_parameter( param.name() ),
        "Can not change an existing parameter." );
  this->parameter_set_.push_back( std::move( param ) );

}

// Convert a text representation of a specie type (eg from input file) into 
// the specie type enum value. Users should "dequote" and "trim" values
// before calling this method.
//
// Sets 'set' to true if valid value is found, otherwise INVALID value is returned.
specie::specie_type specie::string_to_specie_type(std::string val, bool & set)
{
  set = true;
  if (val == core::strngs::fsmobl()) return specie::MOBILE;
  if (val == core::strngs::fsflxd()) return specie::FLEXIBLE;
  if (val == core::strngs::fschon()) return specie::CHANNEL_ONLY;
  if (val == core::strngs::fsfree()) return specie::SOLUTE;
  set = false;
  return specie::INVALID;

}

// Specie type keys as list in same order as enum values.
std::vector< std::string > specie::specie_type_list()
{
  // INVALID value is always highest key value.
  std::vector< std::string > result( specie::INVALID );
  
  result[ specie::MOBILE ] = core::strngs::fsmobl();
  result[ specie::FLEXIBLE ] = core::strngs::fsflxd();
  result[ specie::CHANNEL_ONLY ] = core::strngs::fschon();
  result[ specie::SOLUTE ] = core::strngs::fsfree();
  
  return result;

}

// Write representation of this specie object in the input file format.
//
// Adds standard data and parameter list. Adds x,y,z coordinates
// of cached particles. The position data must have been updated 
// (using update_position) from the ensemble if the generated file 
// is to have latest position information.
void specie::write_document(core::input_document & wr) const
{
  std::size_t ix = wr.add_section( core::strngs::fsspec() );
  auto &sec = wr[ ix ];
  sec.add_entry( core::strngs::fsname(), "\"" + this->label () + "\"" );
  // Output the specie type
  switch ( this->sub_type() )
  {
   case specie::CHANNEL_ONLY:
    sec.add_entry( core::strngs::fstype(), core::strngs::fschon() );
    break;
  case specie::MOBILE:
    sec.add_entry( core::strngs::fstype(), core::strngs::fsmobl() );
    break;
  case specie::FLEXIBLE:
    sec.add_entry( core::strngs::fstype(), core::strngs::fsflxd() );
    break;
  default:
    sec.add_entry( core::strngs::fstype(), core::strngs::fsfree() );
    break; // Assume INVALID will become a free specie
  }
  sec.add_entry( core::strngs::fsctrg(), this->concentration() );
  sec.add_entry( core::strngs::rate_label(), this->rate() );
  sec.add_entry( core::strngs::fsd(), this->radius()*2.0 );
  sec.add_entry( core::strngs::fsz(), this->valency() );
  sec.add_entry( core::strngs::fschex(), this->excess_potential() );
  auto quote = [](std::string s){ return (s.front() == '"' and s.front() != s.back() ? s : '"' + s  + '"'); };
  for (auto & nvpair : this->parameter_set_)
  {
    sec.add_entry( quote( nvpair.name() ), quote( nvpair.value() ) );
  }
  if (this->locations_.size() != 0)
  {
     std::stringstream ss;
     if ( this->is_localized() )
     {
       UTILITY_REQUIRE( this->localize_data_.size() == this->locations_.size(), "Number of particles does not match number of localization points" );
        for (std::size_t ix = 0; ix != this->locations_.size(); ++ix)
        {
           ss << this->locations_[ ix ] << " " << this->localize_data_[ ix ];
           ss << "\n";
        }
     }
     else
     {
        for (std::size_t ix = 0; ix != this->locations_.size(); ++ix)
        {
           ss << this->locations_[ ix ] << "\n";
        }
     }
     std::string result( ss.str() );
     if (result.size() != 0) // remove last '\n' if it exists
     {
        result.pop_back();
     }
     sec.add_entry( core::strngs::fsn(), std::to_string( this->locations_.size() ) + "\n" +
  result );
  }

}

// Update the cached position of a particular particle. The index 'lidx' refers
// to the index of the particle within the species. This method automatically
// updates the size of the array, such resizing can be avoided by first calling
// update_position_size. Such resizing is only allowed for species that can  
// have particles added/removed from the simulation.
void specie::update_position(std::size_t lidx, const geometry::coordinate & pos)
{
  if( this->locations_.size() <= lidx )
  {
    UTILITY_REQUIRE( this->sub_type() == SOLUTE, "Only solute particles can be added or removed." );
    this->locations_.resize( lidx + 1 );
  }
  this->locations_[ lidx ] = pos;
  

}

// Update the number cached positions. This is only allowed for types that support
// addition/deletion.
void specie::update_position_size(std::size_t count)
{
  UTILITY_REQUIRE( this->sub_type() == SOLUTE, "Only solute particles can be added or removed." );
  this->locations_.resize( count );

}

// Add a particle to the specie from the input file when centroid information
// is available. This implicitly sets specie type to MOBILE if it is currently
// set to INVALID, otherwise the specie type must support localisation (e.g.
// MOBILE or FLEXIBLE).
//
// This method should only be called while the simulation is being 
// instantiated (specifically before simulator::generate_simulation is
// called). Calling this method after the simulation has begun is
// undefined.
//
// \require is_localized or sub_type == INVALID
void specie::append_position(const geometry::coordinate & pos, const geometry::centroid & cntr)
{
  if ( this->type_ == INVALID )
  {
    this->type_ = MOBILE;
  }
  UTILITY_REQUIRE( this->type_ == MOBILE or this->type_ == FLEXIBLE, "Only specie type MOBILE and FLEXIBLE require centre/localization information" );
  UTILITY_REQUIRE( ( this->type_ == MOBILE or this->type_ == FLEXIBLE ) and this->locations_.size() == this->localize_data_.size(), "Error in location cache, different sizes for position and centre information" );
  this->locations_.push_back( pos );
  this->localize_data_.push_back( cntr );

}

// Add a particle to the specie from the input file when no centroid information
// is available. This does not implicitly set the specie type.
//
// This method should only be called while the simulation is being 
// instantiated (specifically before simulator::generate_simulation is
// called). Calling this method after the simulation has begun is
// undefined.
//
// \require not is_localized
void specie::append_position(const geometry::coordinate & pos)
{
  UTILITY_REQUIRE( this->type_ != MOBILE and this->type_ != FLEXIBLE and this->localize_data_.empty(), "Specie type MOBILE and FLEXIBLE require centre information." );
  this->locations_.push_back( pos );

}

// Get the cached position of a particular particle. The index 'lidx' refers
// to the index of the particle within the species, and is in the range 0
// to get_position_size().
//
// \pre lidx < get_position_size()
//
// This data is the position data read from the input file. 
const geometry::coordinate& specie::get_position(std::size_t lidx) const
{
  return this->locations_.at( lidx );

}

// Get the localization data of a particular particle. The index 'lidx' refers
// to the index of the particle within the species, and therefore fills the range
// 0 to get_position_size() for localized species.
//
// \pre is_localized and ldx < get_postion_size()
const geometry::centroid& specie::get_localization_data(std::size_t lidx) const
{
  UTILITY_REQUIRE( this->is_localized(), "Only localized species have localization data" );
  return this->localize_data_.at( lidx );

}


} // namespace particle
