#ifndef IONCH_UTILITY_MULTI_ARRAY_H
#define IONCH_UTILITY_MULTI_ARRAY_H

//Hook boost::mulit_array into boost::serialization machinery
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

#include <boost/multi_array.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/array.hpp>

namespace boost
{
namespace serialization
{
// Save a multi_array
template<class Archive, typename ValueType, std::size_t NumDims, typename Allocator = std::allocator<ValueType> >
void save(Archive & ar, const boost::multi_array< ValueType, NumDims, Allocator > & t, unsigned int version)
{
  ar << boost::serialization::make_nvp("dimensions",
                                       boost::serialization::make_array(t.shape(), NumDims));
  ar << boost::serialization::make_nvp("data",
                                       boost::serialization::make_array(t.data(), t.num_elements()));
}

// Load a multi_array
template<class Archive, typename ValueType, std::size_t NumDims, typename Allocator = std::allocator<ValueType> >
void load(Archive & ar, boost::multi_array< ValueType, NumDims, Allocator  > & t, unsigned int version)
{
  typedef typename boost::multi_array< ValueType, NumDims, Allocator>::size_type size_type;
  boost::array< size_type, NumDims > dimensions;
  ar >> boost::serialization::make_nvp("dimensions",
                                       boost::serialization::make_array(dimensions.c_array(), NumDims));
  t.resize(dimensions);
  ar >> boost::serialization::make_nvp("data",
                                       boost::serialization::make_array(t.data(), t.num_elements()));
}

template<class Archive, typename ValueType, std::size_t NumDims, typename Allocator = std::allocator<ValueType> >
inline void serialize(
  Archive & ar,
  boost::multi_array< ValueType, NumDims, Allocator  > & t,
  const unsigned int file_version
)
{
  split_free(ar, t, file_version);
}
}
}


//
namespace utility {


} // namespace utility


#endif