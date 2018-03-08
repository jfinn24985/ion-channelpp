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

#include "geometry/coordinate.hpp"
#include "core/base_input_parameter.hpp"

// - manuals
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
// -
namespace geometry {

// Attempt to read an input value as a floating point number
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error
// \parma above_zero : whether value should be positive
// \param equal_zero : if above_zero is true, allows value to also equal zero 

void coordinate::coordinate_input(std::string name, std::string value, std::string title, std::string section_name, coordinate & target, const core::input_base_reader * reader)
{
  std::vector< double > xyz;
  core::input_base_reader::floats_input( name, value, title, section_name, xyz, reader );
  if( xyz.size() != 3 )
  {
    throw core::input_error::parameter_value_error( title, section_name, name, value, reader, coordinate::set_of_three_error() );
  }
  geometry::coordinate tmp( xyz[0], xyz[1], xyz[2] );
  std::swap( tmp, target );

}

// Attempt to read an input value as a coordinate (x,y,z)
//
// \param param : input parameter
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error

void coordinate::coordinate_input(const core::base_input_parameter & param, std::string title, std::string section_name, coordinate & target)
{
  std::vector< double > xyz;
  core::input_base_reader::floats_input( param, title, section_name, xyz );
  if( xyz.size() != 3 )
  {
    throw core::input_error::parameter_value_error( title, section_name, param, coordinate::set_of_three_error() );
  }
  geometry::coordinate tmp( xyz[0], xyz[1], xyz[2] );
  std::swap( tmp, target );

}


} // namespace geometry
