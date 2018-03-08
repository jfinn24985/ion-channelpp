//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or (at
//your option) any later version.
//
//This program is distributed in the hope that it will be useful, but
//WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------


#ifndef DEBUG
#define DEBUG 0
#endif
extern "C"
{
#include <zip.h>
}
#include "observable/archive_file.hpp"


#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "utility/utility.hpp"

#include <iostream>
#include <new>
namespace observable {

//Construct an error message based on a libarchive error message.
std::string archive_error::make_error_message(int error_number, int system_number)
{
  char buf[4001];
  zip_error_to_str(buf, 4000, error_number, system_number); 
  return std::string(buf);

}

//  Construct an archive with the given path. I the path does not end in
//  ".zip" then append this to the path.
archive_file::archive_file(std::string path, base_sink::open_mode mode)
: base_sink( path, mode )
, arch_( nullptr )
, dirty_( false )
{
  const std::string ext( ".zip" );
  if ( ext != this->path_.substr( this->path_.size() - ext.size() ) )
  {
     this->path_.append( ext );
  }

}

//Dtor.  If the archive is open then this attempts to close the archive.
//If the archive can not be closed, then any changes are silently 
//ignored and the archive discarded before throwing an exception. 
//This is to avoid having the destructor throw an error and not close
//the zip file.
archive_file::~archive_file()
{
   this->ensure_close();
}

//Append "content" onto an existing file with the given "subpath" in the archive.
//
//\pre exists(subpath)
void archive_file::do_append(std::string subpath, std::string content)
{
  //DEBUG std::cerr << "APPEND TO : " << subpath << "\n";
  if ( this->exists( subpath ) )
  {
     // Reading could trigger close/open, so
     // we only call it if path exists.
     std::string oldcontent;
     if (this->read( subpath, oldcontent ))
     {
        oldcontent += content;
        std::swap( content, oldcontent );
     }
  }
  this->write( subpath, content );
  this->dirty_ = true;

}

// Check that the archive is open. If it is not open, then
// attempt to open it. Throw an exception if the archive
// can not be openned.
void archive_file::ensure_open()
{
  if( nullptr == this->arch_ )
  {
    //DEBUG std::cerr << "OPEN : " << this->path_ << "\n";
    // Check path exits
    boost::filesystem::path parent( boost::filesystem::path( this->path_ ).parent_path() );
    if( not parent.empty() )
    {
      if( not boost::filesystem::exists( parent ) )
      {
        boost::filesystem::create_directories( parent );
      }
      if( not boost::filesystem::is_directory( parent ) )
      {
        throw core::input_error::input_logic_error( "Problem with \"" + core::strngs::outputdir_label() + "\" value", core::strngs::simulator_label(), "Unable to create directory " + parent.string() + "." );
      }
    }
    // open file and test for success.
    int flag = 0;
    bool must_exist = false;
    switch( this->mode() )
    {
    case READ_ONLY: // == 0 : must exist (fall through)
    case READ_WRITE: // == 0 : must exist
      must_exist = true;
      break;
    case RW_CREATE:  // ok not to exist
      flag = ZIP_CREATE;
      break;
    case NEW_SINK: // create or truncate to zero size
      flag = ZIP_CREATE | ZIP_TRUNCATE;
    default:
    {
      UTILITY_ALWAYS( false, "Should never be here!" );
      break;
    }
    }
    if( must_exist )
    {
      if( not boost::filesystem::exists( this->path_ ) )
      {
        throw core::input_error::input_logic_error( "Problem with \"" + core::strngs::outputdir_label() + "\" value", core::strngs::simulator_label(), "Request to open archive file " + this->path_ + " but file does not exist." );
      }
    }
    int error;
    this->arch_ = ::zip_open( this->path_.c_str(), flag, &error );
    if( nullptr == this->arch_ )
    {
      throw archive_error( error, 0 );
    }
    this->dirty_ = false;
  }

}

//If the archive is open, attempt to close the archive.
//
//If the archive can not be closed, then any changes are silently 
//ignored and the archive discarded before throwing an exception. 
//This is to avoid having the destructor throw an error and not close
//the zip file.
void archive_file::ensure_close()
{
  if (nullptr != this->arch_)
  {
     //DEBUG std::cerr << "CLOSE : " << this->path_ << "\n";
     if (0 != ::zip_close( this->arch_ ))
     {
        int ze, se;
        ::zip_error_get( this->arch_, &ze, &se );
        throw archive_error( ze, se );
     }
     this->dirty_ = false;
     this->arch_ = nullptr;
  }

}

//Test if a file with the given "subpath" exists in the archive.
bool archive_file::do_exists(std::string subpath)
{
  this->ensure_open();
  int idx = ::zip_name_locate( this->arch_, subpath.c_str(), 0 );
  // Check that -1 means noentry and not an error
  if (idx == -1)
  {
     // file not found?
     int ze, se;
     ::zip_error_get( this->arch_, &ze, &se );
     if (ZIP_ER_NOENT != ze)
     {
         throw archive_error( ze, se );
     }
     return false;
  }
  else
  {
     return true;
  }

}

//Read "content" from an existing file with the given "subpath" in the archive.
//Returns false if subpath does not exist.
bool archive_file::do_read(std::string subpath, std::string & content)
{
  //DEBUG std::cerr << "READ FROM : " << subpath << "\n";
  if (this->dirty_)
  {
     this->ensure_close();
  }
  this->ensure_open();
  int idx = ::zip_name_locate( this->arch_, subpath.c_str(), 0 );
  if (idx == -1)
  {
     // file not found?
     int ze, se;
     ::zip_error_get( this->arch_, &ze, &se );
     if (ZIP_ER_NOENT != ze)
     {
         throw archive_error( ze, se );
     }
     return false;
  }
  
  ::zip_file * target = ::zip_fopen_index( this->arch_, idx, 0 );
  if (not target)
  {
     int ze, se;
     ::zip_error_get( this->arch_, &ze, &se );
     throw archive_error( ze, se );
  }
  struct onexit
  {
     ::zip_file * t_;
     onexit( ::zip_file * t ): t_( t ) {}
     ~onexit() {
        if (t_)
        {
          int err = ::zip_fclose( t_ );
          if (err != 0)
          {
             throw archive_error( err, 0 );
          }
        }
     }
  } tmp( target );
  
  bool at_eof = false;
  std::vector< char > buffer;
  buffer.resize( 4096 );
  content.clear();
  while(not at_eof)
  {
     zip_int64_t size = ::zip_fread( target, buffer.data(), 4096 );
     if (size > 0)
     {
        const std::size_t sz = content.size();
        content.resize( sz + size );
        std::copy( buffer.begin(), buffer.begin() + size, content.begin() + sz);
     }
     at_eof = (size != 4096);
  }
  return true;

}

//Write a "content" of file with the given "subpath" into the archive.
//Any file with the given "subpath" will be overwriten.
void archive_file::do_write(std::string subpath, std::string content)
{
  this->ensure_open();
  //DEBUG std::cerr << "WRITE TO : " << subpath << "\n";
  // Create character buffer for file content.
  char * sbuf = static_cast< char* >( ::malloc( content.size() ) );
  if (sbuf == nullptr)
  {
     throw std::bad_alloc();
  }
  std::copy(content.begin(), content.end(), sbuf);
  ::zip_source * buf = ::zip_source_buffer( this->arch_, sbuf, content.size(), 1);
  
  // check if filename already exists
  int idx = ::zip_name_locate( this->arch_, subpath.c_str(), 0 );
  if (idx == -1)
  {
     // file not found?
     int ze, se;
     ::zip_error_get( this->arch_, &ze, &se );
     if (ZIP_ER_NOENT != ze)
     {
         throw archive_error( ze, se );
     }
     idx = ::zip_add( this->arch_, subpath.c_str(), buf );
  }
  else
  {
      // Replace existing content
      idx = ::zip_replace( this->arch_, idx, buf );
  }
  if (idx == -1)
  {
     // Zip operation failed, decrement count on buf.
     ::zip_source_free( buf );
     int ze, se;
     ::zip_error_get( this->arch_, &ze, &se );
     throw archive_error( ze, se );
  }
  this->dirty_ = true;
  

}

//Construct an error message based on a libarchive error message.
std::string archive_file::make_error_message(int error_number, int system_number)
{
  char buf[4001];
  zip_error_to_str(buf, 4000, error_number, system_number); 
  return std::string(buf);

}


} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(observable::archive_file, "observable::archive_file");