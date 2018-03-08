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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "geometry/distance_calculator.hpp"
#include "utility/archive.hpp"

#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"

namespace geometry {

// Calculate distance between the given position and
// all positions in the given coordinate set. The
// result vector will contain the distances in the
// same sequence as the coordinate set.
//
// NOTE: This is hidden because it should
// only be called throw a geometry_manager
// object.
//
// \pre startindex < endindex
// \pre endindex <= others.size
// \post result.size == end_index
// \post result[0:start_index] == 0.0
void distance_calculator::calculate_distances(const coordinate & pos, const coordinate_set & others, std::vector< double > & result, std::size_t startindex, std::size_t endindex) const
{
  UTILITY_REQUIRE( startindex < endindex, "Start must be before end" );
  UTILITY_REQUIRE( endindex <= others.size(), "End must be less than or equal to coordinate set size" );
  this->do_calculate_distances_sq( pos, others, result, startindex, endindex );
  // apply sqrt to squared distances
  for( std::size_t idx = startindex; idx != endindex; ++idx )
  {
    result[idx] = std::sqrt( result[idx] );
  }
  UTILITY_ENSURE( endindex = result.size(), "Result vector should have endindex entries" );

}

// Calculate distance between the given position and
// all positions in the given coordinate set. The
// result vector will contain the distances in the
// same sequence as the coordinate set.
//
// NOTE: This is hidden because it should
// only be called throw a geometry_manager
// object.
//
// \pre startindex < endindex
// \pre endindex <= others.size
// \post result.size == end_index
// \post result[0:start_index] == 0.0
void distance_calculator::calculate_distances_sq(const coordinate & pos, const coordinate_set & others, std::vector< double > & result, std::size_t startindex, std::size_t endindex) const
{
  UTILITY_REQUIRE( startindex < endindex, "Start must be before end" );
  UTILITY_REQUIRE( endindex <= others.size(), "End must be less than or equal to coordinate set size" );
  this->do_calculate_distances_sq( pos, others, result, startindex, endindex );
  UTILITY_ENSURE( endindex = result.size(), "Result vector should have endindex entries" );

}


} // namespace geometry
