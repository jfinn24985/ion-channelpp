#ifndef IONCH_OBSERVABLE_GDBM_SINK_HPP
#define IONCH_OBSERVABLE_GDBM_SINK_HPP

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

#include "observable/base_sink.hpp"
#include <gdbm.h>

#include <string>

namespace observable {

// * Provide a sink interface to the gdbm library.
// * 
// * This class provides a sink container based on the C-based gdbm
// * library. Code compiled with this type needs to include the libgdbm library 
// * using \c -lgdbm. stringtype is used for keys and values.
//
// datum type is 
// struct datum{
//  void * dptr;
//  int dsize;
//};
class gdbm_sink : public base_sink
 {
   public:
    // C++ wrapper for gdbm datum type
    class datum_type : public ::datum
     {
       public:
        datum_type()
        {
          this->dptr = nullptr;
          this->dsize = 0;
        }

        //* Construct from a GDBM datum. May throw std::bad_alloc.
        datum_type(const ::datum & other);

        //* Construct from a GDBM datum.
        datum_type(::datum && other);

        //* Copy another gdbm_datum object. May throw std::bad_alloc.
        datum_type(const datum_type & other);

        //* Copy another gdbm_datum object
        datum_type(datum_type && other);

        //* Destructor
        ~datum_type();

        datum_type & operator =(datum_type a_other)
        {
           this->swap (a_other);
           return *this;
        }

        //* Swap contents with another gdbm_datum
        void swap(datum_type & a_other)
        {
           std::swap( this->dptr, a_other.dptr );
           std::swap( this->dsize, a_other.dsize );
         }

        //* Set contents of datum object.  May throw std::bad_alloc.
        void buffer(std::string a_buffer);

        //* Get a copy of the contents of datum object
        std::string buffer() const;

        //* Append contents to a datum object.  May throw std::bad_alloc.
        void append(std::string a_buffer);


    };
    

   private:
    void * dbf_;

    gdbm_sink() = default;


   public:
    // Create sink based on the given path.
    gdbm_sink(std::string path, base_sink::open_mode mode = RW_CREATE);

    virtual ~gdbm_sink();


   private:
    gdbm_sink(const gdbm_sink & source) = delete;

    gdbm_sink(gdbm_sink && source) = delete;
    // Transfer ownership of the zip file.
    gdbm_sink & operator=(gdbm_sink source) = delete;
  friend class boost::serialization::access;
    // Serialize the sink class. Not all 
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
       ar & boost::serialization::base_object< base_sink >(*this);
       typename Archive::is_loading is_load;
       if ( is_load )
       {
         this->ensure_close();
       }
    }
    //Append "buffer" to an existing document at the given "path".
    //
    //\pre exists(path)
    void do_append(std::string path, std::string buffer);

    //Open the DB file if it is not already open.
    void ensure_open();

    //Close the DB file if it is not already closed.
    void ensure_close();

    //Check for a document at the given "path".
    bool do_exists(std::string path);


   public:
    //Get the filename of the GDBM file
    std::string filename() const
    {
      return this->path_;
    }


   private:
    //Read the contents of a document at "path" into the "outbuffer".
    //Return true if document exists and has its content read. Return
    //false if the document does not exist.
    bool do_read(std::string path, std::string & outbuffer);

    //Write a "content" of file with the given "subpath" into the archive.
    //Any file with the given "subpath" will be overwritten.
    void do_write(std::string path, std::string buffer);


};

} // namespace observable
#endif
