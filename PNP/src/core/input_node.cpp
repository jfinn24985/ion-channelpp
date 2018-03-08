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

#include "core/input_node.hpp"

// manual include
#include <fstream>
// -
namespace core {

input_node::input_node() 
: buffer_()
, path_()
, pos_(0)
, line_no_(-1)
{}

//Get the next line from the buffer
void input_node::getline(std::string & line)
{
  if (this->pos_ > this->buffer_.size())
  {
     // Handle case of reading past last line of file
     line.clear();
     return;
  }
  else if (this->pos_ == this->buffer_.size())
  {
     // Handle case of last line of file being '\n{EOF}'
     line.clear();
     ++this->pos_;
     return;
  }
  else
  {
     std::string::size_type ipos = this->buffer_.find('\n', this->pos_);
     // Handle case of last line of buffer when ipos = npos
     if (ipos == std::string::npos)
     {
        line = this->buffer_.substr( this->pos_, ipos );
        this->pos_ = this->buffer_.size();
     }
     else
     {
        line = this->buffer_.substr( this->pos_, ipos - this->pos_ );
        this->pos_ = ipos + 1;
     }
  }
  ++this->line_no_;

}

// Read stream into internal buffer.
//
// If return value is true then node contains the contents of
// the named file. If false (or on exception) then contents are
// unchanged. 
bool input_node::read_stream_(std::istream & ais)
{
  if( ais )
  {
    // read stream into temp buffer
    std::string temp_buffer;
    ais.seekg( 0, std::ios::end );
    std::streampos length = ais.tellg();
    ais.seekg( 0, std::ios::beg );
    temp_buffer.reserve( length );
    temp_buffer.assign( ( std::istreambuf_iterator<char>( ais ) ), ( std::istreambuf_iterator<char>() ) );
    // If all ok, swap temp buffer with out buffer.
    std::swap( temp_buffer, this->buffer_ );
    this->rewind();
    return true;
  }
  else
  {
    return false;
  }
  

}

// Work from stream, giving it the path 'filename'.
//
// If return value is true then node contains the contents of
// the named file. If false (or on exception) then contents are
// unchanged. 
bool input_node::set_buffer(const boost::filesystem::path & filename, std::istream & ais)
{
  this->path_ = boost::filesystem::absolute( filename ).string();
  // read stream into our buffer
  return this->read_stream_( ais );

}

// Work from file pointed to by 'filename'.
//
// If return value is true then node contains the contents of
// the named file. If false (or on exception) then contents are
// unchanged. 
bool input_node::set_buffer(const boost::filesystem::path & filename)
{
  this->path_ = boost::filesystem::absolute( filename ).string();
  // read file into our buffer stream
  std::ifstream ifs( this->path_.c_str() );
  return this->read_stream_( ifs );

}

// Work from an existing 'buffer', giving it a dummy 'filename'.
//
// This should always return true.
//
// Throws the same exceptions as std::string::opertor= 
// and boost::filesystem::absolute.
bool input_node::set_buffer(const boost::filesystem::path & filename, std::string buffer)
{
  this->path_ = boost::filesystem::absolute( filename ).string();
  std::swap( this->buffer_, buffer );
  this->rewind();
  return true;

}


} // namespace core
