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

#include "cscchannel/gc_choice_cell.hpp"
#include "particle/ensemble.hpp"
#include "cscchannel/remove_choice_cell.hpp"
#include "cscchannel/channel_system.hpp"
#include "particle/change_set.hpp"
#include "platform/simulator.hpp"
#include "particle/specie.hpp"

// Manual includes
#include "core/strngs.hpp"
#include "utility/mathutil.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
// =
namespace ion_channel {

//Create a grand canonical individual move.
//
//choice hash subtype is "0" (one particle changes)
//hash pattern is ( X, 0, 1, 0 ). NOTE: GC moves use
//the hash pattern of their "add" sub-choice.
gc_choice_cell::gc_choice_cell(const std::size_t & ispec, std::map< std::string, std::string> const& params, const channel_system & geom)
: base_choice( { ispec, 0, 1, 0 } )
, adder_()
, remover_()
{
   // calculate region rates.
   std::vector< double > rates( channel_system::REGION_COUNT, 1.0 );
   std::string rateset;
   auto const& spc = geom.get_specie( ispec );
   if ( not params.empty() )
   {
      std::string bad_parameter_list;
      for ( auto const& entry : params )
      {
         if ( entry.first != core::strngs::fsrtrg() and
               entry.first != core::strngs::fsspec() )
         {
            if ( bad_parameter_list.empty() )
            {
               bad_parameter_list = entry.first;
            }
            else
            {
               bad_parameter_list += " " + entry.first;
            }
         }
      }
      UTILITY_INPUT( bad_parameter_list.empty(), "Unknown options {"+bad_parameter_list+"}. Grand Canonical choice has only \""+core::strngs::fsrtrg()
            +"\" as non-standard parameter.", core::strngs::fstry(), nullptr );
   }
   else
   {
      if ( spc.has_parameter( core::strngs::fsrtrg() ) )
      {
         rateset = spc.parameter( core::strngs::fsrtrg() );
      }
   }
   if ( not rateset.empty() )
   {
      boost::tokenizer<> tok( rateset );
      rates.clear();
      for ( auto itr = tok.begin(); itr != tok.end(); ++itr )
      {
         rates.push_back( boost::lexical_cast< double >( *itr ) );
      }
   }
   // normalize
   double sum_of_rates = 0.0;
   rates.resize( channel_system::REGION_COUNT, 0.0 );
   for ( double v : rates ) sum_of_rates += v;
   UTILITY_INPUT( not utility::feq(0.0, sum_of_rates), "Grand Canonical sum of proabilities must not be zero.", core::strngs::fstry(), nullptr );
   for ( double &v : rates ) v /= sum_of_rates;

   // Create subchoices
   for ( std::size_t ireg = 0; ireg != channel_system::REGION_COUNT; ++ireg )
   {
      cylinder reg = geom.region_geometry( ireg, ispec );
      std::unique_ptr< add_choice_cell > adr( new add_choice_cell( ispec, *this, reg ) );
      this->adder_.push_back( adr.get() );
      adr.release();
      std::unique_ptr< remove_choice_cell > rmr( new remove_choice_cell( ispec, *this, reg ) );
      this->remover_.push_back( rmr.get() );
      rmr.release();
   }
}


gc_choice_cell::~gc_choice_cell() 
{
}

//Create a particle addition/remove trial
std::unique_ptr< particle::change_set > gc_choice_cell::generate(platform::simulator & sys) 
{
  // Choose region to GC.
  double choice{ sys.get_random().uniform(0.0, 2.0) };
  if (choice < 1.0)
  {
     // Add
     for (size_t ireg = channel_system::IZLIM; ireg < channel_system::IBULK; ++ireg)
     {
        choice -= this->adder_[ireg].probability();
        if (choice <= 0.0)
        {
           return this->adder_[ireg].generate(sys);
        }
     }
     return this->adder_[channel_system::IBULK].generate(sys);
  }
  else
  {
     // Remove
     choice -= 1.0;
     for (size_t ireg = channel_system::IZLIM; ireg < channel_system::IBULK; ++ireg)
     {
        choice -= this->remover_[ireg].probability();
        if (choice <= 0.0)
        {
           return this->remover_[ireg].generate(sys);
        }
     }
     return this->remover_[channel_system::IBULK].generate(sys);
  }

}

add_choice_cell const& gc_choice_cell::get_adder(std::size_t ireg) const
{
  return this->adder_[ ireg ];
}

const remove_choice_cell& gc_choice_cell::get_remover(std::size_t ireg) const
{
  return this->remover_[ ireg ];
}

std::string gc_choice_cell::label() const 
{
  return "GC Ind";
}

//Return true only if {spec} is a solute species.
bool gc_choice_cell::permitted(const particle::specie & spec) const
{
  return spec.is_solute();
}


} // namespace ion_channel

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(ion_channel::gc_choice_cell, "ion_channel::gc_choice_cell");