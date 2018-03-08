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

#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/base_input_parameter.hpp"

// manual includes
#include "core/input_parameter_memo.hpp"
#include "core/input_parameter_reference.hpp"
#include "utility/utility.hpp"
// -
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
//--
namespace core {

// --------------------------------------------------
// Observer methods
// --------------------------------------------
// Get the current line (excluding comments)
std::string input_base_reader::trimmed_line() const
{
std::string line = decomment(this->line_, '#');
boost::algorithm::trim( line );
return line;

}

// ----------------------------------------
// mutating methods
// ----------------------------------------
//Clear value of 'line', 'name' and 'value'
void input_base_reader::clear()
{
   this->line_.clear();
   this->name_.clear();
   this->value_.clear();
}

//Set the current line and split into name and value.
//
//\post line() = line
bool input_base_reader::set_line(std::string line)
{
  // Split line
  this->name_.clear();
  this->value_.clear();
  this->line_ = line;
  
  // Preprocess the line
  if ( line.empty () ) return false;
  line = decomment(line, '#');
  if ( line.empty () ) return false;
  boost::algorithm::trim( line );
  if ( line.empty () ) return false;
    
  // (know that first character is non-whitespace)
  
  // have name/value ?? (look for ' ' or '=')
  std::string::size_type ipos = line.find('=');
  if ( ipos == std::string::npos )
  {
     // No '=', try split on a space
     for ( ipos = 0; ipos != line.size(); ++ipos )
     {
        if ( std::isspace( line[ipos] ) ) break;
     }
     if ( ipos == line.size() )
     {
        //     have only name
        this->name_ = line;
     }
     else
     {
        //     name and value
        this->name_ = line.substr( 0, ipos );
        this->value_ = line.substr( ipos + 1 );
     } // if
  }
  else
  {
     //     name and value
     this->name_ = line.substr( 0, ipos );
     this->value_ = line.substr( ipos + 1 );
  }
  if ( not this->name_.empty() ) boost::algorithm::trim( this->name_ );
  if ( not this->value_.empty() ) boost::algorithm::trim( this->value_ );
  return true;

}

//----------------------------------------------------------------------
// Decomment a string
//
// Attempt to remove comments from a string. Resulting string may be
// empty. Whitespace before the comment character is unchanged.
//
// returns the processed string
//
// |# abc  | -> empty
// | # abc  | -> | |
// | #abc| -> | |
// |abc  | unchanged if no comment character present
// |abc  #| -> |abc  |

std::string input_base_reader::decomment(std::string a_val, char comment_start)
{
enum State
{
  START
  , SINGLE
  , DOUBLE
};
State state = START;

for (std::size_t pos = 0; pos < a_val.size(); ++pos)
{
  std::size_t esc = 0;
  while (pos != a_val.size() and a_val[pos] == '\\')
  {
    ++pos;
    ++esc;
  };
  if (pos == a_val.size())
  {
    return a_val;
  }
  if ((esc & 1) == 1) // Pointing to character after an odd escape
  {
    ++pos;
    continue;
  }

  switch (state)
  {
  case SINGLE:
    if (a_val[pos] == '\'') state = START;
    break;

  case DOUBLE:
    if (a_val[pos] == '\"') state = START;
    break;

  default:
    switch (a_val[pos])
    {
    case '\'':
      state = SINGLE;
      break;
    case '\"':
      state = DOUBLE;
      break;
    default:
      if (a_val[pos] != comment_start) break;
      return a_val.substr(0, pos);
    }
  }
}
return a_val;

}

// ----------------------------------------------------------------------
// Dequote a string
//
// Attempt to remove leading and trailing quotes from a string.
// Returns altered string or original value if no quotes.
//
// Quotes are the characters ' or " and they must match to be
// removed. A slash character has no special meaning before a
// quote. Leading and trailing whitespace outside the quotes is
// also removed, but whitespace in the quotes is unchanged.
//
// |'abc  '| -> |abc  |
// |"abc  "| -> |abc  |
// |"abc  '| unchanged
// |"abc  " | -> |abc  |
// |"abc  ' | unchanged

std::string input_base_reader::dequote(std::string a_val)

{
std::string result(a_val);
boost::algorithm::trim(result);
const size_t length(result.size());
if ((length > 1) and (result[0] == result[length - 1]) and (result[0] == '\'' or result[0] == '"'))
{
  size_t esc = 0; // check for odd number of escapes
  while (result[length - (2 + esc)] == '\\') ++esc;
  if ((esc & 1) == 0) // even so last quote not escaped
  {
    return result.substr(1, length - 2);
  }
}
return a_val;

}

// ----------------------------------------------------------------------
// Convert a string of numbers into a double array
//
// NOTE: this method assumes that the only spacing characters in strval
// will be spaces or tabs
//
// \param strval : the line of numbers 
// \param a_arry : the target array
// \param a_size : the maximum number of values to read
// \param a_cnt : the actual number read
// \param a_istt : iostat code of last read, /= 0 means error

bool input_base_reader::read_as_floats(std::string input_buffer, std::vector< double > & a_arry)

{
// Convert any 'd's to 'e's
std::replace(input_buffer.begin(), input_buffer.end(), 'D', 'E');
std::replace(input_buffer.begin(), input_buffer.end(), 'd', 'E');
a_arry.clear();
{
   std::stringstream is(input_buffer);
   while( not is.eof() )
   {
      double val;
      is >> val;
      // Conversion failed so no more numbers for conversion ?
      if (not is) break;
      a_arry.push_back(val);
   }
   return is.eof();
}

}

//Convert textual representation of true/false into bool.
//
//Look for (english) textual representation of boolean.
//
//(1) find 'true' or 'false' ignoring case
//
//(2) if (1) fails attempt stringstream(a_val) >> a_res

bool input_base_reader::read_as_bool(std::string astr, bool & target)
{
  astr = dequote( astr );
  boost::trim( astr );
  boost::to_upper( astr );
  switch (astr.size())
  {
  case 1:
    if (astr == "1")
    {
      target = true;
      return true;
    }
    else if (astr == "0")
    {
      target = false;
      return true;
    }
    break;
  case 4:
    if (astr == "TRUE")
    {
      target = true;
      return true;
    }
    break;
  case 5:
    if (astr == "FALSE")
    {
      target = false;
      return true;
    }
    break;
  case 6:
    if (astr == ".TRUE.")
    {
      target = true;
      return true;
    }
    break;
  case 7:
    if (astr == ".FALSE.")
    {
      target = false;
      return true;
    }
    break;
  default:
    break;
  }
  return false;

}

// Attempt to read an input value as a floating point number
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error
// \parma has_default : is an empty value an error
// \param default_value: what is result when value is empty (if has_default)
// \param reader : reader context pointer

void input_base_reader::bool_input(std::string name, std::string value, std::string title, std::string section_name, bool & target, bool has_default, bool default_value, const input_base_reader * reader)
{
  if( value.empty() )
  {
    if( has_default )
    {
      target = default_value;
      return;
    }
    else
    {
      if( value.empty() )
      {
        throw input_error::missing_value_error( title, section_name, name, reader );
      }
    }
  }
  else
  {
    std::string astr{ dequote( value ) };
    boost::trim( astr );
    boost::to_upper( astr );
    if( astr.empty() )
    {
      if( has_default )
      {
        target = default_value;
        return;
      }
      else
      {
        if( astr.empty() )
        {
          throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::non_empty_value_message() );
        }
      }
    }
    switch (astr.size())
    {
    case 1:
      if (astr == "1")
      {
        target = true;
        return;
      }
      else if (astr == "0")
      {
        target = false;
        return;
      }
      else if (astr == "F")
      {
        target = false;
        return;
      }
      else if (astr == "T")
      {
        target = true;
        return;
      }
      break;
    case 3:
      if (astr == ".F.")
      {
        target = false;
        return;
      }
      else if (astr == ".T.")
      {
        target = true;
        return;
      }
      break;
    case 4:
      if (astr == "TRUE")
      {
        target = true;
        return;
      }
      break;
    case 5:
      if (astr == "FALSE")
      {
        target = false;
        return;
      }
      break;
    case 6:
      if (astr == ".TRUE.")
      {
        target = true;
        return;
      }
      break;
    case 7:
      if (astr == ".FALSE.")
      {
        target = false;
        return;
      }
      break;
    default:
      break;
    }
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::bad_boolean_message() );
  }
  

}

