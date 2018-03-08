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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "core/input_reader.hpp"
#include "core/strngs.hpp"

//--
#include "core/constants.hpp"
#include "core/input_error.hpp"
#include "utility/utility.hpp"

#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace core {

// Generate a reader using the filename of the top-level file.
input_reader::input_reader(std::string filename)
: input_base_reader()
, file_stack_(1)
{
  this->file_stack_[0].set_buffer( filename );
}

// Generate a reader using a (possibly dummy) filename and a
// preexisting input stream.
input_reader::input_reader(std::string filename, std::istream& ais)  
: input_base_reader()
, file_stack_(1)
{
  this->file_stack_[0].set_buffer(filename, ais);
}

input_reader::~input_reader() = default;

// --------------------------------------------------
// Observer methods
// --------------------------------------------
// ----------------------------------------
// mutating methods
// ----------------------------------------
// Add a preread buffer with a (possibly dummy) filename.
void input_reader::add_buffer(const boost::filesystem::path & filename, std::string buffer)
{
if( this->file_stack_.size() >= 100 )
{
  throw input_error::input_file_error( filename.native(), this, "Include depth exceeded, check that input files are not including each other in a loop." );
}
// leave filename as-is
this->file_stack_.push_back( input_node{} );
this->file_stack_.back().set_buffer( filename.string(), buffer );


}

// Add an included file.
void input_reader::add_include(const boost::filesystem::path & filename)
{
if( this->file_stack_.size() >= 100 )
{
  throw input_error::input_file_error( filename.native(), this, "Include depth exceeded, check that input files are not including each other in a loop." );
}
boost::filesystem::path fullpath( filename );
if ( fullpath.has_relative_path() )
{
  boost::filesystem::path here( this->current_filename() );
  if ( here.has_parent_path() )
  {
    fullpath = here.parent_path() / fullpath;
  }
  // else assume is relative to working directory.
}
this->file_stack_.push_back( input_node{} );
this->file_stack_.back().set_buffer( fullpath.string() );


}

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

bool input_reader::do_next() 
{
if ( this->file_stack_.empty() )
{
  return false;
}
// Keep going until we get a result or reach the end of all files.
while ( true )
{
  // Get the top most input node
  input_node &fid = this->file_stack_.back();
  if ( fid.eof() )
  {
    this->file_stack_.pop_back();
    if( this->file_stack_.empty() )
    {
      return false;
    }
    continue;
  }
  std::string line;
  fid.getline( line );

  if ( not this->set_line( line ) ) continue;

  // Check for include.
  if ( this->name() == core::strngs::fsincl() )
  {
    this->add_include( this->value() );
    continue;
  }
  break;
}
return true;


}


} // namespace core
