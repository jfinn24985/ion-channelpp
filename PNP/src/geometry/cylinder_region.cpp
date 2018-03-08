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

#include "geometry/cylinder_region.hpp"
#include "geometry/grid.hpp"
#include "utility/random.hpp"
#include "core/input_document.hpp"
#include "geometry/base_region.hpp"
#include "core/input_parameter_memo.hpp"
#include "geometry/region_meta.hpp"

// Manual includes
#include "core/constants.hpp"
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "geometry/tubular_grid.hpp"
// -
namespace geometry {

cylinder_region::~cylinder_region() = default;
// Increase or decrease the system dimensions to match the new
// volume. Cylinders maintain ratio of half_length to radius.
//
// * NOTE : This is expected to be used only during system
// initialization, ie before checking particle positions or
// asking for new particle positions. Whether any check or
// particle postion calculated before calling this method is
// still valid is undefined.

void cylinder_region::do_change_volume(double vol, double rad)
{
  // Maintain halflength/radius ratio
  // (ignore effect of rad argument).
  //
  //  vol = 2 * (HL - r) * pi * (R - r) * (R - r)
  //
  //  if C = HL/R then HL = C.R
  //
  //  vol = 2 * pi * (CR - r) * (R - r) * (R - r)
  //
  //  To make tractable (CR - r) --> C(R -r)
  //
  //  vol = 2C * pi * (R - r) * (R - r) * (R - r)
  const double ratio { this->half_length() / this->radius() };
  
  const double newradius { std::cbrt( vol / (2 * ratio * core::constants::pi() ) ) };
  
  this->radius_ = newradius + rad;
  
  const double newlength = { vol / ( 2 * core::constants::pi() * std::pow( newradius, 2 ) ) };
  
  this->half_length_ = newlength + rad;

}

// Calculate a bounding box for the valid positions of
// spheres of the given radius within the region. The
// bounding box is defined by its most negative
// (small_corner) and most positive (big_corner).
//
// \pre fits(radius)
// \post big.(x|y|z) - small.(x|y|z) >= 0
void cylinder_region::do_extent(coordinate & small_corner, coordinate & big_corner, double radius) const
{
  big_corner.x = this->radius() - radius;
  big_corner.y = big_corner.x;
  big_corner.z = this->half_length() - radius;
  small_corner.x = -big_corner.x;
  small_corner.y = -big_corner.y;
  small_corner.z = -big_corner.z;

}

// True if there is any position in the region where a spherical
// particle of the given size can be completely included.
bool cylinder_region::do_fits(double radius) const
{
  return this->half_length_ >= radius and this->radius_ >= radius;

}

// Calculate if a sphere centred at the given point and with
// the given radius is within the region.
bool cylinder_region::do_is_inside(const coordinate & pos, double radius) const
{
  // CAN ASSUME fits(radius) == true
  if( ( this->half_length_ - radius ) < std::abs( pos.z ) )
  {
    // sphere too close to end
    return false;
  }
  if( std::pow( this->radius_ - radius, 2 ) < ( pos.x * pos.x + pos.y * pos.y ) )
  {
    // sphere too close to wall
    return false;
  }
  return true;
  

}

// Create a gridder object for this system. The gridder is
// guarranteed to have the minimum given number of points.
// The gridder provides the positions in a random sequence.
//
// (if count = 0 it is incremented to 1).
//
// As the grid is highly regular, the number of nodes will be an
// integral function of the number of points in each dimension.
// For a cube will always have the same number of nodes in
// each dimension and so the total number will be the cube of a
// whole number. The actual number of nodes will therefore be
// the smallest solution to this node function that is larger
// than count.

boost::shared_ptr< grid_generator > cylinder_region::do_make_gridder(utility::random_distribution & rgenr, std::size_t count) const
{
  boost::shared_ptr< grid_generator > result;
  result.reset( new tubular_grid( 2*this->half_length(), this->radius(), count, rgenr ) );
  return result;

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

boost::shared_ptr< grid_generator > cylinder_region::do_make_gridder(double spacing, utility::random_distribution & rgenr) const
{
  boost::shared_ptr< grid_generator > result;
  result.reset( new tubular_grid( 2*this->half_length(), this->radius(), spacing, rgenr ) );
  return result;

}

// Calculate new random position for particle of the given radius to
// be fully inside the region.
//
// \pre fits(radius)
// \post is_inside(pos, radius)
coordinate cylinder_region::do_new_position(utility::random_distribution & rgnr, double radius) const
{
  // A random position within cylinder
  geometry::coordinate pos( 0.0, 0.0, 0.0 );
  if( radius < this->radius_ )
  {
    // Volume is linear in square of radius
    const double r( std::sqrt( rgnr.uniform( std::pow( this->radius_ - radius, 2 ) ) ) );
    const double phi( rgnr.uniform( 0.0, core::constants::pi() * 2 ) );
    pos.x = r * std::cos( phi );
    pos.y = r * std::sin( phi );
  }
  // else x = 0, y = 0
  
  if( radius < this->half_length_ )
  {
    const double reduced_hl( this->half_length_ - radius );
    pos.z = rgnr.uniform( -reduced_hl, reduced_hl );
  }
  // else z = 0
  return pos;
  

}

// Update wr[ix] section with information from the derived class.
void cylinder_region::do_write_document(core::input_document & wr, std::size_t ix) const
{
  const std::string half_length_label { "half-length" };
  const std::string radius_label { "radius" };
  wr[ix].add_entry( core::strngs::fstype(), type_label() );
  wr[ix].add_entry( half_length_label, this->half_length_ );
  wr[ix].add_entry( radius_label, this->radius_ );
  

}

// volume = pi * rad * rad * len
//
// * NOTE: len is cylinder length **not** half length
double cylinder_region::compute_volume(double rad, double len)
{
  return core::constants::pi() * rad * rad * len;
}

// Factory method for creating cube_region objects from
// input data.
boost::shared_ptr< base_region > cylinder_region::region_factory(std::string label, const std::vector< core::input_parameter_memo > & params)
{
  // needed information
  // label : from label arg
  // radius : from param arg
  // half_length : from param arg
  double half_length {};
  double radius {};
  size_t mask{ 0ul };
  const std::string half_length_label { "half-length" };
  const std::string radius_label { "radius" };
  for( auto const& param : params )
  {
    if( param.name() == half_length_label )
    {
      half_length = param.get_float( "Cylinder size", core::strngs::fsregn(), true, false );
      mask |= 1ul;
    }
    else if( param.name() == radius_label )
    {
      radius = param.get_float( "Cylinder size", core::strngs::fsregn(), true, false );
      mask |= 2ul;
    }
  }
  {
    std::string missing_params;
    // check for required parameters.
    if( ( mask & 1ul ) == 0 ) missing_params = half_length_label;
    if( ( mask & 2ul ) == 0 ) missing_params += (missing_params.empty() ? "": " ") + radius_label;
    if( not missing_params.empty() )
    {
      UTILITY_CHECK( params.back().name() == core::strngs::fsend(), "Missing guard entry in vector." );
      throw core::input_error::missing_parameters_error( "Cylinder", core::strngs::fsregn(), missing_params, params.back().filename(), params.back().line_number() );
    }
  }
  // build region object.
  boost::shared_ptr< geometry::base_region > current;
  current.reset( new cylinder_region( label, radius, half_length ) );
  return current;

}

// Create and add a region_definition to the meta object.
void cylinder_region::add_definition(region_meta & meta)
{
  std::string desc( "Cylindrical region centred on the origin with linear axis in z direction. All sides are hard edges when considering valid positions." );
  std::unique_ptr< region_definition > cylinder_defn( new region_definition( type_label(), desc, &cylinder_region::region_factory ) );
  cylinder_defn->add_definition( { "radius", "distance in Angstrom", ">0", "required", "The radius of the cylinder in the XY plane." } );
  cylinder_defn->add_definition( { "half-length", "distance in Angstrom", ">0", "required", "Half the length of the cylinder. The cylinder is centred on the origin so |z| <= half-length." } );
  meta.add_definition( cylinder_defn );

}


} // namespace geometry

BOOST_CLASS_EXPORT_IMPLEMENT( geometry::cylinder_region );