// Attempt to read an input value as a floating point number
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error
// \parma above_zero : whether value should be positive
// \param equal_zero : if above_zero is true, allows value to also equal zero 

void input_base_reader::float_input(std::string name, std::string value, std::string title, std::string section_name, double & target, bool above_zero, bool equal_zero, input_base_reader const* reader)
{
  if( value.empty() )
  {
    throw input_error::missing_value_error( title, section_name, name, reader );
  }
  std::replace(value.begin(), value.end(), 'D', 'E');
  std::replace(value.begin(), value.end(), 'd', 'E');
  const char *start = value.c_str();
  // Move past any whitespace
  while( not *start == '\0' and std::isspace( *start ) ) ++start;
  if( *start == '\0')
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::non_empty_value_message() );
  }
  char *end;
  double result;
  errno = 0;
  result = std::strtod(start, &end);
  if( errno == ERANGE )
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_outside_range_message() );
  }
  if( std::isnan( result ) )
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_nan_message() );
  }
  if( std::isinf( result ) )
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_inf_message() );
  }
  if( *end != '\0' )
  {
    while( not *end == '\0' and std::isspace( *end ) ) ++end;
    if( *end != '\0' )
    {
      throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_bad_message() );
    }
  }
  if( above_zero )
  {
    if( equal_zero )
    {
      if ( result < 0.0 )
      {
        throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_negative_message() );
      }
    }
    else if ( result <= 0.0 )
    {
      throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_zero_message() );
    }
  }
  target = result;

}

