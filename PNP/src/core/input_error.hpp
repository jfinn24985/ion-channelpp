#ifndef IONCH_CORE_INPUT_ERROR_HPP
#define IONCH_CORE_INPUT_ERROR_HPP

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

#include <stdexcept>

#include <string>
#include <cstddef>
#include <boost/format.hpp>


// manual includes
// -
#include <sstream>
// -
namespace core { class base_input_parameter; } 
namespace core { class input_base_reader; } 

namespace core {

// Exception to indicate a malformed input file.
//
// * Includes standard error message format templates.
//
// The standard input error message should follow the 
// form:
//
//   XYZ parameter "name" in section "secname" of
//   input file "filename" line "123"
//     >>line of text<<
//   ** requires a value **
//   ** value (ABC) should be a positive integer **
//
//
//   

class input_error : public std::runtime_error
 {
   private:
    // The parameter name at the error (may be empty string).
    std::string parameter_;

    // The input section name where the error occured.
    std::string section_;

    input_error(std::string msg)
    : std::runtime_error( msg )
    , parameter_()
    , section_()
    {}

    input_error(std::string param, std::string section, std::string msg)
    : std::runtime_error( msg )
    , parameter_( param )
    , section_( section )
    {}


   public:
    virtual ~input_error() = default;

    input_error(input_error && source)
    : std::runtime_error( std::move( source ) )
    , parameter_( std::move( source.parameter_ ) )
    , section_( std::move( source.section_ ) )
    {}

    input_error(const input_error & source)
    : std::runtime_error( source )
    , parameter_( source.parameter_ )
    , section_( source.section_ )
    {}
    

    input_error & operator=(input_error source)
    {
      this->swap( source );
      return *this;
    }

    void swap(input_error & source)
    {
      std::swap( static_cast< std::runtime_error& >(*this), static_cast< std::runtime_error& >(source) );
      std::swap( this->parameter_, source.parameter_ );
      std::swap( this->section_, source.section_ );
    }

    // The name (if any) of the parameter where the error occured.
    std::string parameter() const
    {
      return this->parameter_;
    }

    // The name of the input file section where the error occured.
    std::string section() const
    {
      return this->section_;
    }

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param section : Name of the input file section.
    // \param param : The duplicate parameter.
    static input_error duplicate_parameter_error(std::string section, const base_input_parameter & param);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param name : Name of the unknown section.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error duplicate_section_error(std::string name, std::string filename, std::size_t linenum);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param section : Name of the input file section.
    // \param param : The invalid/unknown parameter.
    static input_error invalid_parameter_error(std::string section, const base_input_parameter & param);

    // Message to indicate a parameter was not valid for this section and subtype in input file.
    //
    // \param section : Name of the input file section.
    // \param subtype : Name of the section subtype.
    // \param name : Name of the unknown parameter.
    // \param line : The complete input file line.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error invalid_parameter_subtype_error(std::string section, std::string subtype, const base_input_parameter & param);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param name : Name of the unknown section.
    // \param filename/linenum : the input file location.
    // \param list : list of valid section labels.
    static input_error invalid_section_error(std::string name, std::string filename, std::size_t linenum, std::string list);

    // Message to indicate a required parameter was missing in input file.
    //
    // \param title : A title phrase for the section.
    // \param section : Name of the input file section.
    // \param params : List of missing parameters.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error missing_parameters_error(std::string title, std::string section, std::string params, std::string filename, std::size_t linenum);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param name : Name of the unknown section.
    // \param reader : the input reader
    // \param list : list of valid section labels.
    static input_error missing_section_error(std::string list);

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
    static input_error missing_value_error(std::string title, std::string section, const base_input_parameter & param);

    // Build a standard input error message about a parameter value error.
    //
    // \param title : A title phrase for the parameter (Should begin with a capital).
    // \param section : Name of the input file section.
    // \param param : The parameter information.
    // \param msg : A short sentence describing the problem.
    
