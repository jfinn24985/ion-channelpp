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

#include "geometry/periodic_cube_region.hpp"
#include "geometry/cubic_grid.hpp"
#include "core/input_parameter_memo.hpp"
#include "geometry/coordinate_set.hpp"
#include "geometry/grid.hpp"
#include "utility/random.hpp"
#include "core/input_document.hpp"
#include "geometry/region_meta.hpp"

// manual includes
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
// -
namespace geometry {

periodic_cube_region::~periodic_cube_region() = default;
//Compute the distances between any new position and existing positions.
//
//\post result.size == end_index
//\post result[0:start_index] == 0.0

void periodic_cube_region::do_calculate_distances_sq(const coordinate & pos, const coordinate_set & others, std::vector< double >& result, std::size_t startindex, std::size_t endindex) const
{
  const geometry::coordinate newpos(pos);
  const double box = this->length_;
  const double hbox = box/2;
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
     double dx = newpos.x - others.x( jj );
     double dy = newpos.y - others.y( jj );
     double dz = newpos.z - others.z( jj );
     dx += (dx > hbox ? -box : ( dx < -hbox ? box : 0.0));
     dy += (dy > hbox ? -box : ( dy < -hbox ? box : 0.0));
     dz += (dz > hbox ? -box : ( dz < -hbox ? box : 0.0));
     result[jj] = (dx*dx + dy*dy + dz*dz);
  }

}

// Compute square of distance between two coordinates in 
// a periodic box.
double periodic_cube_region::calculate_distance_squared(const coordinate & pos1, const coordinate & pos2) const 
{
  const double box (this->length_);
  const double hbox (box/2);
  double dx = pos1.x - pos2.x;
  double dy = pos1.y - pos2.y;
  double dz = pos1.z - pos2.z;
  dx = (dx > hbox ? dx - box : (dx < -hbox ? dx + box : dx));
  dy = (dy > hbox ? dy - box : (dy < -hbox ? dy + box : dy));
  dz = (dz > hbox ? dz - box : (dz < -hbox ? dz + box : dz));
  return dx*dx + dy*dy + dz*dz;

}

// Adjust cube length to accomodate volume.
//
// len = cuberoot( vol )
//
// * NOTE: _can_ give: not fits(rad) after volume change
void periodic_cube_region::do_change_volume(double vol, double rad)
{
  const double oldlen( this->length_ );
  const double oldvol( this->volume_ );
  this->length_ = std::cbrt( vol );
  this->volume_ = vol;
  // We should be able to fit radius after changing the volume.
  if( rad == 0.0 )
  {
    return;
  }
  const bool fits_radius( this->fits( rad ) );
  if( not fits_radius )
  {
    // reset length and volume.
    this->length_ = oldlen;
    this->volume_ = oldvol;
    UTILITY_REQUIRE( fits_radius, "Object can not fit in volume after volume change." );
  }
  

}

// Calculate a bounding box for the valid positions of
// spheres of the given radius within the region. The
// bounding box is defined by its most negative
// (small_corner) and most positive (big_corner).
//
// \pre fits(radius)
// \post big.(x|y|z) - small.(x|y|z) >= 0
void periodic_cube_region::do_extent(coordinate & small_corner, coordinate & big_corner, double radius) const
{
  const double length { this->length_ };
  small_corner.x = 0.0;
  small_corner.y = 0.0;
  small_corner.z = 0.0;
  big_corner.x = length;
  big_corner.y = length;
  big_corner.z = length;

}

// True if there is any position in the region where a spherical
// particle of the given size can be in a valid position. 
//
// * NOTE: While a particle of any size can theoretically exist
// anywhere in a periodic cube, particles that are larger than the cube
// would occlude themselves and are therefore said not to fit.
bool periodic_cube_region::do_fits(double radius) const
{
  return this->length_ >= (2 * radius);
}

// Check if sphere at the given position and radius is inside
// the cube. As all positions can be mapped inside the
// cube, this method answers the question of whether the
// position is inside the primary cube. 
//
// not fits(r) --> not is_inside(..., r)
//
// * NOTE: Does not consider periodicity but ignores the radius if
// fits(radius) is true.
bool periodic_cube_region::do_is_inside(const coordinate & pos, double radius) const
{
  const double len( this->length_ );
  auto inrange = [len](double val)->bool
  {
    return 0.0 <= val and val <= len;
  }; 
  return len >= radius and inrange( pos.x ) and inrange( pos.y ) and inrange( pos.z );

}