// Attempt to read an input value as a floating point number
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error
// \parma above_zero : whether value should be positive
// \param equal_zero : if above_zero is true, allows value to also equal zero 

void input_base_reader::floats_input(std::string name, std::string value, std::string title, std::string section_name, std::vector< double > & target, input_base_reader const* const reader)
{
  if( value.empty() )
  {
    throw input_error::missing_value_error( title, section_name, name, reader );
  }
  const std::string original{ value };
  std::replace(value.begin(), value.end(), 'D', 'E');
  std::replace(value.begin(), value.end(), 'd', 'E');
  const char *start = value.c_str();
  std::vector< double > rlist;
  // Move past any whitespace
  while( not *start == '\0' and std::isspace( *start ) ) ++start;
  if( *start == '\0')
  {
    throw input_error::parameter_value_error( title, section_name, name, original, reader, input_error::non_empty_value_message() );
  }
  char *end;
  errno = 0;
  for (double result = std::strtod(start, &end); start != end; result = std::strtod(start, &end))
  {
    if( errno == ERANGE )
    {
      while( not *start == '\0' and std::isspace( *start ) ) ++start;
      std::string element( start, std::size_t(end - start) );
      throw input_error::parameter_value_error( title, section_name, name, original, reader, "Element " + element + ": " + input_error::number_outside_range_message() );
    }
    if( std::isnan( result ) )
    {
      while( not *start == '\0' and std::isspace( *start ) ) ++start;
      std::string element( start, std::size_t(end - start) );
      throw input_error::parameter_value_error( title, section_name, name, original, reader, "Element " + element + ": " + input_error::number_nan_message() );
    }
    if( std::isinf( result ) )
    {
      while( not *start == '\0' and std::isspace( *start ) ) ++start;
      std::string element( start, std::size_t(end - start) );
      throw input_error::parameter_value_error( title, section_name, name, original, reader, "Element " + element + ": " + input_error::number_inf_message() );
    }
    start = end;
    while( not *start == '\0' and std::isspace( *start ) ) ++start;
    rlist.push_back( result );
    errno = 0;
  }
  if( *end != '\0' and not std::isspace( *end ) )
  {
    std::string element( start ); // may be list but only want first element.
    std::stringstream s( element );
    s >> element;
    throw input_error::parameter_value_error( title, section_name, name, original, reader, "Element " + element + ": " + input_error::number_bad_message() );
  }
  std::swap( rlist, target );

}

