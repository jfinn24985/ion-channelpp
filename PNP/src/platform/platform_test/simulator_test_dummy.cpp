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

#include "platform/platform_test/simulator_test_dummy.hpp"
#include "core/input_delegater.hpp"
#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"
#include "core/input_document.hpp"

// Manual includes
#include "particle/ensemble.hpp"
namespace platform {

//Compute the distances between given position and existing positions.
//
//\pre rij.size <= ens.size
//\post rij[0:startindex] === 0
//\post rij[ens.size:] undefined
void simulator_test_dummy::do_compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, simulator::array_type & rij, std::size_t endindex, std::size_t startindex) const
{
  const geometry::coordinate newpos(position);
  for (std::size_t j = startindex; j != endindex; ++j)
  {
     // --------------
     // Calculate r_ij
     const double dx = newpos.x - coords.x( j );
     const double dy = newpos.y - coords.y( j );
     const double dz = newpos.z - coords.z( j );
     rij[j] = std::sqrt(dx*dx + dy*dy + dz*dz);
  }

}

simulator_test_dummy::~simulator_test_dummy()
{}

// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > simulator_test_dummy::range_x() const
{
  return std::pair< double, double >( 0.0, 1.0 );
}

// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > simulator_test_dummy::range_y() const
{
  return std::pair< double, double >( 0.0, 1.0 );
}
// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > simulator_test_dummy::range_z() const
{
  return std::pair< double, double >( 0.0, 1.0 );
}

} // namespace platform
