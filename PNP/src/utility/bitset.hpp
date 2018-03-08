#ifndef IONCH_UTILITY_BITSET_H
#define IONCH_UTILITY_BITSET_H

// Hook std::bitset into boost::serialization machinery
// Authors: Justin Finnerty
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

#include <bitset>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

namespace boost
{
namespace serialization
{
// Save a bitset
template<class Archive, std::size_t N >
void save(Archive & ar, const std::bitset< N > & t, unsigned int version)
{
   const unsigned long val{ t.to_ulong() };
   ar << boost::serialization::make_nvp( "bits", val );
}

// Load a multi_array
template<class Archive, std::size_t N >
void load(Archive & ar, std::bitset< N > & t, unsigned int version)
{
   unsigned long val;
   ar >> boost::serialization::make_nvp( "bits", val );
   t = std::bitset< N >( val );
}

template<class Archive, std::size_t N >
inline void serialize(
   Archive & ar,
   std::bitset< N > & t,
   const unsigned int file_version
)
{
   split_free(ar, t, file_version);
}
}
}

#endif