// Attempt to read an input value as a textual key. Sets target
// to matching index in key list.
//
// NOTES
//  * an empty string is an error (ie no value or "")
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error

void input_base_reader::key_input(std::string name, std::string value, std::string title, std::string section_name, std::size_t & target, const std::vector< std::string >& keylist, const input_base_reader * reader)
{
  if( value.empty() )
  {
    throw input_error::missing_value_error( title, section_name, name, reader );
  }
  // Remove quotes and outer whitespace.
  std::string protokey = input_base_reader::dequote( value );
  boost::algorithm::trim( protokey );
  if( protokey.empty() )
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::non_empty_value_message() );
  }
  for( std::size_t idx = 0; idx != keylist.size(); ++idx)
  {
    if( keylist[ idx ].find( protokey ) == 0 )
    {
      target = idx;
      return;
    }
  }
  // If here then key not found.
  std::stringstream os;
  os << "(";
  for( auto key : keylist )
  {
    os << key << ",";
  }
  std::string list{ os.str() };
  list.back() = ')';
  throw input_error::parameter_value_error( title, section_name, name, value, reader, (input_error::bad_key_message_format() % list).str() );

}

// Attempt to read an input value as a positive integer number
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error

void input_base_reader::ordinal_input(std::string name, std::string value, std::string title, std::string section_name, std::size_t & target, const input_base_reader * reader)
{
  if( value.empty() )
  {
    throw input_error::missing_value_error( title, section_name, name, reader );
  }
  const char *start = value.c_str();
  // Move past any whitespace
  while( not *start == '\0' and std::isspace( *start ) ) ++start;
  if( *start == '\0')
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::non_empty_value_message() );
  }
  // Check for negative sign
  if( *start == '-' )
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_negative_message() );
  }
  char *end;
  std::size_t result;
  errno = 0;
  // Expect base ten.
  result = std::strtoul(start, &end, 0);
  if( errno == ERANGE )
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_outside_range_message() );
  }
  if( *end != '\0' and not std::isspace( *end ) )
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::number_bad_message() );
  }
  target = result;

}

// Attempt to read an input value as a text value
//
// NOTES:
// *  dequotes the string but does not trim the dequoted string
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error

void input_base_reader::text_input(std::string name, std::string value, std::string title, std::string section_name, std::string & target, const input_base_reader * reader)
{
  if( value.empty() )
  {
    throw input_error::missing_value_error( title, section_name, name, reader );
  }
  // Remove quotes.
  std::string result = input_base_reader::dequote( value );
  if( result.empty() )
  {
    throw input_error::parameter_value_error( title, section_name, name, value, reader, input_error::non_empty_value_message() );
  }
  target = result;

}

// Attempt to read an input value as a floating point number
//
// \param param : input parameter
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error
// \parma has_default : is an empty value an error
// \param default_value: what is result when value is empty (if has_default)

void input_base_reader::bool_input(const base_input_parameter & param, std::string title, std::string section_name, bool & target, bool has_default, bool default_value)
{
  if( param.value().empty() )
  {
    if( has_default )
    {
      target = default_value;
      return;
    }
    else
    {
      if( param.value().empty() )
      {
        throw input_error::missing_value_error( title, section_name, param );
      }
    }
  }
  else
  {
    std::string astr{ dequote( param.value() ) };
    boost::trim( astr );
    boost::to_upper( astr );
    if( astr.empty() )
    {
      if( has_default )
      {
        target = default_value;
        return;
      }
      else
      {
        if( astr.empty() )
        {
          throw input_error::parameter_value_error( title, section_name, param, input_error::non_empty_value_message() );
        }
      }
    }
    switch (astr.size())
    {
    case 1:
      if (astr == "1")
      {
        target = true;
        return;
      }
      else if (astr == "0")
      {
        target = false;
        return;
      }
      else if (astr == "F")
      {
        target = false;
        return;
      }
      else if (astr == "T")
      {
        target = true;
        return;
      }
      break;
    case 3:
      if (astr == ".F.")
      {
        target = false;
        return;
      }
      else if (astr == ".T.")
      {
        target = true;
        return;
      }
      break;
    case 4:
      if (astr == "TRUE")
      {
        target = true;
        return;
      }
      break;
    case 5:
      if (astr == "FALSE")
      {
        target = false;
        return;
      }
      break;
    case 6:
      if (astr == ".TRUE.")
      {
        target = true;
        return;
      }
      break;
    case 7:
      if (astr == ".FALSE.")
      {
        target = false;
        return;
      }
      break;
    default:
      break;
    }
    throw input_error::parameter_value_error( title, section_name, param, input_error::bad_boolean_message() );
  }
  

}

