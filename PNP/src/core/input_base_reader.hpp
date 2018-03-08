#ifndef IONCH_CORE_INPUT_BASE_READER_HPP
#define IONCH_CORE_INPUT_BASE_READER_HPP

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
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


#include <cstddef>
#include <boost/serialization/vector.hpp>

namespace core { class input_error; } 
namespace core { class base_input_parameter; } 

namespace core {

//Base class for input readers.
//
//Key responsibility: Interpretation of a single input line.

class input_base_reader
 {
   private:
    //The current name from the line
    std::string line_;

    //The current name from the line
    std::string name_;

    //The current name from the line
    std::string value_;


   public:
    input_base_reader()
    : line_()
    , name_()
    , value_()
    {}

    virtual ~input_base_reader()
    {}


   private:
    input_base_reader(const input_base_reader & source) = delete;
    input_base_reader(input_base_reader && source) = delete;
    input_base_reader & operator=(const input_base_reader & source) = delete;

  friend class boost::serialization::access;
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
       ar & line_;
       ar & name_;
       ar & value_;
    }
// --------------------------------------------------
// Observer methods
// --------------------------------------------

   public:
    // Get the current line (including comments)
    const std::string & line() const
    {
      return this->line_;
    }

    // Get the name part of the current line
    const std::string & name() const
    {
      return this->name_;
    }

    // Get the current line (excluding comments)
    std::string trimmed_line() const;

    // --------------------------------------------
    // Get the value part of the current line.
    const std::string & value() const
    {
      return this->value_;
    }

// ----------------------------------------
// mutating methods
// ----------------------------------------
    //Add a new buffer to the input document.
    virtual void add_buffer(const boost::filesystem::path & filename, std::string buffer) = 0;

    // Add a new include file to the input document.
    //
    // This method reads the given file and all included file into
    // the reader object. 
    virtual void add_include(const boost::filesystem::path & filename) = 0;

    //Clear value of 'line', 'name' and 'value'
    void clear();

    // --------------------------------------------
    // CURRENT FILE NAME
    //
    // Get the name of the file that is currently being processed.
    // If there is not current file then an empty string or None is returned
    virtual std::string current_filename() const = 0;

    // --------------------------------------------
    // CURRENT LINE NUMBER
    //
    // Get the line number of the file that is currently being
    // processed. If there is no current file then 0(C++) or
    // -1(python) is returned.
    virtual std::size_t current_line_number() const = 0;

    // --------------------------------------------
    // READ NEXT NAME,VALUE PAIR
    // 
    // Return false when there is no more input
    //
    // Reads line from the current input file ignoring blank lines and
    // deleting comments beginning with "#". Comments may appear
    // anywhere on line.
    //
    // Splits the line into a name, value pair.  Value may be an empty
    // string.
    //
    // (In 'include' statements C++ should handle relative paths, 
    // python currently does not)
    
    bool next()
    {
      this->clear();
      return this->do_next();
    }

   private:
    // --------------------------------------------
    // READ NEXT NAME,VALUE PAIR
    // 
    // Return false when there is no more input
    //
    // Reads line from the current input file ignoring blank lines and
    // deleting comments beginning with "#". Comments may appear
    // anywhere on line.
    //
    // Splits the line into a name, value pair.  Value may be an empty
    // string.
    //
    // (In 'include' statements C++ should handle relative paths, 
    // python currently does not)
    
    virtual bool do_next() = 0;


   public:
    //Set the current line and split into name and value.
    //
    //\post line() = line
    bool set_line(std::string line);

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
    
    static std::string decomment(std::string a_val, char comment_start);

