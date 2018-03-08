#ifndef IONCH_OBSERVABLE_MEMORY_SINK_HPP
#define IONCH_OBSERVABLE_MEMORY_SINK_HPP

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

#include "observable/base_sink.hpp"
#include <boost/serialization/map.hpp>
#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>



namespace observable {

//Test "archive" that stores data in memory, for use in 
//testing the interface.
class memory_sink : public base_sink
 {
   private:
    //Data set map
    std::map<std::string, std::string> data_;

    //Append "buffer" to an existing document at the given "path".
    //
    //\pre exists(path)
    void do_append(std::string path, std::string buffer);

    //Check for a document at the given "path".
    bool do_exists(std::string path);


   public:
    // Get the filename of the archive
    boost::filesystem::path filename() const
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

  friend class boost::serialization::access;
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
       ar & boost::serialization::base_object< base_sink >(*this);
       ar & data_;
    }

   public:
    memory_sink()
    : base_sink( "", base_sink::RW_CREATE )
    , data_()
    {}
    memory_sink(std::string rootpath, base_sink::open_mode mode = RW_CREATE)
    : base_sink( rootpath, mode )
    , data_()
    {}

    virtual ~memory_sink();


   private:
    memory_sink(const memory_sink & source) = delete;

    memory_sink(memory_sink && source) = delete;

    memory_sink & operator=(const memory_sink & source) = delete;


};

} // namespace observable
#endif