// Attempt to read an input value as a floating point number
//
// \param param : input parameter
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error
// \parma above_zero : whether value should be positive
// \param equal_zero : if above_zero is true, allows value to also equal zero 

void input_base_reader::float_input(const base_input_parameter & param, std::string title, std::string section_name, double & target, bool above_zero, bool equal_zero)
{
  if( param.value().empty() )
  {
    throw input_error::missing_value_error( title, section_name, param );
  }
  std::string value{ param.value() };
  std::replace(value.begin(), value.end(), 'D', 'E');
  std::replace(value.begin(), value.end(), 'd', 'E');
  const char *start = value.c_str();
  // Move past any whitespace
  while( not *start == '\0' and std::isspace( *start ) ) ++start;
  if( *start == '\0')
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::non_empty_value_message() );
  }
  char *end;
  double result;
  errno = 0;
  result = std::strtod(start, &end);
  if( errno == ERANGE )
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::number_outside_range_message() );
  }
  if( std::isnan( result ) )
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::number_nan_message() );
  }
  if( std::isinf( result ) )
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::number_inf_message() );
  }
  if( *end != '\0' )
  {
    while( not *end == '\0' and std::isspace( *end ) ) ++end;
    if( *end != '\0' )
    {
      throw input_error::parameter_value_error( title, section_name, param, input_error::number_bad_message() );
    }
  }
  if( above_zero )
  {
    if( equal_zero )
    {
      if ( result < 0.0 )
      {
        throw input_error::parameter_value_error( title, section_name, param, input_error::number_negative_message() );
      }
    }
    else if ( result <= 0.0 )
    {
      throw input_error::parameter_value_error( title, section_name, param, input_error::number_zero_message() );
    }
  }
  target = result;

}

// Attempt to read an input value as a sequence of floating point numbers
//
// \param param : input parameter
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error
// \param name : whether to convert value (empty string) or whole line (non-empty
//               string). When converting whole line, use name as parameter name.

void input_base_reader::floats_input(const base_input_parameter & param, std::string title, std::string section_name, std::vector< double > & target)
{
  if( param.value().empty() )
  {
    throw input_error::missing_value_error( title, section_name, param );
  }
  std::string value{ param.value() };
  std::replace(value.begin(), value.end(), 'D', 'E');
  std::replace(value.begin(), value.end(), 'd', 'E');
  const char *start = value.c_str();
  std::vector< double > rlist;
  // Move past any whitespace
  while( not *start == '\0' and std::isspace( *start ) ) ++start;
  if( *start == '\0')
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::non_empty_value_message() );
  }
  char *end;
  errno = 0;
  for (double result = std::strtod(start, &end); start != end; result = std::strtod(start, &end))
  {
    if( errno == ERANGE )
    {
      while( not *start == '\0' and std::isspace( *start ) ) ++start;
      std::string element( start, std::size_t(end - start) );
      throw input_error::parameter_value_error( title, section_name, param, "Element " + element + ": " + input_error::number_outside_range_message() );
    }
    if( std::isnan( result ) )
    {
      while( not *start == '\0' and std::isspace( *start ) ) ++start;
      std::string element( start, std::size_t(end - start) );
      throw input_error::parameter_value_error( title, section_name, param, "Element " + element + ": " + input_error::number_nan_message() );
    }
    if( std::isinf( result ) )
    {
      while( not *start == '\0' and std::isspace( *start ) ) ++start;
      std::string element( start, std::size_t(end - start) );
      throw input_error::parameter_value_error( title, section_name, param, "Element " + element + ": " + input_error::number_inf_message() );
    }
    start = end;
    while( not *start == '\0' and std::isspace( *start ) ) ++start;
    rlist.push_back( result );
    errno = 0;
  }
  if( *end != '\0' and not std::isspace( *end ) )
  {
    std::string element( start ); // may be list but only want first element.
    std::stringstream s( element );
    s >> element;
    throw input_error::parameter_value_error( title, section_name, param, "Element " + element + ": " + input_error::number_bad_message() );
  }
  std::swap( rlist, target );

}