    static input_error parameter_value_error(std::string title, std::string section, const base_input_parameter & param, std::string msg);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param section : Name of the input file section.
    // \param name : Name of the unknown parameter.
    // \param line : The complete input file line.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error duplicate_parameter_error(std::string section, std::string name, const input_base_reader * reader);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param name : Name of the unknown section.
    // \param line : The complete input file line.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error duplicate_section_error(std::string name, const input_base_reader * reader);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param section : Name of the input file section.
    // \param name : Name of the unknown parameter.
    // \param line : The complete input file line.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error invalid_parameter_error(std::string section, std::string name, const input_base_reader * reader);

    // Message to indicate a parameter was not valid for this section and subtype in input file.
    //
    // \param section : Name of the input file section.
    // \param subtype : Name of the section subtype.
    // \param name : Name of the unknown parameter.
    // \param line : The complete input file line.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error invalid_parameter_subtype_error(std::string section, std::string subtype, std::string name, const input_base_reader * reader);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param name : Name of the unknown section.
    // \param reader : the input reader
    // \param list : list of valid section labels.
    static input_error invalid_section_error(std::string name, const input_base_reader * reader, std::string list);

    // Message to indicate a required parameter was missing in input file.
    //
    // \param title : A title phrase for the section.
    // \param section : Name of the input file section.
    // \param params : List of missing parameters.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error missing_parameters_error(std::string title, std::string section, std::string params, const input_base_reader * reader);

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
    static input_error missing_value_error(std::string title, std::string section, std::string name, const input_base_reader * reader);

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
    
    static input_error parameter_value_error(std::string title, std::string section, std::string name, std::string value, const input_base_reader * reader, std::string msg);

    // Build a standard input error message about a parameter value error.
    //
    // \param name : Name of the file to include.
    // \param reader : Reader object.
    // \param msg : A short sentence describing the problem.
    
    static input_error input_file_error(std::string name, const input_base_reader * reader, std::string msg);

    // Message to indicate a parameter was not valid for this section in input file.
    //
    // \param section : Name of the input file section.
    // \param name : Name of the unknown parameter.
    // \param line : The complete input file line.
    // \param filename : Name of the input file.
    // \param linenum : Line number in the input file.
    static input_error input_logic_error(std::string title, std::string section, std::string msg);

    // Message to indicate a value to a parameter was not recognised boolean.
    static std::string bad_boolean_message()
    {
      return "Expected a boolean value (ie. true|false).";
    }

    // Message to indicate a value to a "key" based parameter was not recognised in the input file.
    //
    // Format requires the list of valid keys.
    static boost::format bad_key_message_format();

    // Message to indicate a value for a parameter can not be an empty string in input file.
    //
    // Format takes (1) parameter name and (2) section name as args.
    static std::string non_empty_value_message();

    // Message that number was outside allowed range.
    static std::string number_outside_range_message()
    {
      return "Numeric value given is outside numeric range.";
    }

    // Message that number was a NAN (Not-a-number)
    static std::string number_nan_message()
    {
      return "Not-a-number (NaN) values are not allowed in input.";
    }

    // Message that number was an infinity
    static std::string number_inf_message()
    {
      return "Infinity (inf) values are not allowed in input.";
    }

    // Message that number was negative when only positive numbers are allowed
    static std::string number_negative_message()
    {
      return "Input value must be greater than or equal to zero.";
    }

    // Message that number was outside allowed range.
    static std::string number_zero_message()
    {
      return "Input value must be greater than zero.";
    }

    // Message that number was not recognised as a positive integer
    static std::string number_bad_ordinal_message()
    {
      return "A positive integer value was expected.";
    }

    // Message that number was zero when only positive non-zero numbers are allowed
    static std::string number_bad_message()
    {
      return "A numeric value was expected.";
    }


};

} // namespace core
#endif
