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

#include "core/input_base_meta.hpp"
#include "core/input_base_reader.hpp"
#include "core/input_help.hpp"

// manual includes
#include "core/strngs.hpp"
#include "utility/utility.hpp"
// --
namespace core {

input_base_meta::input_base_meta(std::string label, bool multiple, bool required)
: section_label_ (label)
, multiple_ (multiple)
, required_ (required)
{
  UTILITY_CHECK( core::strngs::is_valid_name(label), "section label must be single word without spaces or numbers" );
}

// Interpret a section  of the input file.
//
// throw an error if input file is incorrect (using UTILITY_INPUT macro)
void input_base_meta::read_section(input_base_reader & reader)
{
  // Process the section header
  UTILITY_REQUIRE( 0 == reader.name().find( this->section_label() ), "Input section label \""+reader.name()+"\" was not the expected label \""+this->section_label()+"\"" );
  if( not reader.value().empty() )
  {
    reader.parameter_value_error( "Section start entry", this->section_label(), "Remove or comment text after section start keyword \"" + reader.name() + "\"." );
  }
  // Reset state at start of read
  this->do_reset();
  // Process section
  while ( reader.next() )
  {
    if ( reader.name().find(core::strngs::fsend()) == 0) break;
    if ( not this->do_read_entry( reader ) )
    {
      reader.invalid_parameter_error( this->section_label(), reader.name() );
    }
  }
  this->do_read_end( reader );

}


} // namespace core