// Attempt to read an input value as a textual key. Sets target
// to matching index in key list.
//
// NOTES
//  * an empty string is an error (ie no value or "")
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error

void input_base_reader::key_input(const base_input_parameter & param, std::string title, std::string section_name, std::size_t & target, const std::vector< std::string >& keylist)
{
  if( param.value().empty() )
  {
    throw input_error::missing_value_error( title, section_name, param );
  }
  // Remove quotes and outer whitespace.
  std::string protokey = input_base_reader::dequote( param.value() );
  boost::algorithm::trim( protokey );
  if( protokey.empty() )
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::non_empty_value_message() );
  }
  for( std::size_t idx = 0; idx != keylist.size(); ++idx)
  {
    if( keylist[ idx ].find( protokey ) == 0 )
    {
      target = idx;
      return;
    }
  }
  // If here then key not found.
  std::stringstream os;
  os << "(";
  for( auto key : keylist )
  {
    os << key << ",";
  }
  std::string list{ os.str() };
  list.back() = ')';
  throw input_error::parameter_value_error( title, section_name, param, (input_error::bad_key_message_format() % list).str() );

}

// Attempt to read an input value as a positive integer number
//
// \param name : input parameter name
// \param value : value in input file
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error

void input_base_reader::ordinal_input(const base_input_parameter & param, std::string title, std::string section_name, std::size_t & target)
{
  if( param.value().empty() )
  {
    throw input_error::missing_value_error( title, section_name, param );
  }
  const char *start = param.value().c_str();
  // Move past any whitespace
  while( not *start == '\0' and std::isspace( *start ) ) ++start;
  if( *start == '\0')
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::non_empty_value_message() );
  }
  // Check for negative sign
  if( *start == '-' )
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::number_negative_message() );
  }
  char *end;
  std::size_t result;
  errno = 0;
  // Expect base ten.
  result = std::strtoul(start, &end, 0);
  if( errno == ERANGE )
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::number_outside_range_message() );
  }
  if( *end != '\0' and not std::isspace( *end ) )
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::number_bad_message() );
  }
  target = result;

}

// Attempt to read an input value as a text value
//
// NOTES:
// *  dequotes the string but does not trim the dequoted string
//
// \param param : input parameter
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \param target : where to put the result, unchanged on error

void input_base_reader::text_input(const base_input_parameter & param, std::string title, std::string section_name, std::string & target)
{
  if( param.value().empty() )
  {
    throw input_error::missing_value_error( title, section_name, param );
  }
  // Remove quotes.
  std::string result = input_base_reader::dequote( param.value() );
  if( result.empty() )
  {
    throw input_error::parameter_value_error( title, section_name, param, input_error::non_empty_value_message() );
  }
  target = result;

}

// Attempt to read input value as a positive integer number. Throw consistently
// formatted error message on failure.
//
// see ordinal_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \return : the resulting ordinal

std::size_t input_base_reader::get_ordinal(std::string title, std::string section_name)
{
  std::size_t result{ 0 };
  input_parameter_reference param( *this );
  ordinal_input( param, title, section_name, result );
  return result;

}

// Attempt to read input value as a boolean. Throw consistently
// formatted error message on failure. 
//
// see bool_input (which is used internally)
//
// \param name : input parameter name
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \parma has_default : is an empty value an error
// \param equal_zero : what is result when value is empty (if has_default)

bool input_base_reader::get_bool(std::string name, std::string title, std::string section_name, bool has_default, bool default_value)
{
  bool result { false };
  input_parameter_reference param( *this );
  bool_input( param, title, section_name, result, has_default, default_value );
  return result;

}

