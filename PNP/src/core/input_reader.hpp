#ifndef IONCH_CORE_INPUT_READER_HPP
#define IONCH_CORE_INPUT_READER_HPP

//Authors: Justin Finnerty
//
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

#include "core/input_base_reader.hpp"
#include "core/input_node.hpp"
#include <boost/serialization/vector.hpp>
#include <string>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


#include <cstddef>

//--
namespace core { class strngs; } 

namespace core {

//  ----------------------------------------------------------------------
//  INPUT FILE READER INTERFACE
//
//  This class provides an iterator like interface over the input
//  files. It internally handles include files and comments.  It uses
//  lazy evaluation, openning include files in line. It detects
//  circular references by limiting the include depth.
//
//  The input_reader only understands two specific strings of the input
//  file vocabulary: 'include' and '#'.  All other strings must be
//  interpreted by managing object(s).
//
//
//  ----------------------------------------------------------------------
//  INPUT PROCESSING HELPER METHODS
//
//  dequote - Remove matching quotes around a string
//  decomment - Remove comment markers and comments
//
//The requirement of needing an input file format that was readable
//and writable by both humans and machines led to the Serializer and
//SerializableMetaClass classes.  The definition of the input file
//is fairly flexible and consists of:
//
// * three words with special meaning if they appear as the first
//   word on a line.  The first two have special meaning only outside
//   an input section.
//
//  * 'fileversion' : denotes name,value pair of the version of the
//    current input file.  The version only applies to input sections
//    after the fileversion pair is encountered
//
//  * 'include' : a name,value pair with the name of an input file
//    to be read in-line at the current position.
//
//  * 'end' : indicates the end of an input section.
//
// * whole and in-line comments beginning with '#'
//
// * input sections started by an arbitrary label and ending with the
//   'end' special word.  Input sections may appear in any order.
//   Handling of the content of a section is delegated to a
//   SerializeMetaClass object with the same section label.  How
//   multiple sections with the same label are handled is the
//   responsibility of the SerializeMetaClass object.
//
//To allow sections to appear in any order requires a two stage read
//process.  In the first stage the sections are passed to the
//SerializeMetaClass objects in the order they are encountered in the
//input file.  The second stage occurs once the input file has been
//completely read, the SerializeMetaClass objects are then notified
//in the order they appear in the Serializer.section_handlers list.

class input_reader : public input_base_reader
 {
   private:
    // The set of buffered input streams/files
    std::vector< input_node > file_stack_;

// --------------------------------------------------
// Lifetime methods
// --------------------------------------------------

   public:
    // Generate a reader using the filename of the top-level file.
    input_reader(std::string filename);

    // Generate a reader using a (possibly dummy) filename and a
    // preexisting input stream.
    input_reader(std::string filename, std::istream & ais);

    ~input_reader();

    input_reader() = default;


   private:
    // invalid operation
    input_reader(const input_reader & source) = delete;

    // invalid operation
    input_reader(input_reader && source);

    // invalid operation
    const input_reader & operator =(const input_reader & );

// --------------------------------------------------
// Observer methods
// --------------------------------------------

   public:
    // Get the name of the file that is currently being processed.
    // If there is not current file then an empty string or None is returned
    std::string current_filename() const override
    {
      return (this->file_stack_.empty() ? std::string() : this->file_stack_.back().path());
    }

    // Get the line number of the file that is currently being
    // processed. If there is no current file then 0(C++) or
    // -1(python) is returned.
    std::size_t current_line_number() const override
    {
      return (this->file_stack_.empty() ? 0 : this->file_stack_.back().line_number());
    } 

// ----------------------------------------
// mutating methods
// ----------------------------------------
    // Add a preread buffer with a (possibly dummy) filename.
    void add_buffer(const boost::filesystem::path & filename, std::string buffer) override;

    // Add an included file.
    void add_include(const boost::filesystem::path & filename) override;


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
    
    bool do_next() override;


};
// --------------------------------------------------
// Lifetime methods
// --------------------------------------------------

} // namespace core
#endif
