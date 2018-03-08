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

#include "cscpbc/jump_choice_pbc.hpp"
#include "utility/archive.hpp"

#include "particle/ensemble.hpp"
#include "particle/change_set.hpp"
#include "platform/simulator.hpp"

// Manual includes
#include "core/strngs.hpp"
#include "cscpbc/periodic_system.hpp"
// -
namespace periodic_cube {

jump_choice_pbc::jump_choice_pbc(std::size_t ispec, const std::map< std::string, std::string >& params)
: base_choice( { ispec, 1, 1, 1 } )
{
  UTILITY_INPUT( params.size() == 0 or
           ( 1 == params.size() and 1 == params.count( core::strngs::fsspec() ) ), "Jump trial requires no extra parameters", core::strngs::fstry(), nullptr );

}

jump_choice_pbc::~jump_choice_pbc() 
{}

std::unique_ptr< particle::change_set > jump_choice_pbc::generate(platform::simulator & sys) 
{
  std::unique_ptr< trial::change_set > resultset( new trial::change_set( *this ) );
  auto const& ens = sys.get_ensemble();
  trial::change_atom atom;
  atom.key = this->key().key;
  // Select a particle at random
  switch (sys.get_specie( atom.key ).count())
  {
  case 0:
    {
      resultset->set_fail(); // No particles of this specie
      return resultset;
    }
    break;
  case 1:
    {
      atom.index = ens.nth_specie_index( atom.key, 0 );
    }
    break;
  default:
    {
      atom.index = ens.nth_specie_index( atom.key, sys.get_random().randint( 0, sys.get_specie( atom.key ).count() - 1) );
    }
    break;
  }
  // Get old position
  atom.old_position = ens.position(atom.index);
  atom.eps_old = ens.eps( atom.index );
  const double cell_length{ dynamic_cast< periodic_cube::periodic_system& >( sys ).length() };
  // New random position within boundary
  atom.new_position.x = sys.get_random().uniform( 0.0, cell_length );
  atom.new_position.y = sys.get_random().uniform( 0.0, cell_length );
  atom.new_position.z = sys.get_random().uniform( 0.0, cell_length );
  // No need to check for valid position because the way we generate the new
  // position should always give a valid position.
  atom.eps_new = sys.permittivity_at( atom.new_position );
  resultset->add_atom (atom);
  return resultset;

}

//Name for this trial
std::string jump_choice_pbc::label() const 
{
  return "Jump";
}


} // namespace periodic_cube

BOOST_CLASS_EXPORT_IMPLEMENT(periodic_cube::jump_choice_pbc);