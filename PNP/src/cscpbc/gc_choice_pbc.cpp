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

#include "cscpbc/gc_choice_pbc.hpp"
#include "particle/ensemble.hpp"
#include "platform/simulator.hpp"
#include "trial/choice.hpp"

// manual includes
#include "core/strngs.hpp"
#include "utility/utility.hpp"
// -
namespace periodic_cube {

gc_chooser_pbc::~gc_chooser_pbc() 
{
}

std::string gc_chooser_pbc::label() const 
{
  return "GC Ind";
}

//  Function to add move choice objects to the simulation.
void gc_chooser_pbc::make_chooser(const std::map< std::string, std::string >& params, double rate, platform::simulator & sim)
{
  for (std::size_t ispec = 0; ispec != sim.specie_count (); ++ispec)
  {
     // All species with non-zero specie rate can have a GC move
     // in a PBC simulation
     auto const& spec = sim.get_specie( ispec );
     const double choice_rate { rate * spec.rate() };
     if ( not utility::feq( choice_rate, 0.0 ) )
     {
        std::unique_ptr< base_choice > choice_ptr( new gc_choice_pbc( ispec ) );
        choice_ptr->set_probability( choice_rate );
        //sim.add_chooser( choice_ptr.release() );
     }
  }

}

//  Generate and add choices to simulator.
void gc_chooser_pbc::generate_choices(const platform::simulator & sim, boost::ptr_vector< trial::base_choice >& choices) const
{
  {
    // All species with non-zero specie rate can have a GC move
    // in a PBC simulation
    double specie_rate_sum = 0.0;
    for ( auto const& spec : sim.get_species() )
    {
      specie_rate_sum += spec.rate();
    }
    IONCH_INPUT( not utility::feq( specie_rate_sum ), "Can not create simulation with sum of species set at zero.", core::strngs::fstry() );
    for (std::size_t ispec = 0; ispec != sim.get_specie_count(); ++ispec )
    {
      auto const& spec = sim.get_specie( ispec );
      const double choice_rate
      {
        rate * spec.rate()
      };
      if ( not utility::feq( choice_rate, 0.0 ) )
      {
        std::unique_ptr< base_choice > add_ptr( new add_choice_pbc( ispec ) );
        std::unique_ptr< base_choice > rem_ptr( new add_choice_pbc( ispec ) );
        add_ptr->set_probability( choice_rate );
        rem_ptr->set_probability( choice_rate );
        choices.push_back( add_ptr.release() );
        choices.push_back( rem_ptr.release() );
      }
    }
  }
  

}


} // namespace periodic_cube

BOOST_CLASS_EXPORT_IMPLEMENT(periodic_cube::gc_choice_pbc);