    static std::string decomment(std::string a_val)
    {
      return input_base_reader::decomment(a_val, '#');
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
    
    static std::string dequote(std::string a_val);

    // --------------------------------------------------
    // Convert the current line into an array of floats. Returns true
    // if the whole line was converted.
    //
    // Any data in a_arry will be deleted on entry to the function.
    //
    // The actual number of elements converted will be the final size of
    // a_arry.
    static bool read_as_floats(std::string input_buffer, std::vector< double > & a_arry);

    //Convert textual representation of true/false into bool.
    //
    //Look for (english) textual representation of boolean.
    //
    //(1) find 'true' or 'false' ignoring case
    //
    //(2) if (1) fails attempt stringstream(a_val) >> a_res
    
    static bool read_as_bool(std::string astr, bool & target);

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
    
    static void bool_input(std::string name, std::string value, std::string title, std::string section_name, bool & target, bool has_default, bool default_value, const input_base_reader * reader);

    // Attempt to read an input value as a floating point number
    //
    // \param name : input parameter name
    // \param value : value in input file
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    // \parma above_zero : whether value should be positive
    // \param equal_zero : if above_zero is true, allows value to also equal zero 
    
    static void float_input(std::string name, std::string value, std::string title, std::string section_name, double & target, bool above_zero, bool equal_zero, input_base_reader const* reader);

    // Attempt to read an input value as a floating point number
    //
    // \param name : input parameter name
    // \param value : value in input file
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    // \parma above_zero : whether value should be positive
    // \param equal_zero : if above_zero is true, allows value to also equal zero 
    
    static void floats_input(std::string name, std::string value, std::string title, std::string section_name, std::vector< double > & target, input_base_reader const* reader);

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
    
    static void key_input(std::string name, std::string value, std::string title, std::string section_name, std::size_t & target, const std::vector< std::string >& keylist, const input_base_reader * reader);

    // Attempt to read an input value as a positive integer number
    //
    // \param name : input parameter name
    // \param value : value in input file
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    
    static void ordinal_input(std::string name, std::string value, std::string title, std::string section_name, std::size_t & target, const input_base_reader * reader);

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
    
    static void text_input(std::string name, std::string value, std::string title, std::string section_name, std::string & target, const input_base_reader * reader);

    // Attempt to read an input value as a floating point number
    //
    // \param param : input parameter
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    // \parma has_default : is an empty value an error
    // \param default_value: what is result when value is empty (if has_default)
    
    static void bool_input(const base_input_parameter & param, std::string title, std::string section_name, bool & target, bool has_default, bool default_value);

    // Attempt to read an input value as a floating point number
    //
    // \param param : input parameter
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    // \parma above_zero : whether value should be positive
    // \param equal_zero : if above_zero is true, allows value to also equal zero 
    
    static void float_input(const base_input_parameter & param, std::string title, std::string section_name, double & target, bool above_zero, bool equal_zero);

    // Attempt to read an input value as a sequence of floating point numbers
    //
    // \param param : input parameter
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    // \param name : whether to convert value (empty string) or whole line (non-empty
    //               string). When converting whole line, use name as parameter name.
    
    static void floats_input(const base_input_parameter & param, std::string title, std::string section_name, std::vector< double > & target);

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
    
    static void key_input(const base_input_parameter & param, std::string title, std::string section_name, std::size_t & target, const std::vector< std::string >& keylist);

    // Attempt to read an input value as a positive integer number
    //
    // \param name : input parameter name
    // \param value : value in input file
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    
    static void ordinal_input(const base_input_parameter & param, std::string title, std::string section_name, std::size_t & target);

    // Attempt to read an input value as a text value
    //
    // NOTES:
    // *  dequotes the string but does not trim the dequoted string
    //
    // \param param : input parameter
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    
    static void text_input(const base_input_parameter & param, std::string title, std::string section_name, std::string & target);

    // Attempt to read input value as a positive integer number. Throw consistently
    // formatted error message on failure.
    //
    // see ordinal_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \return : the resulting ordinal
    
    std::size_t get_ordinal(std::string title, std::string section_name);

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
    
    bool get_bool(std::string name, std::string title, std::string section_name, bool has_default, bool default_value);

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
    
    double get_float(std::string title, std::string section_name, bool above_zero, bool equal_zero);

    // Attempt to read input value as a text value, an empty string is an error. Throw consistently
    // formatted error message on failure.
    //
    // see text_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \return : the resulting text
    
    std::string get_text(std::string title, std::string section_name);

    // Attempt to read input value as a key value matching a list of valid keys. The
    // index of the matching key in the list is returned. Throw consistently
    // formatted error message on failure.
    //
    // see key_input (which is used internally)
    //
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \return : the resulting text
    
    std::size_t get_key(std::string title, std::string section_name, const std::vector< std::string >& keylist);

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
    
    void get_floats(std::string title, std::string section_name, std::vector< double >& target);

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
    
    void get_line_as_floats(std::string name, std::string title, std::string section_name, std::vector< double >& target);

    // Helper function to report error on duplicate parameters.
    //
    // \param is_first : Duplicate parameter error thrown if not true.
    // \param section : input file section label.
    void duplicate_test(bool is_first, std::string section) const;

    // Helper function to report error on duplicate parameters.
    //
    // \param is_first : Duplicate parameter error thrown if not true.
    // \param section : input file section label.
    void missing_parameters_error(std::string title, std::string section, std::string params) const;

    // Helper function to report an unknown parameter error.
    //
    // \param param : Unknown parameter.
    // \param section : input file section label.
    void invalid_parameter_error(std::string section, std::string param) const;

    // Helper function to report an unknown parameter error.
    //
    // \param param : Unknown parameter.
    // \param section : input file section label.
    void invalid_parameter_subtype_error(std::string section, std::string subtype, std::string param) const;

    // Helper function to report a parameter value error.
    //
    // \param title : Describe parameter.
    // \param section : input file section label.
    // \param msg : textual message on what error is.
    void parameter_value_error(std::string title, std::string section, std::string msg) const;


};

} // namespace core
#endif
