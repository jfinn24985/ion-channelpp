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

#include "geometry/cubic_grid.hpp"
#include "utility/random.hpp"

namespace geometry {

// Initialise the cube grids with the given lengths, inter-node 
// spacing and nodes per dimension.
//
// This leaves a minimum of 'spacing' distance between any grid
// point and the cube edge. There will be an even number of grid
// points in each direction.
//
// \param space : the inter-node spacing
// \param per_dim : The number of nodes in each dimension.
//
// * For cube with width/length = space * per_dim

cubic_grid::cubic_grid(const coordinate & origin, double space, std::size_t per_dim, utility::random_distribution & rgen)
: sequence_()
, origin_( origin )
, steps_( per_dim ) // (*)
, delta_( space )
{
  sequence_.resize( std::pow( per_dim, 3 ) );
  for( size_t ii = 0; ii != sequence_.size(); ++ii ) sequence_[ii] = ii;
  rgen.shuffle( sequence_ );
}


cubic_grid::~cubic_grid() = default;

#if ( defined(DEBUG) and DEBUG>0 )
//Dump grid data to stream.
void cubic_grid::dump(std::ostream & os)
{
  os << "[" << this->origin_ << "](" << this->steps_ << ")(" << this->delta_ << ")";
  os << ":[" << this->size() << "]=" << this->sequence_.back() << "\n";

}
#endif
// Get the next location from the grid with
// indication of iteration ending.
//
// \return size > 0
bool cubic_grid::next(coordinate & pnt) 
{
if (this->sequence_.empty())
{
  return false;
}
else
{
  lldiv_t part;
  part = std::lldiv(std::int64_t(this->sequence_.back()), std::int64_t(this->steps_));
  const std::size_t xi (part.rem);
  part = std::lldiv(part.quot, std::int64_t(this->steps_));
  const std::size_t yi (part.rem);
  const std::size_t zi (part.quot);
  pnt = geometry::coordinate( this->origin_.x + this->delta_ * (xi + 0.5), this->origin_.y + this->delta_ * (yi + 0.5), this->origin_.z + this->delta_ * (zi + 0.5) );
  this->sequence_.pop_back();
  return true;
}

}

// Generate a regular grid of nodes that fits in a cube of the given
// width. The requested grid must have the given minimum inter-node
// spacing and have a minimum of spacing/2 between the nodes and the
// cube walls.
//
// \pre spacing < cube_width

boost::shared_ptr< cubic_grid > cubic_grid::make_grid(double cube_width, double spacing, utility::random_distribution & rgenr)
{
  return make_grid( { 0.0, 0.0, 0.0 }, cube_width, spacing, rgenr );
}

// Generate a regular grid of nodes that fits in a cube of the given
// width. The requested grid must have the given minimum inter-node
// spacing and have a minimum of spacing/2 between the nodes and the
// cube walls.
//
// \pre spacing < cube_width

boost::shared_ptr< cubic_grid > cubic_grid::make_grid(double cube_width, utility::random_distribution & rgenr, std::size_t count)
{
  return make_grid( { 0.0, 0.0, 0.0 }, cube_width, rgenr, count );
}

// Generate a regular grid of nodes that fits in a cube of the given
// width. The requested grid must have the given minimum inter-node
// spacing and have a minimum of spacing/2 between the nodes and the
// cube walls.
//
// \pre spacing < cube_width

boost::shared_ptr< cubic_grid > cubic_grid::make_grid(const coordinate & origin, double cube_width, double spacing, utility::random_distribution & rgenr)
{
  UTILITY_REQUIRE( spacing < cube_width, "No grid can be generated with the given spacing." );
  // calc max count from spacing and cube width.
  std::size_t count( std::max( 1ul, std::size_t(std::floor(cube_width / spacing) ) ) );
  double newspace = cube_width / count;
  return boost::shared_ptr< cubic_grid >( new cubic_grid( origin, newspace, count, rgenr ) );

}

// Generate a regular grid of nodes that fits in a cube of the given
// width. The requested grid must have the given minimum inter-node
// spacing and have a minimum of spacing/2 between the nodes and the
// cube walls.
//
// \pre spacing < cube_width

boost::shared_ptr< cubic_grid > cubic_grid::make_grid(const coordinate & origin, double cube_width, utility::random_distribution & rgenr, std::size_t count)
{
  std::size_t ndim( 1 ); // per nodes in each dim
  double delta = cube_width;
  if( count > 1 )
  {
    ndim = std::size_t( std::floor( std::cbrt( count ) ) );
    while( std::pow( ndim, 3 ) < count )
    {
      // (*)handle possible off-by-one case that may come from
      // converting to floating point and back when count is
      // a cube of an integer.
      ++ndim;
    }
    if ( std::pow( ndim - 1, 3 ) > count )
    {
      --ndim;
      UTILITY_CHECK( std::pow( ndim - 1, 3 ) < count, "Calculation of nodes-per-dimension failed." );
    }
    delta = cube_width / double( ndim );
  }
  return boost::shared_ptr< cubic_grid >( new cubic_grid( origin, delta, ndim, rgenr ) );

}


} // namespace geometry

BOOST_CLASS_EXPORT_IMPLEMENT(geometry::cubic_grid);