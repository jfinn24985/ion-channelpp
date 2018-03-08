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

#include "observable/memory_sink.hpp"

namespace observable {

//Append "buffer" to an existing document at the given "path".
//
//\pre exists(path)
void memory_sink::do_append(std::string path, std::string buffer)
{
  if ( this->exists( path ) )
  {
     this->data_[ path ] += buffer;
  }
  else
  {
     this->data_.insert( std::make_pair( path, buffer ) );
  }

}

//Check for a document at the given "path".
bool memory_sink::do_exists(std::string path)
{
  return ( this->data_.end() != this->data_.find( path ) );

}

//Read the contents of a document at "path" into the "outbuffer".
//Return true if document exists and has its content read. Return
//false if the document does not exist.
bool memory_sink::do_read(std::string path, std::string & outbuffer)
{
  if ( this->exists( path ) )
  {
     outbuffer = this->data_[ path ];
     return true;
  }
  else
  {
     outbuffer.clear();
     return false;
  }

}

//Write a "content" of file with the given "subpath" into the archive.
//Any file with the given "subpath" will be overwritten.
void memory_sink::do_write(std::string path, std::string buffer)
{
  if ( this->exists( path ) )
  {
     this->data_[ path ] = buffer;
  }
  else
  {
     this->data_.insert( std::make_pair( path, buffer ) );
  }

}

memory_sink::~memory_sink() = default;


} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(observable::memory_sink, "observable::memory_sink");