// Attempt to read an input value as a floating point number. Throw consistently
// formatted error message on failure.
//
// see float_input (which is used internally)
//
// \param name : input parameter name
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \parma above_zero : whether value should be positive
// \param equal_zero : if above_zero is true, allows value to also equal zero 

double input_base_reader::get_float(std::string title, std::string section_name, bool above_zero, bool equal_zero)
{
  double result{ 0 };
  input_parameter_reference param( *this );
  float_input( param, title, section_name, result, above_zero, equal_zero );
  return result;

}

// Attempt to read input value as a text value, an empty string is an error. Throw consistently
// formatted error message on failure.
//
// see text_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \return : the resulting text

std::string input_base_reader::get_text(std::string title, std::string section_name)
{
  std::string result;
  input_parameter_reference param( *this );
  text_input( param, title, section_name, result );
  return result;

}

// Attempt to read input value as a key value matching a list of valid keys. The
// index of the matching key in the list is returned. Throw consistently
// formatted error message on failure.
//
// see key_input (which is used internally)
//
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \return : the resulting text

std::size_t input_base_reader::get_key(std::string title, std::string section_name, const std::vector< std::string >& keylist)
{
  std::size_t result{0};
  input_parameter_reference param( *this );
  key_input( param, title, section_name, result, keylist );
  return result;

}

// Attempt to read an input value as a series of floating point numbers. Throw 
// consistently formatted error message on failure.
//
// see floats_input (which is used internally)
//
// \param name : input parameter name
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \parma above_zero : whether value should be positive
// \param equal_zero : if above_zero is true, allows value to also equal zero 

void input_base_reader::get_floats(std::string title, std::string section_name, std::vector< double >& target)
{
  input_parameter_reference param( *this );
  floats_input( param, title, section_name, target );

}

// Attempt to read an input line as a series of floating point numbers. Throw consistently
// formatted error message on failure.
//
// see floats_input (which is used internally)
//
// \param name : input parameter name
// \param title : text giving class context for error, should start with capital.
// \param section_name : section context for error
// \parma above_zero : whether value should be positive
// \param equal_zero : if above_zero is true, allows value to also equal zero 

void input_base_reader::get_line_as_floats(std::string name, std::string title, std::string section_name, std::vector< double >& target)
{
  UTILITY_REQUIRE( not name.empty(), "Require name for parameter." );
  input_parameter_memo param( *this );
  param.set_name( name );
  param.set_value( param.line() );
  floats_input( param, title, section_name, target );

}

// Helper function to report error on duplicate parameters.
//
// \param is_first : Duplicate parameter error thrown if not true.
// \param section : input file section label.
void input_base_reader::duplicate_test(bool is_first, std::string section) const
{
  if( not is_first )
  {
    throw core::input_error::duplicate_parameter_error( section, this->name(), this );
  }
}

// Helper function to report error on duplicate parameters.
//
// \param is_first : Duplicate parameter error thrown if not true.
// \param section : input file section label.
void input_base_reader::missing_parameters_error(std::string title, std::string section, std::string params) const
{
  throw core::input_error::missing_parameters_error( title, section, params, this );
}

// Helper function to report an unknown parameter error.
//
// \param param : Unknown parameter.
// \param section : input file section label.
void input_base_reader::invalid_parameter_error(std::string section, std::string param) const
{
  throw core::input_error::invalid_parameter_error( section, param, this );
}

// Helper function to report an unknown parameter error.
//
// \param param : Unknown parameter.
// \param section : input file section label.
void input_base_reader::invalid_parameter_subtype_error(std::string section, std::string subtype, std::string param) const
{
  throw core::input_error::invalid_parameter_subtype_error( section, subtype, param, this );
}

// Helper function to report a parameter value error.
//
// \param title : Describe parameter.
// \param section : input file section label.
// \param msg : textual message on what error is.
void input_base_reader::parameter_value_error(std::string title, std::string section, std::string msg) const
{
  throw core::input_error::parameter_value_error( title, section, this->name(), this->value(), this, msg );
}


} // namespace core