// Create a gridder object for this system. The gridder inter-node
// distance is guarranteed to have the given minimum spacing.
// Every node is also guarranteed to be a minimum of (spacing/2)
// distant from any side.  The gridder provides the positions
// in a random sequence.
//
// The spacing may be increased slightly to more evenly fit the
// particles into the region. The gridder calculates the maximum
// number of nodes that can fit within the region and have the
// given spacing. It then relaxes the grid to evenly fit in the
// region and recalculates the spacing.

boost::shared_ptr< grid_generator > periodic_cube_region::do_make_gridder(double spacing, utility::random_distribution & rgenr) const
{
  // To avoid grid points from falling outside the confines of the
  // cube we reduce length by a small amount.
  return geometry::cubic_grid::make_grid( { 0.0, 0.0, 0.0 }, this->length_ * 0.9999999, spacing, rgenr );
}

// Create a gridder object for this system. The gridder is
// guarranteed to have the minimum given number of points.
// The gridder provides the positions in a random sequence.
//
// As the grid is highly regular, the number of nodes will be an
// integral function of the number of points in each dimension.
// For a cube will always have the same number of nodes in
// each dimension and so the total number will be the cube of a
// whole number. The actual number of nodes will therefore be
// the smallest solution to this node function that is larger
// than count.

boost::shared_ptr< grid_generator > periodic_cube_region::do_make_gridder(utility::random_distribution & rgenr, std::size_t count) const
{
  // To avoid grid points from falling outside the confines of the
  // cube we reduce length by a small amount.
  return geometry::cubic_grid::make_grid( { 0.0, 0.0, 0.0 }, this->length_ * 0.9999999, rgenr, count );
}

// Calculate new random position for particle. IGNORES RADIUS
//
// * NOTE: For this periodic system all positions are valid and
// so any new position should be anywhere in the cube. Limiting
// particle to be all inside would not be the desired behavior. 

coordinate periodic_cube_region::do_new_position(utility::random_distribution & rgnr, double radius) const
{
  return geometry::coordinate( rgnr.uniform( this->length_ ), rgnr.uniform( this->length_ ), rgnr.uniform( this->length_ ) );

}

// Wrap position for particle inside cube. IGNORES RADIUS
//
// * NOTE: For this periodic system all positions are valid and
// so position should be anywhere in the cube. Limiting
// particle to be all inside would not be the desired behavior. 
bool periodic_cube_region::do_new_position_wrap(coordinate & pos, double radius) const
{
  auto wrap = [](double a, double range)->double
  {
    double r = std::fmod( a, range );
    if (r < 0.0) r += range;
    return r; 
  };
  pos.x = wrap( pos.x, this->length_ );
  pos.y = wrap( pos.y, this->length_ );
  pos.z = wrap( pos.z, this->length_ );
  return true;

}

// Update wr[ix] section with information from the derived class.
void periodic_cube_region::do_write_document(core::input_document & wr, std::size_t ix) const
{
  const std::string width_label { "width" };
  wr[ix].add_entry( core::strngs::fstype(), type_label() );
  wr[ix].add_entry( width_label, this->length() );
  

}

// Factory method for creating cube_region objects from
// input data.
boost::shared_ptr< base_region > periodic_cube_region::region_factory(std::string label, const std::vector< core::input_parameter_memo > & params)
{
  // needed information
  // label : from label arg
  // width : from param arg
  double width {};
  bool found_width{ false };
  
  const std::string width_label { "width" };
  
  for( auto const& param : params )
  {
    if( param.name() == width_label )
    {
      width = param.get_float( "Periodic cube size", core::strngs::fsregn(), true, false );
      found_width = true;
    }
  }
  {
    // check for required parameters.
    if( not found_width )
    {
      UTILITY_CHECK( params.back().name() == core::strngs::fsend(), "Missing guard entry in vector." );
      throw core::input_error::missing_parameters_error( "Periodic cube", core::strngs::fsregn(), width_label, params.back().filename(), params.back().line_number() );
    }
  }
  
  // build region object.
  boost::shared_ptr< geometry::base_region > current;
  current.reset( new periodic_cube_region( label, width ) );
  return current;

}

// Create and add a region_definition to the meta object.
void periodic_cube_region::add_definition(region_meta & meta)
{
  std::string desc( "A cube with periodic boundaries with the origin at one corner. Can be used as a system region." );
  std::unique_ptr< region_definition > cube_defn( new region_definition( type_label(), desc, &periodic_cube_region::region_factory ) );
  cube_defn->add_definition( { "width", "distance in Angstrom", ">0", "required", "The width in each dimension of the cubic region" } );
  meta.add_definition( cube_defn );

}


} // namespace geometry

BOOST_CLASS_EXPORT_IMPLEMENT( geometry::periodic_cube_region );