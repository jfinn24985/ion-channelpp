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

#include "geometry/open_split_cylinder_region.hpp"
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
// =
namespace geometry {

open_split_cylinder_region::~open_split_cylinder_region() = default;
// Increase or decrease the system dimensions to match the new
// volume. Cylinders maintain ratio of half_length to radius.
//
// * NOTE : This is expected to be used only during system
// initialization, ie before checking particle positions or
// asking for new particle positions. Whether any check or
// particle postion calculated before calling this method is
// still valid is undefined.

void open_split_cylinder_region::do_change_volume(double vol, double rad)
{
  // **This region ignores rad argument**
  // Maintain halflength/radius ratio
  //
  //  vol = 2 * HL * pi * R * R
  //
  //  if C = HL/R then HL = C.R
  //
  //  vol = 2 * pi * C * R**3
  //
  const double ratio { this->half_length() / this->radius() };
  
  this->set_radius( std::cbrt( vol / (2 * ratio * core::constants::pi() ) ) );
  
  this->set_half_length( vol / ( 2 * core::constants::pi() * std::pow( this->radius(), 2 ) ) );

}

// True if there is any position in the region where a spherical
// particle of the given size can be completely included, this
// ignores the length.
bool open_split_cylinder_region::do_fits(double radius) const
{
  return true;
}

// Calculate if a sphere centred at the given point and with
// the given radius is within the region. This class ignores the 
// radius when considering the z direction.
bool open_split_cylinder_region::do_is_inside(const coordinate & pos, double radius) const
{
  if( ( this->offset_ ) > std::abs( pos.z ) )
  {
    // sphere centre between two parts.
    return false;
  }
  if( ( this->offset_ + this->half_length() ) < std::abs( pos.z ) )
  {
    // sphere beyond end.
    return false;
  }
  if( std::pow( this->radius(), 2 ) < ( pos.x * pos.x + pos.y * pos.y ) )
  {
    // sphere outside wall
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

boost::shared_ptr< grid_generator > open_split_cylinder_region::do_make_gridder(utility::random_distribution & rgenr, std::size_t count) const
{
  boost::shared_ptr< grid_generator > result;
  result.reset( new split_tube_grid( this->offset_, 2*this->half_length(), this->radius(), count, rgenr ) );
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

boost::shared_ptr< grid_generator > open_split_cylinder_region::do_make_gridder(double spacing, utility::random_distribution & rgenr) const
{
  boost::shared_ptr< grid_generator > result;
  result.reset( new split_tube_grid( this->offset_, 2*this->half_length(), this->radius(), spacing, rgenr ) );
  return result;

}

// Calculate new random position for particle of the given radius to
// be inside the region. This class ignores the radius when calculating
// the new z coordinate.
//
// \pre fits(radius)
// \post is_inside(pos, radius)
coordinate open_split_cylinder_region::do_new_position(utility::random_distribution & rgnr, double radius) const
{
  // A random position within either cylinder part
  geometry::coordinate pos( 0.0, 0.0, 0.0 );
  // volume is linear in square of radius
  const double r( std::sqrt( rgnr.uniform( std::pow( this->radius(), 2 ) ) ) );
  const double phi( rgnr.uniform( 0.0, core::constants::pi() * 2 ) );
  pos.x = r * std::cos( phi );
  pos.y = r * std::sin( phi );
  pos.z = rgnr.uniform( -this->half_length(), this->half_length() );
  pos.z = ( pos.z < 0.0 ? pos.z - this->offset_ : pos.z + this->offset_ );
  return pos;
  
  

}

// Update wr[ix] section with information from the derived class.
void open_split_cylinder_region::do_write_document(core::input_document & wr, std::size_t ix) const
{
  const std::string half_length_label { "half-length" };
  const std::string radius_label { "radius" };
  const std::string offset_label { "offset" };
  wr[ix].add_entry( core::strngs::fstype(), type_label() );
  wr[ix].add_entry( half_length_label, this->half_length() );
  wr[ix].add_entry( radius_label, this->radius() );
  wr[ix].add_entry( offset_label, this->offset() );
  

}

// Factory method for creating cube_region objects from
// input data.
boost::shared_ptr< base_region > open_split_cylinder_region::region_factory(std::string label, const std::vector< core::input_parameter_memo > & params)
{
  // needed information
  // label : from label arg
  // radius : from param arg
  // half_length : from param arg
  // offset : from param arg
  double half_length {};
  double radius {};
  double offset {};
  size_t mask{ 0ul };
  
  const std::string half_length_label { "half-length" };
  const std::string radius_label { "radius" };
  const std::string offset_label { "offset" };
  for( auto const& param : params )
  {
    if( param.name() == half_length_label )
    {
      half_length = param.get_float( "Split cylinder size", core::strngs::fsregn(), true, false );
      mask |= 1ul;
    }
    else if( param.name() == radius_label )
    {
      radius = param.get_float( "Split cylinder size", core::strngs::fsregn(), true, false );
      mask |= 2ul;
    }
    else if( param.name() == offset_label )
    {
      offset = param.get_float( "Split cylinder split", core::strngs::fsregn(), true, false );
      mask |= 4ul;
    }
  }
  {
    std::string missing_params;
    // check for required parameters.
    if( ( mask & 1ul ) == 0ul ) missing_params = half_length_label;
    if( ( mask & 2ul ) == 0ul ) missing_params += (missing_params.empty() ? "": " ") + radius_label;
    if( ( mask & 4ul ) == 0ul ) missing_params += (missing_params.empty() ? "": " ") + offset_label;
    if( not missing_params.empty() )
    {
      UTILITY_CHECK( params.back().name() == core::strngs::fsend(), "Missing guard entry in vector." );
      throw core::input_error::missing_parameters_error( "Split cylinder", core::strngs::fsregn(), missing_params, params.back().filename(), params.back().line_number() );
    }
  }
  // build region object.
  boost::shared_ptr< geometry::base_region > current;
  current.reset( new open_split_cylinder_region( label, radius, half_length, offset ) );
  return current;

}

// Create and add a region_definition to the meta object.
void open_split_cylinder_region::add_definition(region_meta & meta)
{
  std::string desc( "A cylindrical region centred on the origin with linear axis in z direction. All walls are considered soft. Cylinder is cut in the centre and the two parts displaced \"offset\" from the origin along the z-axis." );
  std::unique_ptr< region_definition > cylinder_defn( new region_definition( type_label(), desc, &open_split_cylinder_region::region_factory ) );
  cylinder_defn->add_definition( { "radius", "distance in Angstrom", ">0", "required", "The radius of the cylinder in the XY plane." } );
  cylinder_defn->add_definition( { "half-length", "distance in Angstrom", ">0", "required", "Half the length of the cylinder. The cylinder is centred on the origin so: offset <= |z| <= half-length + offset." } );
  cylinder_defn->add_definition( { "offset", "distance in Angstrom", ">0", "required", "Half the length of the gap between the two cylinder halves. The cylinder is centred on the origin so: offset <= |z| <= half-length + offset." } );
  meta.add_definition( cylinder_defn );

}


} // namespace geometry

BOOST_CLASS_EXPORT_IMPLEMENT( geometry::open_split_cylinder_region );