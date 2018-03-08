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

#include "cscchannel/remove_choice_cell.hpp"
#include "particle/change_set.hpp"
#include "platform/simulator.hpp"

// Manual includes
#include "particle/ensemble.hpp"
// =-
namespace ion_channel {

//Create a particle addition/remove trial
std::unique_ptr< particle::change_set > remove_choice_cell::generate(platform::simulator & sys) 
{
  std::unique_ptr < trial::change_set > result (new trial::change_set (*this));
  // REMOVE
  // ------
  auto specie_count = sys.get_specie( this->key().key ).count();
  if (specie_count == 0)
  {
    result->set_fail(); // No particles of this specie
    return result;
  }
  const auto& ens = sys.get_ensemble();
  trial::change_atom atom; // change being built
  atom.key = this->key().key;
  atom.do_new = false;
  // Select random particle of target specie
  atom.index = ens.nth_specie_index (atom.key, sys.get_random().randint( 0, specie_count - 1));
  // Set old position
  atom.old_position = ens.position (atom.index);
  atom.eps_old = ens.eps( atom.index );
  
  result->update_exponential_factor( -sys.get_specie( atom.key ).chemical_potential() );
  result->update_probability_factor( specie_count / this->region_.volume() );
  
  result->add_atom( atom );
  return result;

}

std::string remove_choice_cell::label() const 
{
  return "-- Del";
}

remove_choice_cell::~remove_choice_cell() 
{
}


} // namespace ion_channel

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(ion_channel::remove_choice_cell, "ion_channel::remove_choice_cell");