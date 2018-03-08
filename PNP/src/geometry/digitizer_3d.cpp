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

#include "geometry/digitizer_3d.hpp"
#include "utility/archive.hpp"


namespace geometry {

// Create digitizer.
//
// NOTE: spacing <= 0.0 is undefined.
digitizer_3d::digitizer_3d(const coordinate & small, const coordinate & big, double spacing)
: xaxis_( std::min( small.x, big.x ), std::max( small.x, big.x ), spacing )
, yaxis_( std::min( small.y, big.y ), std::max( small.y, big.y ), spacing )
, zaxis_( std::min( small.z, big.z ), std::max( small.z, big.z ), spacing )
, spacing_( spacing )
, yskip_( xaxis_.size() )
, zskip_( yaxis_.size() * yskip_ )
, size_( zaxis_.size() * zskip_ )
{
  UTILITY_REQUIRE( spacing > 0.0, "Cannot define digitizer with zero or negative spacing." );
}

// For serialization.
digitizer_3d::digitizer_3d()
: xaxis_()
, yaxis_()
, zaxis_()
, spacing_()
, yskip_()
, zskip_()
, size_()
{}


// Convert a coordinate inside the bounding box into a
// linear index.
//
// \pre xaxis.in_range(pos.x) and yaxis.in_range(pos.y) and zaxis.in_range(pos.z)
// \post result < size
std::size_t digitizer_3d::convert(const coordinate & pos) const
{
  const std::size_t xidx = this->xaxis_.convert( pos.x );
  const std::size_t yidx = this->yaxis_.convert( pos.y );
  const std::size_t zidx = this->zaxis_.convert( pos.z );
  return zidx * this->zskip_ + yidx * this->yskip_ + xidx;
}
// Convert 3d bin index into x, y and z axis bin indices
void digitizer_3d::bin_to_index(std::size_t bin, std::size_t & xidx, std::size_t & yidx, std::size_t & zidx) const
{
  lldiv_t part;
  part = std::lldiv(std::int64_t(bin), std::int64_t(this->zskip_));
  zidx = std::size_t(part.quot);
  part = std::lldiv(part.rem, std::int64_t(this->yskip_));
  yidx = std::size_t(part.quot);
  xidx = std::size_t(part.rem);

}

// Calculate corners of the sampling cube at given index.
//
// \pre idx < size
// \post all points inside bounding box defined by ( x_axis.mininum, y_axis.minimum, z_axis.minimum ) to 
//  ( x_axis.maxinum, y_axis.maximum, z_axis.maximum )
void digitizer_3d::corners(std::size_t idx, std::array< coordinate, 8 > & points)
{
  UTILITY_REQUIRE( idx < size_, "Index out of range." );
  ldiv_t part;
  part = std::div( (long int)idx, (long int)this->zskip_ );
  const std::size_t zidx { (std::size_t)part.quot };
  part = std::div( part.rem, (long int)this->yskip_ );
  const std::size_t yidx { (std::size_t)part.quot };
  const std::size_t xidx { (std::size_t)part.rem };
  
  // llh
  points[ 0 ] = geometry::coordinate{ xaxis_.minimum() + xidx * spacing_, yaxis_.minimum() + yidx * spacing_, zaxis_.minimum() + zidx * spacing_ };
  points[ 1 ] = geometry::coordinate{ points[ 0 ].x + spacing_, points[ 0 ].y, points[ 0 ].z };
  points[ 2 ] = geometry::coordinate{ points[ 0 ].x, points[ 0 ].y + spacing_, points[ 0 ].z };
  points[ 3 ] = geometry::coordinate{ points[ 0 ].x + spacing_, points[ 0 ].y + spacing_, points[ 0 ].z };
  points[ 4 ] = geometry::coordinate{ points[ 0 ].x, points[ 0 ].y, points[ 0 ].z + spacing_ };
  points[ 5 ] = geometry::coordinate{ points[ 0 ].x + spacing_, points[ 0 ].y, points[ 0 ].z + spacing_ };
  points[ 6 ] = geometry::coordinate{ points[ 0 ].x, points[ 0 ].y + spacing_, points[ 0 ].z + spacing_ };
  points[ 7 ] = geometry::coordinate{ points[ 0 ].x + spacing_, points[ 0 ].y + spacing_, points[ 0 ].z + spacing_ };

}


} // namespace geometry
