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

#include "geometry/base_region.hpp"
#include "utility/archive.hpp"

#include "geometry/grid.hpp"
#include "utility/random.hpp"
#include "core/input_document.hpp"

// manual include
// for pi()
#include "core/constants.hpp"
#include "core/strngs.hpp"
//-
namespace geometry {

// Calculate a bounding box for the valid positions of spheres
// of the given radius within the region. The bounding box is
// defined by its most negative (small_corner) and most positive
// (big_corner).
//
// \pre fits(radius) 
// \post big.(x|y|z) - small.(x|y|z) >= 0

void base_region::extent(coordinate & small_corner, coordinate & big_corner, double radius) const
{
  UTILITY_REQUIRE( this->fits( radius ), "Can not calculate extent for sphere that does not fit the region." );
  return this->do_extent( small_corner, big_corner, radius );
}

// Calculate new random position for particle of the given radius to
// be in a valid position in the region.  The position distribution
// function should be equivolumetric.
//
// \pre fits(radius)
// \post is_inside(result, radius)
coordinate base_region::new_position(utility::random_distribution & rgnr, double radius) const
{
  UTILITY_REQUIRE( this->fits( radius ), "Particle can not fit in region." );
  return this->do_new_position( rgnr, radius );
}

// Calculate new random position in a sphere centred on pos and
// assign to pos. The particle is displaced a uniform linear
// distance in a random direction. This means that it is not an
// equivolume distribution (unlike new_position)
//
// * NOTE: for periodic systems the position should be 
// automatically wrapped to be inside the system and
// so should always return true.
//
// \pre is_inside(pos, radius) and fits(radius)
// \return is_inside(pos, radius)

bool base_region::new_position_offset(utility::random_distribution & rgnr, coordinate & pos, double offset, double radius) const
{
  UTILITY_REQUIRE( this->fits( radius ), "Particle can not fit in region." );
  UTILITY_REQUIRE( this->is_inside( pos, radius ), "Particle must start inside region." );
  this->do_offset( rgnr, pos, offset );
  return this->do_new_position_wrap( pos, radius );
}

// Calculate new random position in a sphere centred on pos and
// assign to pos. The particle is displaced a uniform linear
// distance in a random direction. This means that it is not an
// equivolume distribution (unlike new_position)
//
// pos.x += r cos(theta) sin(phi)
//
// pos.y += r sin(theta) sin(phi)
//
// pos.z += r cos(phi)

void base_region::do_offset(utility::random_distribution & rgnr, coordinate & pos, double offset)
{
  const double r( rgnr.uniform( 0.0, offset ) );
  const double theta( rgnr.uniform( 0.0, 2 * core::constants::pi() ) );
  const double phi( rgnr.uniform( 0.0, 2 * core::constants::pi() ) );
  pos.x	+= r * std::cos(theta) * std::sin(phi);
  pos.y += r * std::sin(theta) * std::sin(phi);
  pos.z += r * std::cos(phi);

}

// Add an input file section to wr.
//
// Output of this method is something like
//
// region
// name [label]
// <call do_write_input_section>
// end

void base_region::write_document(core::input_document & wr) const 
{
  std::size_t ix = wr.add_section( core::strngs::fsregn() );
  wr[ix].add_entry( core::strngs::fsname(), this->label() );
  this->do_write_document( wr, ix );
  

}


} // namespace geometry

//#include <boost/serialization/export.hpp>
//BOOST_CLASS_EXPORT_GUID(geometry::base_region, "geometry::base_region");