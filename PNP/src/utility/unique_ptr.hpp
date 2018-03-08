#ifndef IONCH_UTILITY_UNIQUE_PTR_HPP
#define IONCH_UTILITY_UNIQUE_PTR_HPP
#include <memory>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

namespace boost
{
namespace serialization
{
// Save single value unique_ptr
template<class Archive, typename ValueType, typename DeAllocator >
void save(Archive & ar, const std::unique_ptr< ValueType, DeAllocator > & t, unsigned int version)
{
  const ValueType *const tx (t.get());
  ar << boost::serialization::make_nvp("unique_ptr", tx);
}

// Load single value unique_ptr
template<class Archive, typename ValueType, typename DeAllocator >
void load(Archive & ar, std::unique_ptr< ValueType, DeAllocator > & t, unsigned int version)
{
  ValueType * tx;
  ar >> boost::serialization::make_nvp("unique_ptr", tx);
  t.reset(tx);
}

template<class Archive, typename ValueType, typename DeAllocator >
inline void serialize(
  Archive & ar,
  std::unique_ptr< ValueType, DeAllocator > & t,
  const unsigned int file_version
)
{
  split_free(ar, t, file_version);
}

// Array value unique_ptr serialization is undefined
template<class Archive, typename ValueType, typename DeAllocator >
inline void serialize(
  Archive & ar,
  std::unique_ptr< ValueType[], DeAllocator > & t,
  const unsigned int file_version
);


}
}


//
// Hook std::unique_ptr into boost::serialization machinery
// Author Justin Finnerty
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

namespace utility {


} // namespace utility
#endif
