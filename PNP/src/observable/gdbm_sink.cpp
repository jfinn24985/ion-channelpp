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

#include "observable/gdbm_sink.hpp"

// manuals
#include "core/input_error.hpp"
#include "core/strngs.hpp"
// -
namespace observable {

//* Construct from a GDBM datum. May throw std::bad_alloc.
gdbm_sink::datum_type::datum_type(const ::datum & other)
{
  this->dptr = nullptr;
  this->dsize = 0;
  if (other.dsize > 0)
  {
     this->dptr = static_cast< char* >( ::malloc( other.dsize ) );
     if ( nullptr == this->dptr )
     {
        throw std::bad_alloc();
     }
     ::memcpy( this->dptr, other.dptr, other.dsize );
     this->dsize = other.dsize;
  }
  

}

//* Construct from a GDBM datum.
gdbm_sink::datum_type::datum_type(::datum && other)
{
  this->dptr = nullptr;
  this->dsize = 0;
  std::swap( this->dptr, other.dptr );
  std::swap( this->dsize, other.dsize );
  

}

//* Copy another gdbm_datum object. May throw std::bad_alloc.
gdbm_sink::datum_type::datum_type(const gdbm_sink::datum_type & other)
{
  this->dptr = nullptr;
  this->dsize = 0;
  if (other.dsize > 0)
  {
     this->dptr = static_cast< char* >( ::malloc( other.dsize ) );
     if ( nullptr == this->dptr )
     {
        throw std::bad_alloc();
     }
     ::memcpy( this->dptr, other.dptr, other.dsize );
     this->dsize = other.dsize;
  }
  
  

}

//* Copy another gdbm_datum object
gdbm_sink::datum_type::datum_type(gdbm_sink::datum_type && other)
{
  this->dptr = nullptr;
  this->dsize = 0;
  std::swap( this->dptr, other.dptr );
  std::swap( this->dsize, other.dsize );

}

//* Destructor
gdbm_sink::datum_type::~datum_type()
{
if ( this->dptr )
{
  ::free( this->dptr );
}

}

//* Set contents of datum object.  May throw std::bad_alloc.
void gdbm_sink::datum_type::buffer(std::string a_buffer)
{
  // a_buffer is empty
  if ( a_buffer.empty() )
  {
     if ( this->dsize > 0 )
     {
        ::free( this->dptr );
        this->dptr = nullptr;
        this->dsize = 0;
     }
     return;
  }
  else
  {
     if ( this->dsize > 0 )
     {
        // Check if buffer contents are not the same.
        if (0 != a_buffer.compare( 0, a_buffer.size(), this->dptr, this->dsize ) )
        {
           // conversion of dsize is safe as we have tested it is greater than 0
           if ( a_buffer.size() != std::size_t( this->dsize ) )
           {
              // current buffer is different size
              //
              // * If realloc fails then newptr is nul and dptr is untouched.
              //
              // * If realloc succeeds and newptr /= dptr then realloc has called free( dptr ).
              //
              // This requires using newptr to check for nul return before assigning
              // the value to dptr. In this way a failure leaves the object in a valid state.
              char * newptr = static_cast< char* >( ::realloc( this->dptr, a_buffer.size() * sizeof( char ) ) );
              if ( nullptr == newptr )
              {
                 throw std::bad_alloc();
              }
              std::swap( newptr, this->dptr );
           }
        }
     }
     else
     {
        this->dptr = static_cast< char* >( ::malloc( a_buffer.size() * sizeof( char ) ) );
        if ( nullptr == this->dptr )
        {
           throw std::bad_alloc();
        }
     }
     ::memcpy( this->dptr, a_buffer.data(), a_buffer.size() * sizeof( char ) );
     this->dsize = a_buffer.size();
  }

}

//* Get a copy of the contents of datum object
std::string gdbm_sink::datum_type::buffer() const
{
  return (this->dsize > 0 ? std::string( this->dptr, this->dsize ) : std::string() );
}

//* Append contents to a datum object.  May throw std::bad_alloc.
void gdbm_sink::datum_type::append(std::string a_buffer)
{
  // a_buffer is empty is valid input, just ignore
  if ( a_buffer.empty() )
  {
     return;
  }
  else
  {
     if ( this->dsize > 0 )
     {
             // current buffer exists
              //
              // * If realloc fails then newptr is nul and dptr is untouched.
              //
              // * If realloc succeeds and newptr /= dptr then realloc has called free( dptr ).
              //
              // This requires using newptr to check for nul return before assigning
              // the value to dptr. In this way a failure leaves the object in a valid state.
              char * newptr = static_cast< char* >( ::realloc( this->dptr, this->dsize + a_buffer.size() * sizeof( char ) ) );
              if ( nullptr == newptr )
              {
                 throw std::bad_alloc();
              }
              std::swap( newptr, this->dptr );
     }
     else
     {
        this->dptr = static_cast< char* >( ::malloc( a_buffer.size() * sizeof( char ) ) );
        if ( nullptr == this->dptr )
        {
           throw std::bad_alloc();
        }
     }
     ::memcpy( this->dptr + this->dsize, a_buffer.data(), a_buffer.size() * sizeof( char ) );
     this->dsize = this->dsize + a_buffer.size();
  }

}

// Create sink based on the given path.
gdbm_sink::gdbm_sink(std::string path, base_sink::open_mode mode)
: base_sink(  path, mode )
, dbf_( nullptr )
{
  const std::string ext( ".dbm" );
  if ( ext != this->path_.substr( this->path_.size() - ext.size() ) )
  {
     this->path_.append( ext );
  }

}

gdbm_sink::~gdbm_sink()
{
   this->ensure_close();
}

//Append "buffer" to an existing document at the given "path".
//
//\pre exists(path)
void gdbm_sink::do_append(std::string path, std::string buffer)
{
  // Empty buffer is allowed, but ignored.
  UTILITY_CHECK( not buffer.empty(), "append method should ensure buffer is not empty before calling append_priv." );
  
  this->ensure_open();
  datum_type key;
  key.buffer( path );
  datum_type content( std::move( ::gdbm_fetch( static_cast< GDBM_FILE >(this->dbf_), key ) ) );
  content.append( buffer );
  ::gdbm_store( static_cast< GDBM_FILE >(this->dbf_), key, content, GDBM_REPLACE );
  

}

//Open the DB file if it is not already open.
void gdbm_sink::ensure_open()
{
  UTILITY_REQUIRE( not this->path_.empty(), "Attempt to use sink before setting path" );
  if( nullptr == this->dbf_ )
  {
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
    int flag;
    bool must_exist = false;
    switch( this->mode() )
    {
    case READ_ONLY:
      flag = GDBM_READER;
      must_exist = true;
      break;
    case READ_WRITE:
      flag = GDBM_WRITER;
      must_exist = true;
      break;
    case RW_CREATE:
      flag = GDBM_WRCREAT;
      break;
    case NEW_SINK:
      flag = GDBM_NEWDB;
      break;
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
    this->dbf_ = ::gdbm_open( const_cast< char* >( this->path_.c_str() ), 4096, flag, 0644, nullptr );
    if( nullptr == this->dbf_ )
    {
      // error occurred
      std::string mesg = ::gdbm_strerror( ::gdbm_errno );
      throw core::input_error::input_logic_error( "Possible problem with \"" + core::strngs::outputdir_label() + "\" value", core::strngs::simulator_label(), mesg );
    }
  }

}

//Close the DB file if it is not already closed.
void gdbm_sink::ensure_close()
{
  if ( nullptr != this->dbf_ )
  {
     ::gdbm_close( static_cast< GDBM_FILE >(this->dbf_) );
     this->dbf_ = nullptr;
  }
  

}

//Check for a document at the given "path".
bool gdbm_sink::do_exists(std::string path)
{
  this->ensure_open();
  datum_type key;
  key.buffer( path );
  return 0 != ::gdbm_exists( static_cast< GDBM_FILE >(this->dbf_), key );

}

//Read the contents of a document at "path" into the "outbuffer".
//Return true if document exists and has its content read. Return
//false if the document does not exist.
bool gdbm_sink::do_read(std::string path, std::string & outbuffer)
{
  UTILITY_REQUIRE( not path.empty(), "Can not read from empty path." );
  this->ensure_open();
  datum_type key;
  key.buffer( path );
  if ( 0 != ::gdbm_exists( static_cast< GDBM_FILE >(this->dbf_), key ) )
  {
     datum_type content( std::move( ::gdbm_fetch( static_cast< GDBM_FILE >(this->dbf_), key ) ) );
     outbuffer = content.buffer();
     return true;
  }
  else
  {
     return false;
  }

}

//Write a "content" of file with the given "subpath" into the archive.
//Any file with the given "subpath" will be overwritten.
void gdbm_sink::do_write(std::string path, std::string buffer)
{
  this->ensure_open();
  datum_type key, content;
  key.buffer( path );
  content.buffer( buffer );
  ::gdbm_store( static_cast< GDBM_FILE >(this->dbf_), key, content, GDBM_REPLACE );

}


} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(observable::gdbm_sink, "observable::gdbm_sink");