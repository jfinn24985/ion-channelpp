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

#include "evaluator/check_data.hpp"
#include "evaluator/induced_charge.hpp"
#include "cscchannel/channel_system.hpp"

namespace evaluator {

// Set object's data from source data.
void check_data::set_data(const induced_charge & icc, const ion_channel::channel_system & sim)
{
  this->set_nsub( icc.get_nsub0() );
  this->set_dxf( icc.get_dxf() );
  this->set_dxw( icc.get_dxw() );
  this->set_pore_hlength( sim.pore_hlength() );
  this->set_cell_hlength( sim.cell_hlength() );
  this->set_pore_radius( sim.pore_radius() );
  this->set_protein_radius( sim.protein_radius() );
  this->set_cell_radius( sim.cell_radius() );
  this->set_vestibule_arc( sim.vestibule_arc_radius() );
  this->set_membrane_arc( sim.membrane_arc_radius() );
  this->set_permittivity( sim.permittivity() );
  this->set_protein_permittivity( sim.protein_permittivity() );

}

// Compare source data with this object
bool check_data::compare(const induced_charge & icc, const ion_channel::channel_system & sim) const
{
  return this->nsub == this->hash_dbl( icc.get_nsub0() )
         and this->dxf == this->hash_dbl( icc.get_dxf() )
         and this->dxw == this->hash_dbl( icc.get_dxw() )
         and this->pore_hlength == this->hash_dbl( sim.pore_hlength() )
         and this->cell_hlength == this->hash_dbl( sim.cell_hlength() )
         and this->pore_radius == this->hash_dbl( sim.pore_radius() )
         and this->protein_radius == this->hash_dbl( sim.protein_radius() )
         and this->cell_radius == this->hash_dbl( sim.cell_radius() )
         and this->vestibule_arc == this->hash_dbl( sim.vestibule_arc_radius() )
         and this->membrane_arc == this->hash_dbl( sim.membrane_arc_radius() )
         and this->permittivity == this->hash_dbl( sim.permittivity() )
         and this->protein_permittivity == this->hash_dbl( sim.protein_permittivity() );

}

// Compare two check data objects
bool check_data::equivalent(const check_data & other) const
{
  return this->nsub == other.nsub
         and this->dxf == other.dxf
         and this->dxw == other.dxw
         and this->pore_hlength == other.pore_hlength
         and this->cell_hlength == other.cell_hlength
         and this->pore_radius == other.pore_radius
         and this->protein_radius == other.protein_radius
         and this->cell_radius == other.cell_radius
         and this->vestibule_arc == other.vestibule_arc
         and this->membrane_arc == other.membrane_arc
         and this->permittivity == other.permittivity
         and this->protein_permittivity == other.protein_permittivity;

}

// Write data to stream.
void check_data::write_data(std::ostream & os) const
{
  // Write check values
  os << core::strngs::comment_begin() << " " << this->nsub << " "
     << this->dxf << " " << this->dxw << " " << core::strngs::comment_end() << "\n";
  os << core::strngs::comment_begin() << " " << this->pore_hlength << " "
     << this->cell_hlength << " " << core::strngs::comment_end() << "\n";
  os << core::strngs::comment_begin() << " " << this->pore_radius << " "
     << this->protein_radius << " " << this->cell_radius << " "
     << core::strngs::comment_end() << "\n";
  os << core::strngs::comment_begin() << " " << this->vestibule_arc << " "
     << this->membrane_arc << " " << this->permittivity << " "
     << this->protein_permittivity << " " << core::strngs::comment_end() << "\n\n";
  

}

// Read data from a stream. Return false if problems reading the stream.
bool check_data::read_data(std::istream & input)
{
  // Read check variables.
  std::string cmnt;
  
  input >> cmnt >> this->nsub >> this->dxf >> this->dxw;
  if ( cmnt != core::strngs::comment_begin() ) return false;
  std::getline( input, cmnt );
  input >> cmnt >> this->pore_hlength >> this->cell_hlength;
  if ( cmnt != core::strngs::comment_begin() ) return false;
  std::getline( input, cmnt );
  input >> cmnt >> this->pore_radius >> this->protein_radius >> this->cell_radius;
  if ( cmnt != core::strngs::comment_begin() ) return false;
  std::getline( input, cmnt );
  input >> cmnt >> this->vestibule_arc >> this->membrane_arc >> this->permittivity >> this->protein_permittivity;
  if ( cmnt != core::strngs::comment_begin() ) return false;
  std::getline( input, cmnt );
  

}

// Compare two check data objects
bool check_data::swap(const check_data & other) const
{
  std::swap( this->nsub, other.nsub );
  std::swap( this->dxf, other.dxf );
  std::swap( this->dxw, other.dxw );
  std::swap( this->pore_hlength, other.pore_hlength );
  std::swap( this->cell_hlength, other.cell_hlength );
  std::swap( this->pore_radius, other.pore_radius );
  std::swap( this->protein_radius, other.protein_radius );
  std::swap( this->cell_radius, other.cell_radius );
  std::swap( this->vestibule_arc, other.vestibule_arc );
  std::swap( this->membrane_arc, other.membrane_arc );
  std::swap( this->permittivity, other.permittivity );
  std::swap( this->protein_permittivity, other.protein_permittivity; );

}


} // namespace evaluator
