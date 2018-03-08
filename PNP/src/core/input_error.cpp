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

#include "core/input_error.hpp"
#include "core/base_input_parameter.hpp"
#include "core/input_base_reader.hpp"

// manual includes
#include "core/strngs.hpp"
// -
namespace core {

// Message to indicate a parameter was not valid for this section in input file.
//
// \param section : Name of the input file section.
// \param param : The duplicate parameter.
input_error input_error::duplicate_parameter_error(std::string section, const base_input_parameter & param)
{
std::stringstream os;
os << "Parameter \"" << param.name() << "\" appears more than once in section \""
  << section << "\"";
if( not param.filename().empty() )
{
  os  << " at input file \""
      << param.filename() << "\" line " << param.line_number()
      << ".\n   >>" << param.line() << "<<\n";
}
else
{
  os << ".\n";
}
os << "** Parameter \"" << param.name() << "\" can only appear once per section. **\n";
return input_error( param.name(), section, os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param name : Name of the unknown section.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::duplicate_section_error(std::string name, std::string filename, std::size_t linenum)
{
std::stringstream os;
os << "Once-only section \"" << name << "\" appears more than once";
if( not filename.empty() )
{
  os  << ", second appearance at input file \""
      << filename << "\" line " << linenum;
}
os << ".\n** Section \"" << name << "\" can only appear once. **\n";
return input_error( "", name, os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param section : Name of the input file section.
// \param param : The invalid/unknown parameter.
input_error input_error::invalid_parameter_error(std::string section, const base_input_parameter & param)
{
std::stringstream os;
os << "Unknown parameter \"" << param.name()
   << "\" in section \"" << section << "\"";
if( not param.filename().empty() )
{
  os << " at input file \"" << param.filename()
     << "\" line " << param.line_number()
     << ".\n   >>" << param.line() << "<<\n";
}
else
{
  os << ".\n";
}
os << "** Parameter \"" << param.name() << "\" is not expected in this section. **\n";
return input_error( "", section, os.str() );

}

// Message to indicate a parameter was not valid for this section and subtype in input file.
//
// \param section : Name of the input file section.
// \param subtype : Name of the section subtype.
// \param name : Name of the unknown parameter.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::invalid_parameter_subtype_error(std::string section, std::string subtype, const base_input_parameter & param)
{
std::stringstream os;
os << "Unknown parameter \"" << param.name() << "\" in section \""
  << section << "\" subtype \"" << subtype << "\"";
if( not param.filename().empty() )
{
  os << " detected at input file \"" << param.filename()
       << "\" line " << param.line_number() << ".\n";
}
else
{
  os << ".\n";
}
os << "** Parameter \"" << param.name() << "\" is not expected in this section and type. **\n";
return input_error( "", section, os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param name : Name of the unknown section.
// \param filename/linenum : the input file location.
// \param list : list of valid section labels.
input_error input_error::invalid_section_error(std::string name, std::string filename, std::size_t linenum, std::string list)
{
std::stringstream os;
os << "Unknown section label \"" << name << "\"";
if( not filename.empty() )
{
  os << " at input file \"" << filename << "\" line " << linenum << ".\n";
}
else
{
  os << ".\n";
}
os << "** Section label must be one of : " << list << ". **\n";
return input_error( "", "", os.str() );

}

// Message to indicate a required parameter was missing in input file.
//
// \param title : A title phrase for the section.
// \param section : Name of the input file section.
// \param params : List of missing parameters.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::missing_parameters_error(std::string title, std::string section, std::string params, std::string filename, std::size_t linenum)
{
std::stringstream os;
os << title << " section \"" << section
  << "\" is missing some required parameters";
if( not filename.empty() )
{
  os << " at input file \"" << filename
     << "\" in section ending at line " << linenum << ".\n";
}
else
{
  os << ".\n";
}
os << "** Add the following parameters to the section: " << params << " **\n";
return input_error( "", section, os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param name : Name of the unknown section.
// \param reader : the input reader
// \param list : list of valid section labels.
input_error input_error::missing_section_error(std::string list)
{
std::stringstream os;
os << "Missing required sections detected at end of input file.\n" 
   << "** The following sections must be present in the input : " << list << ". **\n";
return input_error( "", "", os.str() );

}

// input_error to indicate a parameter was missing a required value in input file.
//
// \param title : A title phrase for the parameter (Should begin with a capital).
// \param section : Name of the input file section.
// \param name : Name of the parameter.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
//
// 
input_error input_error::missing_value_error(std::string title, std::string section, const base_input_parameter & param)
{
std::stringstream os;
os << title << " parameter \"" << param.name() << "\"";
if( not section.empty() )
{
  os << " in section \"" << section << "\"";
}
if( not param.filename().empty() )
{ 
  os << " at input file \"" << param.filename() << "\" line "
       << param.line_number() << ".\n   >>" << param.line() << "<<\n";
}
else
{
  os << ".\n";
}
os << "** Expected a value. **\n";
return input_error( param.name(), section, os.str() );

}

// Build a standard input error message about a parameter value error.
//
// \param title : A title phrase for the parameter (Should begin with a capital).
// \param section : Name of the input file section.
// \param param : The parameter information.
// \param msg : A short sentence describing the problem.

input_error input_error::parameter_value_error(std::string title, std::string section, const base_input_parameter & param, std::string msg)
{
std::stringstream os;
os << title << " parameter \"" << param.name() << "\" with value (" << param.value() << ")";
if( not section.empty() )
{
  os << " in section \"" << section << "\"";
}
if( not param.filename().empty() )
{
   os << " at input file \"" << param.filename() << "\" line "
       << param.line_number() << ".\n   >>" << param.line() << "<<\n";
}
else
{
  os << ".\n";
}
os << "** " << msg << " **\n";
return input_error( param.name(), section, os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param section : Name of the input file section.
// \param name : Name of the unknown parameter.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::duplicate_parameter_error(std::string section, std::string name, const input_base_reader * reader)
{
std::stringstream os;
os << "Parameter \"" << name << "\" appears more than once in section \""
  << section << "\"";
if( reader and not reader->current_filename().empty() )
{
  os  << " at input file \""
      << reader->current_filename() << "\" line " << reader->current_line_number()
      << ".\n   >>" << reader->line() << "<<\n";
}
else
{
  os << ".\n";
}
os << "** Parameter \"" << name << "\" can only appear once per section. **\n";
return input_error( "", section, os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param name : Name of the unknown section.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::duplicate_section_error(std::string name, const input_base_reader * reader)
{
std::stringstream os;
os << "Once-only section \"" << name << "\" appears more than once";
if( reader and not reader->current_filename().empty() )
{
  os  << ", second appearance at input file \""
      << reader->current_filename() << "\" line " << reader->current_line_number();
}
os << ".\n** Section \"" << name << "\" can only appear once. **\n";
return input_error( "", "", os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param section : Name of the input file section.
// \param name : Name of the unknown parameter.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::invalid_parameter_error(std::string section, std::string name, const input_base_reader * reader)
{
std::stringstream os;
os << "Unknown parameter \"" << name 
    << "\" in section \"" << section << "\"";
if( reader and not reader->current_filename().empty() )
{
  if( reader->name() == name )
  { 
    os << " at input file \"" << reader->current_filename()
       << "\" line " << reader->current_line_number()
       << ".\n   >>" << reader->line() << "<<\n";
  }
  else
  {
    os << " detected at input file \"" << reader->current_filename()
       << "\" line " << reader->current_line_number() << ".\n"; 
  }
}
else
{
  os << ".\n";
}
os << "** Parameter \"" << name << "\" is not expected in this section. **\n";
return input_error( "", section, os.str() );

}

// Message to indicate a parameter was not valid for this section and subtype in input file.
//
// \param section : Name of the input file section.
// \param subtype : Name of the section subtype.
// \param name : Name of the unknown parameter.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::invalid_parameter_subtype_error(std::string section, std::string subtype, std::string name, const input_base_reader * reader)
{
std::stringstream os;
os << "Unknown parameter \"" << name << "\" in section \""
  << section << "\" subtype \"" << subtype << "\"";
if( reader and not reader->current_filename().empty() )
{
  if( reader->name() == name )
  {
    os << " at input file \"" << reader->current_filename()
       << "\" line " << reader->current_line_number()
       << ".\n   >>" << reader->line() << "<<\n";
  }
  else
  {
    os << " detected at input file \"" << reader->current_filename()
       << "\" line " << reader->current_line_number() << ".\n";
  }
}
else
{
  os << ".\n";
}
os << "** Parameter \"" << name << "\" is not expected in this section and type. **\n";
return input_error( "", section, os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param name : Name of the unknown section.
// \param reader : the input reader
// \param list : list of valid section labels.
input_error input_error::invalid_section_error(std::string name, const input_base_reader * reader, std::string list)
{
std::stringstream os;
os << "Unknown section label \"" << name << "\"";
if( reader and not reader->current_filename().empty() )
{
  if( reader->name() == name )
  { 
    os << " at input file \"" << reader->current_filename()
       << "\" line " << reader->current_line_number()
       << ".\n   >>" << reader->line() << "<<\n";
  }
  else
  {
    os << " detected at input file \"" << reader->current_filename()
       << "\" line " << reader->current_line_number() << ".\n"; 
  }
}
else
{
  os << ".\n";
}
os << "** Section label must be one of : " << list << ". **\n";
return input_error( "", "", os.str() );

}

// Message to indicate a required parameter was missing in input file.
//
// \param title : A title phrase for the section.
// \param section : Name of the input file section.
// \param params : List of missing parameters.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::missing_parameters_error(std::string title, std::string section, std::string params, const input_base_reader * reader)
{
std::stringstream os;
os << title << " section \"" << section
  << "\" is missing some required parameters";
if( reader and not reader->current_filename().empty() )
{
  os << " at input file \"" << reader->current_filename()
     << "\" in section ending at line " << reader->current_line_number() << ".\n";
}
else
{
  os << ".\n";
}
os << "** Add the following parameters to the section: " << params << " **\n";
return input_error( "", section, os.str() );

}

// input_error to indicate a parameter was missing a required value in input file.
//
// \param title : A title phrase for the parameter (Should begin with a capital).
// \param section : Name of the input file section.
// \param name : Name of the parameter.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
//
// 
input_error input_error::missing_value_error(std::string title, std::string section, std::string name, const input_base_reader * reader)
{
std::stringstream os;
os << title << " parameter \"" << name << "\"";
if( not section.empty() )
{
  os << " in section \"" << section << "\"";
}
if( reader and not reader->current_filename().empty() )
{ 
  if( reader->name().find( name ) == 0 )
  {
    os << " at input file \"" << reader->current_filename() << "\" line "
       << reader->current_line_number() << ".\n   >>" << reader->line() << "<<\n";
  }
  else
  {
    os << " ending at input file \"" << reader->current_filename() << "\" line "
       << reader->current_line_number() << ".\n";
  }
}
else
{
  os << ".\n";
}
os << "** Expected a value. **\n";
return input_error( name, section, os.str() );

}

// Build a standard input error message about a parameter value error.
//
// \param title : A title phrase for the parameter (Should begin with a capital).
// \param section : Name of the input file section.
// \param name : Name of the parameter.
// \param value : The incorrect parameter value.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
// \param msg : A short sentence describing the problem.

input_error input_error::parameter_value_error(std::string title, std::string section, std::string name, std::string value, const input_base_reader * reader, std::string msg)
{
std::stringstream os;
os << title << " parameter \"" << name << "\" with value (" << value << ")";
if( not section.empty() )
{
  os << " in section \"" << section << "\"";
}
if( reader and not reader->current_filename().empty() )
{
  if( reader->name() == name )
  {
    os << " at input file \"" << reader->current_filename() << "\" line "
       << reader->current_line_number() << ".\n   >>" << reader->line() << "<<\n";
  }
  else
  {
    os << " ending at input file \"" << reader->current_filename() << "\" line "
       << reader->current_line_number() << ".\n";
  }
}
else
{
  os << ".\n";
}
os << "** " << msg << " **\n";
return input_error( name, section, os.str() );

}

// Build a standard input error message about a parameter value error.
//
// \param name : Name of the file to include.
// \param reader : Reader object.
// \param msg : A short sentence describing the problem.

input_error input_error::input_file_error(std::string name, const input_base_reader * reader, std::string msg)
{
std::stringstream os;
os << "Problem including file \"" << name << "\" using keyword \""
   << core::strngs::fsincl() << "\"";
if( reader )
{
  os  << " at input file \""
      << reader->current_filename() << "\" line " << reader->current_line_number()
      << ".\n   >>" << reader->line() << "<<\n";
}
else
{
  os << ".\n";
}
os << "** " << msg << " **\n";
return input_error( core::strngs::fsincl(), "", os.str() );

}

// Message to indicate a parameter was not valid for this section in input file.
//
// \param section : Name of the input file section.
// \param name : Name of the unknown parameter.
// \param line : The complete input file line.
// \param filename : Name of the input file.
// \param linenum : Line number in the input file.
input_error input_error::input_logic_error(std::string title, std::string section, std::string msg)
{
std::stringstream os;
os << title << " in section \"" << section << "\".\n** " << msg << " **\n";
return input_error( "", section, os.str() );

}

// Message to indicate a value to a "key" based parameter was not recognised in the input file.
//
// Format requires the list of valid keys.
boost::format input_error::bad_key_message_format()
{
  static boost::format fmt( "A value from the list %1% was expected." );
  return fmt;
}

// Message to indicate a value for a parameter can not be an empty string in input file.
//
// Format takes (1) parameter name and (2) section name as args.
std::string input_error::non_empty_value_message()
{
  return "Expected a non-empty value.";
}


} // namespace core
