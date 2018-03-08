#ifndef IONCH_CORE_BASE_INPUT_PARAMETER_HPP
#define IONCH_CORE_BASE_INPUT_PARAMETER_HPP

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

#include <string>
#include <cstddef>
#include <boost/serialization/vector.hpp>

namespace core { class input_base_reader; } 

namespace core {

// Capture information about a parameter.
class base_input_parameter
 {
   public:
    // The parameter name
    virtual std::string name() const = 0;

    // The parameter value
    virtual std::string value() const = 0;

    // The line containing the name/value pair
    virtual std::string line() const = 0;

    // The name of the input file containing the parameter
    virtual std::string filename() const = 0;

    // The line number in the input file that contains this parameter.
    virtual std::size_t line_number() const = 0;

    virtual ~base_input_parameter() {}

    // Attempt to read input value as a positive integer number. Throw consistently
    // formatted error message on failure.
    //
    // see input_base_reader::ordinal_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \return : the resulting ordinal
    
    std::size_t get_ordinal(std::string title, std::string section_name) const;

    // Attempt to read input value as a boolean. Throw consistently
    // formatted error message on failure. 
    //
    // see input_base_reader::bool_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \parma has_default : is an empty value an error
    // \param default_value : what is result when value is empty (if has_default)
    
    bool get_bool(std::string title, std::string section_name, bool has_default, bool default_value) const;

    // Attempt to read an input value as a floating point number. Throw consistently
    // formatted error message on failure.
    //
    // see input_base_reader::float_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \parma above_zero : whether value should be positive
    // \param equal_zero : if above_zero is true, allows value to also equal zero 
    
    double get_float(std::string title, std::string section_name, bool above_zero, bool equal_zero) const;

    // Attempt to read input value as a text value, an empty string is an error. Throw consistently
    // formatted error message on failure.
    //
    // see input_base_reader::text_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \return : the resulting text
    
    std::string get_text(std::string title, std::string section_name) const;

    // Attempt to read input value as a key value matching a list of valid keys. The
    // index of the matching key in the list is returned. Throw consistently
    // formatted error message on failure.
    //
    // see input_base_reader::key_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \return : the index in the give key list
    
    std::size_t get_key(std::string title, std::string section_name, const std::vector< std::string >& keylist) const;

    // Attempt to read an input value as a series of floating point numbers. Throw 
    // consistently formatted error message on failure.
    //
    // see input_base_reader::floats_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : vector containing result
    
    void get_floats(std::string title, std::string section_name, std::vector< double >& target) const;

    // Attempt to read an input line as a series of floating point numbers. Throw consistently
    // formatted error message on failure.
    //
    // see input_base_reader::floats_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : vector containing result
    
    void get_line_as_floats(std::string name, std::string title, std::string section_name, std::vector< double >& target) const;

    bool equal(const base_input_parameter & other) const;

    friend bool operator==(const base_input_parameter & lhs, const base_input_parameter & rhs)
    {
      return lhs.equal( rhs );
    }


};

} // namespace core
#endif
