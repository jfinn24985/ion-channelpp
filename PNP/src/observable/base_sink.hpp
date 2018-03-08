#ifndef IONCH_OBSERVABLE_BASE_SINK_HPP
#define IONCH_OBSERVABLE_BASE_SINK_HPP

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

#include "utility/archive.hpp"

#include "observable/output_file.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


#include <iostream>
#include "utility/unique_ptr.hpp"

// manual includes
#include "utility/utility.hpp"
//- 
namespace observable { struct output_datum; } 

namespace observable {

// Interface to allow output documents to be written in a
// controlled manner.  Derived classes could be implemented
// to use standard files or some sort of archive (ie only
// one open OS file).
//
// NOTE: possible inconsistent behaviour among derived
// classes: Sink classes are intended to manage saving
// results to permanent storage. In this regard it is
// expected that the results of 'read'/'exists' calls will
// depend on calls to 'write'/'append' regardless of when
// an object is serialized or deserialized. (i.e.  calls to
// 'write'/'append' between serialization and deserialization
// will be reflected in results of 'read'/'exists'.) However,
// a sink may be implemented only in memory in which case
// the results of 'read'/'exists' calls will reflect
// the state at the point of serialization. (i.e.
// calls to 'write'/'append' between serialization and
// deserialization will _NOT_ be reflected in results of
// 'read'/'exists'.) Such inconsistent behaviour is a
// "feature" not a "bug".

class base_sink
 {
   public:
    // sink open modes. 
    //
    // These are advisory.  Sink classes should attempt to support
    // these values as much as possible.  Errors may be reported
    // using exceptions. 
    enum open_mode
     {
      READ_ONLY,// Open existing sink for reading
      READ_WRITE,// Open existing sink for reading/writing
      RW_CREATE,// Open sink for reading/writing, create new sink if 
      // it doesn't exist.
      NEW_SINK// Open new sink for reading/writing, replaces old sink if 
      // it exists.

    };


   private:
    // The simulations data base.
    boost::ptr_vector< output_dataset > data_sets_;


   protected:
    //The path of the archive in the OS filesystem.
    std::string path_;


   private:
    // The unique identifier for a simulation
    std::string uuid_;

    // How to open the sink.
    open_mode mode_;


   protected:
    // Constructor for serialization.
    base_sink()
    : data_sets_()
    , path_()
    , uuid_()
    , mode_()
    {}


   public:
    //Construct a sink based on the given path.
    base_sink(std::string path, open_mode mode)
    : path_( path )
    , uuid_( make_uuid_string() )
    , mode_( mode )
    {}

    virtual ~base_sink() {};


   private:
    base_sink(const base_sink & source) = delete;
    base_sink(base_sink && source) = delete;

    base_sink & operator=(const base_sink & source) = delete;

  friend class boost::serialization::access;
    // Serialize the sink class. 
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
       ar & this->data_sets_; ar & this->path_; ar & this->uuid_; ar & this->mode_;
    }

   public:
    //Append "buffer" to an existing document at the given "path".
    //
    //\pre exists(path) : strong exception safety
    void append(std::string path, std::string buffer)
    {
       UTILITY_REQUIRE( not path.empty(), "Can not append to an empty location/path." );
       if ( not buffer.empty() ) // empty buffer is not an error
          this->do_append( path, buffer );
    }


   private:
    //Append "buffer" to an existing document at the given "path".
    //
    //\pre exists(path)
    virtual void do_append(std::string path, std::string buffer) = 0;


   public:
    //Check for a document at the given "path".
    //
    //\pre not path.empty : strong exception safety
    bool exists(std::string path)
    {
       UTILITY_REQUIRE( not path.empty(), "Can not check for an empty location/path." );
       return this->do_exists( path );
    }


   private:
    //Check for a document at the given "path".
    virtual bool do_exists(std::string path) = 0;


   public:
    // Get the path of the root of the output data. For
    // single file archives this will be the filename
    boost::filesystem::path filename() const
    {
      return this->path_;
    }


   private:
    //Generate a UUID string
    static std::string make_uuid_string();


   public:
    // Get the sink's open mode.
    open_mode mode() const
    {
      return this->mode_;
    }

    //Read the contents of a document at "path" into the "outbuffer".
    //Return true if document exists and has its content read. Return
    //false if the document does not exist.
    //
    //\pre not path.empty : strong exception safety
    bool read(std::string path, std::string & outbuffer)
    {
       UTILITY_REQUIRE( not path.empty(), "Can not read from an empty location/path." );
       return this->do_read( path, outbuffer );
    }


   private:
    //Read the contents of a document at "path" into the "outbuffer".
    //Return true if document exists and has its content read. Return
    //false if the document does not exist.
    virtual bool do_read(std::string path, std::string & outbuffer) = 0;


   public:
    // Get the run UUID string
    std::string uuid() const
    {
      return this->uuid_;
    }

    //Write a "content" of file with the given "subpath" into the archive.
    //Any file with the given "subpath" will be overwritten.
    //
    //\pre mode /= READ_ONLY
    //\pre not path.empty and not buffer.empty : strong exception safety
    void write(std::string path, std::string buffer)
    {
       UTILITY_REQUIRE( this->mode() != READ_ONLY, "Can not write to read-only sink." );
       UTILITY_REQUIRE( not path.empty(), "Can not write to an empty location/path." );
       UTILITY_REQUIRE( not buffer.empty(), "Can not write without content." );
       return this->do_write( path, buffer );
    }


   private:
    //Write a "content" of file with the given "subpath" into the archive.
    //Any file with the given "subpath" will be overwritten.
    virtual void do_write(std::string path, std::string buffer) = 0;


   public:
    // Write UUID header line
    void header(std::ostream & os);

    // Write UUID to description stream
    void description(std::ostream & os);

    // Is this dataset defined
    bool has_dataset(std::string label) const;

    // Add a data set definition
    //
    // \pre not has_dataset( dataset.label )
    // \post has_dataset( dataset.label )
    void add_dataset(std::unique_ptr< output_dataset > dataset);

    // Move data into the set object.
    //
    // \pre has_dataset( label )
    void receive_data(std::string label, std::unique_ptr< output_datum > datum);

    // Write all of the data sets.
    void write_dataset();


};

} // namespace observable
#endif
