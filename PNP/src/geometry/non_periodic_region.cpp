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

#include "geometry/non_periodic_region.hpp"
#include "utility/archive.hpp"

#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"

namespace geometry {

non_periodic_region::non_periodic_region(std::string label)
: base_region( label )
{}
non_periodic_region::~non_periodic_region() = default;

//Compute the distances between given position and existing positions.
//
//\pre rij.size == endindex
//\post rij[0:startindex] === 0
void non_periodic_region::do_calculate_distances_sq(const coordinate & pos, const coordinate_set & others, std::vector< double >& result, std::size_t startindex, std::size_t endindex) const
{
  const geometry::coordinate newpos(pos);
  result.resize( endindex );
  if( startindex != 0 )
  {
    for (std::size_t jj = 0; jj != startindex; ++jj)
    {
      result[jj] = 0.0;
    }
  }
  for (std::size_t jj = startindex; jj != endindex; ++jj)
  {
     // --------------
     // Calculate r_ij
     const double dx = newpos.x - others.x( jj );
     const double dy = newpos.y - others.y( jj );
     const double dz = newpos.z - others.z( jj );
     result[jj] = (dx*dx + dy*dy + dz*dz);
  }

}

//Compute distance between two coordinates.
double non_periodic_region::calculate_distance_squared(const coordinate & pos1, const coordinate & pos2) const
{
  const double dx = pos1.x - pos2.x;
  const double dy = pos1.y - pos2.y;
  const double dz = pos1.z - pos2.z;
  return (dx*dx + dy*dy + dz*dz);
}

// Can not wrap particle so just return is_inside(pos, radius)
//
// \return is_inside(pos, radius)
bool non_periodic_region::do_new_position_wrap(coordinate & pos, double radius) const
{
  return this->is_inside( pos, radius );
}


} // namespace geometry

BOOST_CLASS_EXPORT_IMPLEMENT( geometry::non_periodic_region );