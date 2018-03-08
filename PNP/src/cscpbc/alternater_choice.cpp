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

#include "cscpbc/alternater_choice.hpp"
#include "particle/change_set.hpp"
#include "platform/simulator.hpp"

// Manual includes
#include "particle/ensemble.hpp"
#include "cscpbc/periodic_system.hpp"
// -
namespace periodic_cube {

alternater_choice::~alternater_choice() 
{}

alternater_choice::alternater_choice(std::size_t ispec) 
: gc_choice_pbc(ispec)
{}


//Generate a new change set based on this choice type.
std::unique_ptr< particle::change_set > alternater_choice::generate(platform::simulator & sys) 
{
  const double real_target_{ sys.get_specie(this->key().key).concentration() * sys.volume(0ul) / core::constants::to_SI() };
  if (sys.get_specie(this->key().key).count() < real_target_)
  {
    return this->get_adder().generate (sys);
  }
  else
  {
    return this->get_remover().generate (sys);
  }

}


} // namespace periodic_cube
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(periodic_cube::alternater_choice, "periodic_cube::alternater_choice");