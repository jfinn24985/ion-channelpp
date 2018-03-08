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

#include "core/input_preprocess.hpp"

// Manual includes
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "utility/utility.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
// --

namespace core {

// Generate a reader.
input_preprocess::input_preprocess()  
: input_base_reader()
, includes_()
, file_stack_()
{}

input_preprocess::~input_preprocess() = default;
// --------------------------------------------------
// Observer methods
// --------------------------------------------
//Find (and return) an include file with the given name. Return
//'end' if not found.
input_preprocess::iterator input_preprocess::find_include(const boost::filesystem::path & filename)
{
  std::string fn = boost::filesystem::absolute( filename ).string();
  return this->includes_.find( fn );

}

// ----------------------------------------
// mutating methods
// ----------------------------------------
// Add a new buffer to the input document.
void input_preprocess::add_buffer(const boost::filesystem::path & filename, std::string buffer)
{
  // Make filename complete
  std::string fn = boost::filesystem::absolute( filename ).string();
  // Require that it is not already in the map
  UTILITY_REQUIRE( this->end() == this->includes_.find( fn ), "Input file included multiple times." );
  
  // Process the buffer
  this->includes_[ fn ].set_buffer( fn, buffer );
  this->process( this->includes_[ fn ] );
  
  if ( this->file_stack_.empty() )
  {
     // (re)initialize the system
     this->root_node_ = fn;
     UTILITY_CHECK( this->end() != this->includes_.find( fn ), "Input file not found after including it." );
     this->file_stack_.push_back( this->includes_.find( fn ) );
  }

}

// Add a new include file to the input document.
//
// This method reads the given file and all included file into
// the reader object. 
void input_preprocess::add_include(const boost::filesystem::path & filename)
{
  std::string fn = boost::filesystem::absolute( filename ).string();
  // Require that it is not already in the map
  UTILITY_REQUIRE( this->end() == this->includes_.find( fn ), "Input file included multiple times." );
  this->includes_[ fn ].set_buffer( fn );
  this->process( this->includes_[ fn ] );
  
  if ( this->file_stack_.empty() )
  {
     // initialize the system
     this->root_node_ = fn;
     UTILITY_CHECK( this->end() != this->includes_.find( fn ), "Input file not found after including it." );
     this->file_stack_.push_back( this->includes_.find( fn ) );
  }
  

}

void input_preprocess::process(input_node & anode)
{
  //
  // Attempt to handle input error cases where include keyword
  // is found without a value
  //
  // "include\n"
  // "include \n"
  
  const std::string incl_label( core::strngs::fsincl() );
  // look for include files in buffer
  while ( not anode.eof() )
  {
    std::string line;
    std::string name;
    std::string value;
    anode.getline( line );
    boost::algorithm::trim( line );
    // only consider lines with 'include' as first word
    if( 0 != line.find( incl_label ) ) continue;
    this->set_line( line );
    // If the name is not include (eg someone wrote include....) continue
    if( this->name() != incl_label ) continue;
    std::string filename_str{ this->dequote( this->value() ) };
    boost::algorithm::trim( filename_str );
    if( filename_str.empty() )
    {
      this->parameter_value_error( "Include file", "", core::input_error::non_empty_value_message() );
    }
    // Complete filename based on the current node's path
    boost::filesystem::path fn( filename_str );
    // reset name/value.
    this->clear();
  
    // Add include to map
    if( fn.has_relative_path() )
    {
      // All nodes should have absolute paths.
      boost::filesystem::path here( anode.path() );
      fn = here.parent_path() / fn;
    }
    this->add_include( fn );
  }
  // reset buffer position.
  anode.rewind();
  

}

// ----------------------------------------
// input line processing
// ----------------------------------------
//  dequote
// --------------------------------------------
// READ NEXT NAME, VALUE PAIR
// 
// Return false when there is no more input
//
// Reads line from the current input file ignoring blank lines and
// deleting comments beginning with "#". Comments may appear
// anywhere on line.  Included files will be automatically opened
// (and closed) as if they form one continuous input stream.
//
// Splits the line into a name, value pair.  Value may be an empty
// string.

bool input_preprocess::do_next() 
{
if (this->file_stack_.empty())
{
  return false;
}
// Keep going until we get a result or reach the end of all files.
while (true)
{
  // Get the top most stream
  input_node &fid = this->file_stack_.back()->second;
  if (fid.eof())
  {
    this->file_stack_.pop_back();
    if (this->file_stack_.empty())
    {
      return false;
    }
    continue;
  }
  std::string line;
  fid.getline( line );
  if ( not this->set_line( line ) ) continue;

  // Check for include.
  if (this->name() == core::strngs::fsincl())
  {
    boost::filesystem::path fn( this->value());
    if ( fn.has_relative_path() )
    {
      boost::filesystem::path root( fid.path() );
      if ( root.has_parent_path() )
      {
        fn = root.parent_path() / fn;
      }
    }
    // search fid for include file and update
    // stack.
    if( this->file_stack_.size() >= 100 )
    {
      throw input_error::input_file_error( this->value(), this, "Include depth exceeding 100 levels, which probably indicates an include cycle." );
    }
    this->file_stack_.push_back( this->find_include( fn ) );
    UTILITY_CHECK( this->end() != this->file_stack_.back(), "No matching input node for path "+fn.string() );
    continue;
  }
  break;
}
return true;

}


} // namespace core

BOOST_CLASS_EXPORT_IMPLEMENT( core::input_preprocess );