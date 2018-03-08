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

#include "geometry/cube_region.hpp"
#include "geometry/cubic_grid.hpp"
#include "geometry/grid.hpp"
#include "utility/random.hpp"
#include "core/input_document.hpp"
#include "geometry/base_region.hpp"
#include "core/input_parameter_memo.hpp"
#include "geometry/region_meta.hpp"

// manual includes
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "utility/fuzzy_equals.hpp"
// -
namespace geometry {

cube_region::~cube_region() = default;
// Adjust cube length to accomodate volume.
//
// if is_open()
//  len = cuberoot( vol )
// else
//  len + 2 rad = cuberoot( vol )
//
// * NOTE: can never give: not fits(rad)
void cube_region::do_change_volume(double vol, double rad)
{
  if( this->is_open() ) 
  {
   // we can ignore radius
   this->length_ = std::cbrt( vol );
  }
  else
  {
    // (len - 2r)**3 = volume
    this->length_ = std::cbrt( vol ) + 2 * rad;
  }

}

// Calculate a bounding box for the valid positions of
// spheres of the given radius within the region. The
// bounding box is defined by its most negative
// (small_corner) and most positive (big_corner).
//
// \pre fits(radius)
// \post big.(x|y|z) - small.(x|y|z) >= 0
void cube_region::do_extent(coordinate & small_corner, coordinate & big_corner, double radius) const
{
  if (this->is_open())
  {
    // ignore radius
    radius = 0.0;
  }
  const double length { this->length_ - (2*radius) };
  small_corner.x = this->origin_.x + radius;
  small_corner.y = this->origin_.y + radius;
  small_corner.z = this->origin_.z + radius;
  big_corner.x = small_corner.x + length;
  big_corner.y = small_corner.y + length;
  big_corner.z = small_corner.z + length;

}

// True if there is any position in the region where a spherical
// particle of the given size can be in a valid position. 
//
// * NOTE: While a particle of any size can theoretically exist
// anywhere in a periodic cube, particles that are larger than the cube
// would occlude themselves and are therefore said not to fit.
bool cube_region::do_fits(double radius) const
{
  return this->is_open_ or this->length_ >= (2 * radius);
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
bool cube_region::do_is_inside(const coordinate & pos, double radius) const
{
  double len( this->length_ );
  geometry::coordinate orig( this->origin_ );
  auto inrange = [](double min, double width, double val)->bool
  {
    return !(val < min) and !((min + width) < val);
  };
  if( not this->is_open_ )
  {
    UTILITY_CHECK( (2 * radius) <= len, "Test if particle could fit in cube should have been performed." );
    len -= (2 * radius);
    orig.x += radius;
    orig.y += radius;
    orig.z += radius;
  }
  return inrange( orig.x, len, pos.x )
     and inrange( orig.y, len, pos.y )
     and inrange( orig.z, len, pos.z );

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

boost::shared_ptr< grid_generator > cube_region::do_make_gridder(double spacing, utility::random_distribution & rgenr) const
{
  // To avoid grid points from falling outside the confines of the
  // cube we reduce length by a small amount.
  return geometry::cubic_grid::make_grid( this->origin_, this->length_ * 0.9999999, spacing, rgenr );

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

boost::shared_ptr< grid_generator > cube_region::do_make_gridder(utility::random_distribution & rgenr, std::size_t count) const
{
  // To avoid grid points from falling outside the confines of the
  // cube we reduce length by a small amount.
  return geometry::cubic_grid::make_grid( this->origin_, this->length_ * 0.9999999, rgenr, count );
}

// Calculate new random position for particle. IGNORES RADIUS
//
// * NOTE: For this periodic system all positions are valid and
// so any new position should be anywhere in the cube. Limiting
// particle to be all inside would not be the desired behavior. 

coordinate cube_region::do_new_position(utility::random_distribution & rgnr, double radius) const
{
  double len( this->length_ );
  geometry::coordinate orig( this->origin_ );
  if( not this->is_open_ )
  {
    len -= (2 * radius);
    orig.x += radius;
    orig.y += radius;
    orig.z += radius;
  }
  return geometry::coordinate( orig.x + rgnr.uniform( len ), orig.y + rgnr.uniform( len ), orig.z + rgnr.uniform( len ) );

}

// Calculate volume accessible to centre-points of spherical particles
// with the given radius. For this periodic system this is always 
// equivalent to volume().
//
// NOTE: can assume fits(radius)
double cube_region::do_volume(double radius) const
{
  double len( this->is_open_ ? this->length_ : this->length_ - 2 * radius );
  return std::pow( len, 3 );

}

// Update wr[ix] section with information from the derived class.
void cube_region::do_write_document(core::input_document & wr, std::size_t ix) const
{
  const std::string width_label { "width" };
  const std::string origin_label { "origin" };
  const std::string isopen_label { "open" };
  wr[ix].add_entry( core::strngs::fstype(), type_label() );
  wr[ix].add_entry( width_label, this->length_ );
  wr[ix].add_entry( origin_label, this->origin_ );
  wr[ix].add_entry( isopen_label, this->is_open_ );
  

}

// Factory method for creating cube_region objects from
// input data.
boost::shared_ptr< base_region > cube_region::region_factory(std::string label, const std::vector< core::input_parameter_memo > & params)
{
  // needed information
  // FROM label PARAMETER
  // label : from label arg
  // FROM params PARAMETER
  // origin : from param arg
  // width : from param arg
  // open : from param arg or default:true
  std::size_t mask{ 0ul };
  double width {};
  geometry::coordinate origin {};
  bool isopen { true }; // default is true
  
  const std::string width_label { "width" };
  const std::string origin_label { "origin" };
  const std::string isopen_label { "open" };
  // check for required parameters.
  for( auto const& param : params )
  {
    if( param.name() == width_label )
    {
      width = param.get_float( "Cube size", core::strngs::fsregn(), true, false );
      mask |= 1ul;
    }
    else if( param.name() == origin_label )
    {
      geometry::coordinate::coordinate_input( param, "Cube location", core::strngs::fsregn(), origin );
      mask |= 2ul;
    }
    else if( param.name() == isopen_label )
    {
      isopen = param.get_bool( "Cube wall permeability", core::strngs::fsregn(), true, true );
    }
  }
  std::string miss_params;
  if( ( mask & 1ul ) == 0ul ) miss_params = width_label;
  if( ( mask & 2ul ) == 0ul ) miss_params += ( miss_params.empty() ? "" : " " ) + origin_label;
  
  if( not miss_params.empty() )
  {
    UTILITY_CHECK( params.back().name() == core::strngs::fsend(), "Missing guard entry in vector." );
    throw core::input_error::missing_parameters_error( "Cube", core::strngs::fsregn(), miss_params, params.back().filename(), params.back().line_number() );
  }
  
  // build region object.
  boost::shared_ptr< geometry::base_region > current;
  current.reset( new cube_region( label, width, origin, isopen ) );
  return current;

}

// Create and add a region_definition to the meta object.
void cube_region::add_definition(region_meta & meta)
{
  std::string desc( "A non-periodic cube region. The cube surfaces can be permeable (open=true) or impermeable (open=false) to simulation particles." );
  std::unique_ptr< region_definition > cube_defn( new region_definition( type_label(), desc, &cube_region::region_factory ) );
  cube_defn->add_definition( { "origin", "x,y,z coordinates in Angstrom", "", "required", "The coordinate of the lowest (most negative) corner of the cubic region" } );
  cube_defn->add_definition( { "width", "number in Angstrom", ">0", "required", "The width in each dimension of the cubic region"  } );
  cube_defn->add_definition( { "open", "boolean", "true|false", "true", "Whether the cube walls are open or closed to simulation particles." } );
  meta.add_definition( cube_defn );

}


} // namespace geometry

BOOST_CLASS_EXPORT_IMPLEMENT( geometry::cube_region );