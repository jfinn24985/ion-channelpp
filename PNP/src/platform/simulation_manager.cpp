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

#include "platform/simulation_manager.hpp"
#include "core/input_document.hpp"
#include "platform/simulation.hpp"

// manual includes
#include "core/strngs.hpp"
// -
// -
namespace platform {

simulation_manager::~simulation_manager()
{}

// Details about the current simulation to be written to the
// log at the start of the simulation.
void simulation_manager::description(std::ostream & os) const 
{
  os << core::strngs::horizontal_bar() << "\n";
  this->do_description( os );
  os << " thermalization interval : " << this->equilibration_interval() << "\n";
  os << "     production interval : " << this->production_interval() << "\n";

}

// Write data of simulation manager object into the input
// document object at the given section.
void simulation_manager::write_part_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fsnavr(), this->equilibration_interval() );
  wr[ ix ].add_entry( core::strngs::fsnstp(), this->production_interval() );
  this->do_write_part_document( wr, ix );

}


} // namespace platform
