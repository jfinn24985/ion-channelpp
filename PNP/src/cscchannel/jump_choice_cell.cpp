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

#include "cscchannel/jump_choice_cell.hpp"
#include "particle/ensemble.hpp"
#include "cscchannel/channel_system.hpp"
#include "particle/change_set.hpp"
#include "platform/simulator.hpp"

// Manual includes
#include "core/strngs.hpp"
// =
namespace ion_channel {

//
//Choice hash subtype for jump move is "1", so 
//pattern for jump move is { X, 1, 1, 1 }.
jump_choice_cell::jump_choice_cell(std::size_t ispec, std::map< std::string, std::string> const& params, const channel_system & geom)
: base_choice( { ispec, 1, 1, 1 } )
, region_( geom.cell_radius() - geom.get_specie( ispec ).radius()
      , geom.cell_hlength() - geom.get_specie( ispec ).radius() )
{
   UTILITY_INPUT( params.empty() or
         ( 1 == params.size() and 1 == params.count( core::strngs::fsspec() ) ), 
         "Jump choice has no non-standard parameters.", 
         core::strngs::fstry(), nullptr );
}  


jump_choice_cell::~jump_choice_cell() 
{}

std::unique_ptr< particle::change_set > jump_choice_cell::generate(platform::simulator & sys) 
{
  std::unique_ptr< trial::change_set > resultset (new trial::change_set (*this));
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
  // give particle a random position within subregion
  this->region_.generate( atom.new_position, sys.get_random() );
  
  if (not sys.is_valid_position( atom.new_position, atom.key ))
  {
    resultset->set_fail ();
  }
  atom.eps_new = sys.permittivity_at( atom.new_position );
  resultset->add_atom (atom);
  return resultset;

}

//Name for this trial
std::string jump_choice_cell::label() const 
{
  return "Jump";
}


} // namespace ion_channel

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(ion_channel::jump_choice_cell, "ion_channel::jump_choice_cell");