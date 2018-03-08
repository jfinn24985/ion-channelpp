#ifndef IONCH_CORE_INPUT_NODE_HPP
#define IONCH_CORE_INPUT_NODE_HPP

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

#include "utility/archive.hpp"

#include <string>
#include <cstddef>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>



namespace core {

// Manage dat associated with line-wise reading of an input file. This
// includes the file name and current read position.
//
// This class follows canonical pattern.
class input_node
 {
   private:
    //Content of input file
    std::string buffer_;

    //Path of input stream
    std::string path_;

    //Current position in the buffer.
    std::size_t pos_;

    //Current line number in file
    int line_no_;


   public:
    input_node();

    ~input_node() = default;

    input_node(const input_node & source)
    : buffer_(source.buffer_)
    , path_(source.path_)
    , pos_(source.pos_)
    , line_no_(source.line_no_)
    {}

    input_node(input_node && source)
    : buffer_(std::move(source.buffer_))
    , path_(std::move(source.path_))
    , pos_(source.pos_)
    , line_no_(source.line_no_)
    {}

    input_node & operator=(input_node source)
    {
      this->swap(source);
      return *this;
    }

    void swap(input_node & source)
    {
      std::swap(buffer_, source.buffer_);  
      std::swap(path_, source.path_);
      std::swap(pos_, source.pos_);
      std::swap(line_no_, source.line_no_);
    }
    

  friend class boost::serialization::access;

   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & buffer_;
        ar & path_;
        ar & pos_;
        ar & line_no_;
      }


   public:
    //Is our current position at the end of the buffer?
    //
    // "eof" is true under the following condition:
    // *  pos_ >= input_buffer.size
    bool eof() const
    {
      return this->buffer_.size() <= this->pos_;
    }

    // Current byte position in the file/buffer
    std::size_t file_position() const
    {
       return this->pos_;
    }

    //Get the next line from the buffer
    void getline(std::string & line);

    // Current line number in the file/buffer
    std::size_t line_number() const
    {
       return this->line_no_;
    }

    // The name of the file that we obtained the buffer contents from.
    std::string path() const
    {
      return this->path_;
    }


   private:
    // Read stream into internal buffer.
    //
    // If return value is true then node contains the contents of
    // the named file. If false (or on exception) then contents are
    // unchanged. 
    bool read_stream_(std::istream & ais);


   public:
    // Work from stream, giving it the path 'filename'.
    //
    // If return value is true then node contains the contents of
    // the named file. If false (or on exception) then contents are
    // unchanged. 
    bool set_buffer(const boost::filesystem::path & filename, std::istream & ais);

    // Work from file pointed to by 'filename'.
    //
    // If return value is true then node contains the contents of
    // the named file. If false (or on exception) then contents are
    // unchanged. 
    bool set_buffer(const boost::filesystem::path & filename);

    // Work from an existing 'buffer', giving it a dummy 'filename'.
    //
    // This should always return true.
    //
    // Throws the same exceptions as std::string::opertor= 
    // and boost::filesystem::absolute.
    bool set_buffer(const boost::filesystem::path & filename, std::string buffer);

    // Reset the position and line number to the beginning of
    // the file/buffer
    void rewind()
    {
      this->pos_ = 0;
      this->line_no_ = 0;
    }


};

} // namespace core
#endif
