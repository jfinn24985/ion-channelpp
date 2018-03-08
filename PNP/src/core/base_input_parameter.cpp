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

#include "core/base_input_parameter.hpp"
#include "core/input_base_reader.hpp"

// manual includes
#include "core/input_parameter_memo.hpp"
#include "utility/utility.hpp"
// -
namespace core {

// Attempt to read input value as a positive integer number. Throw consistently
// formatted error message on failure.
//
// see input_base_reader::ordinal_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \return : the resulting ordinal

std::size_t base_input_parameter::get_ordinal(std::string title, std::string section_name) const
{
  std::size_t result{ 0 };
  input_base_reader::ordinal_input( *this, title, section_name, result );
  return result;

}

// Attempt to read input value as a boolean. Throw consistently
// formatted error message on failure. 
//
// see input_base_reader::bool_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \parma has_default : is an empty value an error
// \param default_value : what is result when value is empty (if has_default)

bool base_input_parameter::get_bool(std::string title, std::string section_name, bool has_default, bool default_value) const
{
  bool result { false };
  input_base_reader::bool_input( *this, title, section_name, result, has_default, default_value );
  return result;

}

// Attempt to read an input value as a floating point number. Throw consistently
// formatted error message on failure.
//
// see input_base_reader::float_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \parma above_zero : whether value should be positive
// \param equal_zero : if above_zero is true, allows value to also equal zero 

double base_input_parameter::get_float(std::string title, std::string section_name, bool above_zero, bool equal_zero) const
{
  double result{ 0 };
  input_base_reader::float_input( *this, title, section_name, result, above_zero, equal_zero );
  return result;

}

// Attempt to read input value as a text value, an empty string is an error. Throw consistently
// formatted error message on failure.
//
// see input_base_reader::text_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \return : the resulting text

std::string base_input_parameter::get_text(std::string title, std::string section_name) const
{
  std::string result;
  input_base_reader::text_input( *this, title, section_name, result );
  return result;

}

// Attempt to read input value as a key value matching a list of valid keys. The
// index of the matching key in the list is returned. Throw consistently
// formatted error message on failure.
//
// see input_base_reader::key_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \return : the index in the give key list

std::size_t base_input_parameter::get_key(std::string title, std::string section_name, const std::vector< std::string >& keylist) const
{
  std::size_t result{0};
  input_base_reader::key_input( *this, title, section_name, result, keylist );
  return result;

}

// Attempt to read an input value as a series of floating point numbers. Throw 
// consistently formatted error message on failure.
//
// see input_base_reader::floats_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : vector containing result

void base_input_parameter::get_floats(std::string title, std::string section_name, std::vector< double >& target) const
{
  input_base_reader::floats_input( *this, title, section_name, target );

}

// Attempt to read an input line as a series of floating point numbers. Throw consistently
// formatted error message on failure.
//
// see input_base_reader::floats_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : vector containing result

void base_input_parameter::get_line_as_floats(std::string name, std::string title, std::string section_name, std::vector< double >& target) const
{
  UTILITY_REQUIRE( not name.empty(), "Require name for parameter." );
  input_parameter_memo param( *this );
  param.set_name( name );
  param.set_value( param.line() );
  input_base_reader::floats_input( param, title, section_name, target );

}

bool base_input_parameter::equal(const base_input_parameter & other) const
{
  return name() == other.name() and value() == other.value() and line() == other.line()
     and filename() == other.filename() and line_number() == other.line_number();
}


} // namespace core
