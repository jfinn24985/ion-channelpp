#ifndef IONCH_OBSERVABLE_ARCHIVE_FILE_HPP
#define IONCH_OBSERVABLE_ARCHIVE_FILE_HPP

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

#include <stdexcept>

#include <string>
#include "observable/base_sink.hpp"

extern "C"
{
 struct zip;
};

namespace observable {

//Error caused by differences between what
//the library expects and the real system.
class archive_error : public std::runtime_error
 {
   public:
    //Construct en error message based on an error condition
    //in a libzip method.
    archive_error(int error_number, int system_number)
    : std::runtime_error( make_error_message( error_number, system_number ) )
    {}

    virtual ~archive_error() noexcept(true) {}

    archive_error(archive_error && source)
    : std::runtime_error(  source )
    {}

    archive_error(const archive_error & source)
    : std::runtime_error(  source )
    {}

    archive_error & operator=(archive_error source)
    {
      std::swap( *this, source );
      return *this;
    }

    //Construct an error message based on a libarchive error message.
    static std::string make_error_message(int error_number, int system_number);


};
//Class that allows multiple "files" to be placed in a single
//archive file (ie only one open OS file). The archive is
//in the standard "zip" format that can be unarchived outside the
//program.
class archive_file : public base_sink
 {
   private:
    //The zip archive descriptor
    ::zip * arch_;

    // Whether there are unsaved writes.
    bool dirty_;

    //Default ctor
    archive_file() = default;


   public:
    //  Construct an archive with the given path. I the path does not end in
    //  ".zip" then append this to the path.
    archive_file(std::string path, base_sink::open_mode mode = RW_CREATE);

    //Dtor.  If the archive is open then this attempts to close the archive.
    //If the archive can not be closed, then any changes are silently 
    //ignored and the archive discarded before throwing an exception. 
    //This is to avoid having the destructor throw an error and not close
    //the zip file.
    virtual ~archive_file();


   private:
    archive_file(const archive_file & source) = delete;

    // Transfer ownership of the zip file.
    archive_file(archive_file && source) = delete;
    // Transfer ownership of the zip file.
    archive_file & operator=(archive_file source) = delete;
  friend class boost::serialization::access;
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
       ar & boost::serialization::base_object< base_sink >(*this);
       ar & dirty_;
       typename Archive::is_loading is_load;
       if ( is_load )
       {
         this->ensure_close();
       }
    }
    //Append "content" onto an existing file with the given "subpath" in the archive.
    //
    //\pre exists(subpath)
    void do_append(std::string subpath, std::string content);

    // Check that the archive is open. If it is not open, then
    // attempt to open it. Throw an exception if the archive
    // can not be openned.
    void ensure_open();

    //If the archive is open, attempt to close the archive.
    //
    //If the archive can not be closed, then any changes are silently 
    //ignored and the archive discarded before throwing an exception. 
    //This is to avoid having the destructor throw an error and not close
    //the zip file.
    void ensure_close();

    //Test if a file with the given "subpath" exists in the archive.
    bool do_exists(std::string subpath);

    //Read "content" from an existing file with the given "subpath" in the archive.
    //Returns false if subpath does not exist.
    bool do_read(std::string subpath, std::string & content);

    //Write a "content" of file with the given "subpath" into the archive.
    //Any file with the given "subpath" will be overwriten.
    void do_write(std::string subpath, std::string content);


   public:
    //Construct an error message based on a libarchive error message.
    static std::string make_error_message(int error_number, int system_number);


};

} // namespace observable
#endif

