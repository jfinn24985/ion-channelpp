#ifndef IONCH_CORE_INPUT_PREPROCESS_HPP
#define IONCH_CORE_INPUT_PREPROCESS_HPP

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

#include "core/input_base_reader.hpp"
#include "core/input_node.hpp"
#include <boost/serialization/map.hpp>
#include <string>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


#include <cstddef>

namespace core { class strngs; } 

namespace core {

class input_preprocess;

//  ----------------------------------------------------------------------
//  INPUT FILE READER AGGREGATION
//
//  This class provides the ability to read all the input files
//  into buffers before attempting to process the input.  This
//  class supports serialization which
//  allows the input document to be read by one thread and then
//  distributed to other threads in a parallel application. It detects
//  circular references by checking each include file resolves to
//  a unique name and limiting the include stack.
//  

class input_preprocess : public input_base_reader
 {
   public:
    typedef std::map< std::string, input_node >::iterator iterator;

    typedef std::map< std::string, input_node >::const_iterator const_iterator;


   private:
    //Map of paths to files
    std::map< std::string, input_node > includes_;

    //The set of pointers to nodes.
    std::vector< iterator > file_stack_;

    //The path of the root input buffer 
    std::string root_node_;

// --------------------------------------------------
// Lifetime methods
// --------------------------------------------------

   public:
    // Generate a reader.
    input_preprocess();

    virtual ~input_preprocess();


   private:
    // invalid operation
    input_preprocess(const input_preprocess & ) = delete;

    // invalid operation
    input_preprocess(input_preprocess && source) = delete;

    // invalid operation
    const input_preprocess & operator =(const input_preprocess & );

  friend class boost::serialization::access;
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version);
// --------------------------------------------------
// Observer methods
// --------------------------------------------

   public:
    iterator begin()
    {
      return this->includes_.begin();
    }

    const_iterator begin() const
    {
      return this->includes_.begin();
    }

    // Get the name of the file that is currently being processed.
    // If there is not current file then an empty string or None is returned
    std::string current_filename() const override
    {
      return ( this->file_stack_.empty() ? std::string() : this->file_stack_.back()->second.path() );
    }

    // Get the line number of the file that is currently being
    // processed. If there is no current file then 0(C++) or
    // -1(python) is returned.
    std::size_t current_line_number() const override
    {
      return ( this->file_stack_.empty() ? 0 : this->file_stack_.back()->second.line_number() );
    } 

    iterator end()
    {
      return this->includes_.end();
    }

    const_iterator end() const
    {
      return this->includes_.end();
    }

    //Find (and return) an include file with the given name. Return
    //'end' if not found.
    iterator find_include(const boost::filesystem::path & filename);

// ----------------------------------------
// mutating methods
// ----------------------------------------
    // Add a new buffer to the input document.
    void add_buffer(const boost::filesystem::path & filename, std::string buffer) override;

    // Add a new include file to the input document.
    //
    // This method reads the given file and all included file into
    // the reader object. 
    void add_include(const boost::filesystem::path & filename) override;


   private:
    void process(input_node & anode);

// ----------------------------------------
// input line processing
// ----------------------------------------
    // READ NEXT NAME,VALUE PAIR
    // 
    // Return false when there is no more input
    //
    // Reads line from the current input file skipping blank lines and
    // deleting comments beginning with "#". Comments may appear
    // anywhere on line. Lines that only contain whitespace and or
    // comments are considered to be blank lines.
    //
    // Splits the line into a name, value pair.  Value may be an empty
    // string.
    //
    // (In 'include' statements C++ should handle relative paths, 
    // python currently does not)
    
    bool do_next() override;


};
// --------------------------------------------------
// Lifetime methods
// --------------------------------------------------
template<class Archive> inline void input_preprocess::serialize(Archive & ar, const unsigned int version)
{
  ar & boost::serialization::base_object< input_base_reader >(*this);
  ar & includes_;
  ar & root_node_;
  typename Archive::is_loading isload;
  if ( isload )
  {
     // reconstruct from paths.
     std::size_t count;
     ar & count;
     if (count > 0)
     {
        for (std::size_t ii = 0; ii != count; ++ii)
        {
           std::string pp;
           ar & pp;
           file_stack_.push_back( this->includes_.find( pp ) );
        }
     }
  }
  else
  {
     // save paths instead of iterators.
     std::size_t count = file_stack_.size();
     ar & count;
     if ( not file_stack_.empty() )
     {
        for (auto const& ii : file_stack_)
        {
           std::string pp = ii->first;
           ar & pp;
        }
     }
  }
  

}


} // namespace core

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( core::input_preprocess );
#endif
