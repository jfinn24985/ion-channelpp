#ifndef IONCH_UTILITY_FILESYSTEM_HPP
#define IONCH_UTILITY_FILESYSTEM_HPP

//Authors: Justin Finnerty
//
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

#include "utility/utility.hpp"
#include <fstream>
#include <boost/typeof.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/operations.hpp> 
#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>



namespace utility {

//A set of utility functions for operating on filesystem objects.
class IONCH_API filesystem {
   public:
    //Compare the binary contents of the file pointed to by a_path with
    //the contents of a "buffer" from a_start to a_end.  This reads 
    //the file as a series of elements with the same type as *a_begin.
    //
    //This is equivalent to:
    //
    //for (filestream< typeof(*a_begin) > is(path);is and a_begin != a_end; ++a_begin)
    //{
    // if (*a_begin != is.get) return false
    //}
    //return true
    //
    //For non-char types this is very different compare_content.
    template< typename IterType > static inline bool compare_binary(const std::string& a_path,  IterType a_start, IterType a_end);

    //Compare the contents of the file pointed to by a_path with
    //the contents of a "buffer" from a_start to a_end.  This reads 
    //the file as a serialized series of elements with the same type as *a_begin.
    //
    //This is very different to:
    //
    //for (fstream is(path); is and a_begin != a_end; ++a_begin)
    //{
    //   typeof(*a_begin) value;
    //   is >> value;
    //  if (*a_begin != value) return false;
    //}
    //return true;
    //
    //Note this is very different to compare_binary for non-char types.
    template< typename IterType > static inline bool compare_content(const std::string& a_path,  IterType a_start, IterType a_end);

    //Attempt to create a relative path from a_base to a_leafer.  If this 
    //is not possible complete(a_leafer) is returned.  The relative path is
    //constructed relative to the whole of a_base.  If either paths are
    //not complete, then they are made "complete".
    static boost::filesystem::path relative_path(const boost::filesystem::path & a_base, const boost::filesystem::path & a_leafer);

};
//Compare the binary contents of the file pointed to by a_path with
//the contents of a "buffer" from a_start to a_end.  This reads 
//the file as a series of elements with the same type as *a_begin.
//
//This is equivalent to:
//
//for (filestream< typeof(*a_begin) > is(path);is and a_begin != a_end; ++a_begin)
//{
// if (*a_begin != is.get) return false
//}
//return true
//
//For non-char types this is very different compare_content.
template< typename IterType > inline bool filesystem::compare_binary(const std::string& a_path,  IterType a_start, IterType a_end)
{
  typedef BOOST_TYPEOF(*a_begin) CharType;
  typedef boost::iostreams::file_source< CharType > input_source;
  typedef boost::iostreams::char_traits< CharType > char_traits;
  for (input_source ifs (a_path);
       a_begin != a_end; 
       ++a_begin)
    {
      const char_traits::int_type result = boost::iostreams::get(ifs);
      if (result == char_traits::eof())
        {
  	return false;
        }
      UTILITY_CHECK(result != char_traits::would_block(), "Simple file source should be non-blocking.");
      if (result != *a_begin)
        {
  	return false;
        }
    }
  return true;

}

//Compare the contents of the file pointed to by a_path with
//the contents of a "buffer" from a_start to a_end.  This reads 
//the file as a serialized series of elements with the same type as *a_begin.
//
//This is very different to:
//
//for (fstream is(path); is and a_begin != a_end; ++a_begin)
//{
//   typeof(*a_begin) value;
//   is >> value;
//  if (*a_begin != value) return false;
//}
//return true;
//
//Note this is very different to compare_binary for non-char types.
template< typename IterType > inline bool filesystem::compare_content(const std::string& a_path,  IterType a_start, IterType a_end)
{
  typedef BOOST_TYPEOF(*a_begin) CharType;
  for (std::ifstream ifs (a_path);
       ifs and a_begin != a_end; 
       ++a_begin)
    {
      CharType result;
      ifs >> result;
      if (result != *a_begin)
        {
  	return false;
        }
    }
  return true;

}


} // namespace utility
#endif
