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

#include "geometry/membrane_cylinder_region.hpp"
#include "geometry/grid.hpp"
#include "utility/random.hpp"
#include "core/input_document.hpp"
#include "geometry/base_region.hpp"
#include "core/input_parameter_memo.hpp"
#include "geometry/region_meta.hpp"

// - manual includes
#include "core/constants.hpp"
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "utility/fuzzy_equals.hpp"
#include "geometry/tubular_grid.hpp"
// -
namespace geometry {

membrane_cylinder_region::~membrane_cylinder_region() = default;
// Increase or decrease the system dimensions to match the new
// volume. Cylinders maintain ratio of half_length to radius.
//
// * NOTE : This is expected to be used only during system
// initialization, ie before checking particle positions or
// asking for new particle positions. Whether any check or
// particle postion calculated before calling this method is
// still valid is undefined.

void membrane_cylinder_region::do_change_volume(double vol, double rad)
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

// Calculate a bounding box for the valid positions of
// spheres of the given radius within the region. The
// bounding box is defined by its most negative
// (small_corner) and most positive (big_corner).
//
// \pre fits(radius)
// \post big.(x|y|z) - small.(x|y|z) >= 0
void membrane_cylinder_region::do_extent(coordinate & small_corner, coordinate & big_corner, double radius) const
{
  big_corner.x = this->radius() - radius;
  big_corner.y = big_corner.x;
  big_corner.z = this->channel_half_length() + this->half_length() - radius;
  small_corner.x = -big_corner.x;
  small_corner.y = -big_corner.y;
  small_corner.z = -big_corner.z;

}

// True if there is any position in the region where a spherical
// particle of the given size can be completely included, this
// ignores the length.
bool membrane_cylinder_region::do_fits(double radius) const
{
  return true;
}

// Calculate if a sphere centred at the given point and with
// the given radius is within the region. The checks if the
// sphere is in the cell compartments or the channel.
bool membrane_cylinder_region::do_is_inside(const coordinate & pos, double radius) const
{
  // Absolute Z position
  const double abs_z_pos { std::abs( pos.z ) };
  // Radial position
  const double radial_sq_pos { pos.x * pos.x + pos.y * pos.y };
  // Radius of channel mouth at at the membrane surface (tube radius + arc
  // radius)
  const double mouth_radius { this->channel_radius_ + this->arc_radius_ };
  
  // Outside of membrane
  if( abs_z_pos >= ( this->channel_half_length_ + radius ) )
  {
    // check if outside extreme of compartment in r and z?
    return not( radial_sq_pos > std::pow( this->radius()  - radius, 2 )
                or abs_z_pos > ( this->half_length() + this->channel_half_length_ - radius ) );
  }
  
  // (else inside membrane and) Within channel cylinder inner-radius?
  if( radial_sq_pos <= std::pow( this->channel_radius_ - radius, 2 ) )
  {
    return true;
  }
  
  // Not in vestibule OR Outside maximum vestibule radius means overlap
  if( abs_z_pos <= ( this->channel_half_length_ - this->arc_radius_ )
     or radial_sq_pos > std::pow( mouth_radius, 2 ) )
  {
    return false;
  }
  
  // Outside channel radius but may be in the arc zone. We can
  // calculate the maximum radius allowed for the sphere based
  // on the z-offset (dz) and adjusted arc radius (r) and ddr:
  //    ddr * ddr + dz * dz = r * r
  // where ddr is the radial distance from the arc centrepoint 
  // towards the axis of rotation. The maximum allowed radius
  // is then the radius of the arc centrepoint less ddr.
  const double ddr
  {
     std::sqrt( std::pow( this->arc_radius_ + radius, 2 ) - std::pow( abs_z_pos - ( this->channel_half_length_ - this->arc_radius_ ), 2 ) )
  };
  const double r2maximum 
  {
     std::pow( this->channel_radius_ + this->arc_radius_ - ddr, 2 )
  };
  return ( radial_sq_pos <= r2maximum );
  

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

boost::shared_ptr< grid_generator > membrane_cylinder_region::do_make_gridder(utility::random_distribution & rgenr, std::size_t count) const
{
  boost::shared_ptr< grid_generator > result;
  result.reset( new split_tube_grid( this->channel_half_length_, 2*this->half_length(), this->radius(), count, rgenr ) );
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

boost::shared_ptr< grid_generator > membrane_cylinder_region::do_make_gridder(double spacing, utility::random_distribution & rgenr) const
{
  boost::shared_ptr< grid_generator > result;
  result.reset( new split_tube_grid( this->channel_half_length_, 2*this->half_length(), this->radius(), spacing, rgenr ) );
  return result;

}

// Calculate new random position for particle of the given radius to
// be inside the region. The position distribution should be uniform 
// over the volume.
//
// \pre fits(radius)
// \post is_inside(pos, radius)
//
// PROs: bound number of calls to rgnr and bound calculation time.
//
// CONs: complex, requires careful testing to guarrantee equivolume 
//   distribution.

coordinate membrane_cylinder_region::do_new_position(utility::random_distribution & rgnr, double radius) const
{
  // A random position anywhere in cell
  geometry::coordinate pos( 0.0, 0.0, 0.0 );
  // search cache for entry
  for( std::size_t idx = 0; idx != this->cache_.size(); ++idx )
  {
    if( utility::feq( radius, this->cache_[idx].radius() ) )
    {
      return this->cache_[idx].generate_position( rgnr );
    }
  }
  // if here then no inserter found.
  this->cache_.push_back( new membrane_cell_inserter( radius, this->radius(), this->half_length(), this->channel_radius(), this->channel_half_length(), this->arc_radius() ) );
  return this->cache_.back().generate_position( rgnr );

}

// Calculate new random position for particle of the given
// radius to be inside the region. This alternate version
// randomly allocates a point in the bounding cylinder (V_bound)
// and tests to see if it is in the allowed volume, repeating
// the procedure until a point lies within the allowed volume
// (V_allowed). The run time of the method is therefore unbound,
// but should average to V_bound/V_allowed times longer than a
// cylinder of V_bound.
//
// \pre fits(radius)
// \post is_inside(pos, radius)
//
// PROs: robust, simple to guarrantee equivolume 
//   distribution.
//
// CONs: unbound number of calls to rgnr.
//

coordinate membrane_cylinder_region::alt_new_position(utility::random_distribution & rgnr, double radius) const
{
  // A random position within cylinder
  geometry::coordinate pos( 0.0, 0.0, 0.0 );
  do
  {
    // volume is linear in square of radius
    const double r( std::sqrt(rgnr.uniform( std::pow( this->radius() - radius, 2 ) ) ) );
    const double phi( rgnr.uniform( 0.0, core::constants::pi() * 2 ) );
    pos.x = r * std::cos( phi );
    pos.y = r * std::sin( phi );
    const double halflength{ this->half_length() + this->channel_half_length() - radius };
    pos.z = rgnr.uniform( -halflength, halflength );
  } while( not this->do_is_inside( pos, radius ) );
  return pos;
  

}

// Volume of cell between z0 and z1
//
// \pre radius < channel_radius
// \pre z0, z1 in [ 0, channel_half_length + half_length - radius ]
// \pre z0 < z1
double membrane_cylinder_region::sub_volume(double z0, double z1, double radius)
{
  UTILITY_REQUIRE( z0 >= 0.0, "Symmetry around 0.0, only use positive z values." );
  UTILITY_REQUIRE( z0 < z1, "Must have a positive interval." );
  UTILITY_REQUIRE( radius < this->channel_radius(), "Must have a sphere that fits inside the channel." );
  UTILITY_REQUIRE( z1 <= (this->half_length() + this->channel_half_length() - radius), "Interval must fit inside the cell." );
  const double vestibule_offset{ this->channel_half_length() - this->arc_radius() };
  if( z1 <= vestibule_offset )
  {
    // both in channel : pi.r2.h
    return core::constants::pi() * std::pow( this->channel_radius() - radius, 2 ) * (z1- z0);
  }
  else if ( z0 >= this->channel_half_length() + radius )
  {
    // both in cell
    return core::constants::pi() * std::pow( this->radius() - radius, 2 ) * (z1- z0);
  }
  else
  {
    double result{ 0.0 };
    double z0bar{ z0 };
    double z1bar{ z1 };
    if( z0 < vestibule_offset )
    {
      z0bar = vestibule_offset;
      // z0 in channel
      if( this->channel_radius() >= radius )
      {
        result += core::constants::pi() * std::pow( this->channel_radius() - radius, 2 ) * (vestibule_offset - z0);
      }
    }
    if( z1 > this->channel_half_length() + radius )
    {
      // z1 in cell
      z1bar = this->channel_half_length() + radius;
      result += core::constants::pi() * std::pow( this->radius() - radius, 2 ) * (z1 - (this->channel_half_length() + radius));
    }
    return result + membrane_cell_inserter::volume_of_rotation( vestibule_offset, this->channel_radius() + this->arc_radius(), this->arc_radius() + radius, z0bar, z1bar );
  }

}

// Update wr[ix] section with information from the derived class.
void membrane_cylinder_region::do_write_document(core::input_document & wr, std::size_t ix) const
{
  const std::string half_length_label { "half-length" };
  const std::string radius_label { "radius" };
  const std::string offset_label { "channel-half-length" };
  const std::string tube_radius_label { "channel-radius" };
  const std::string arc_label { "arc" };
  wr[ix].add_entry( core::strngs::fstype(), type_label() );
  wr[ix].add_entry( half_length_label, this->half_length() );
  wr[ix].add_entry( radius_label, this->radius() );
  wr[ix].add_entry( arc_label, this->arc_radius() );
  wr[ix].add_entry( offset_label, this->channel_half_length() );
  wr[ix].add_entry( tube_radius_label, this->channel_radius() );
  

}

// Factory method for creating membrane_cylinder region objects from
// input data.
boost::shared_ptr< base_region > membrane_cylinder_region::region_factory(std::string label, const std::vector< core::input_parameter_memo > & params)
{
  // needed information
  // label : from label arg
  // radius : from param arg
  // half_length : from param arg
  // offset : from param arg
  // arc_radius : from param arg
  // tube_radius : from param arg
  double half_length {};
  double radius {};
  double offset {};
  double tube_radius {};
  double arc_radius {};
  size_t mask{ 0ul };
  
  const std::string half_length_label { "half-length" };
  const std::string radius_label { "radius" };
  const std::string offset_label { "channel-half-length" };
  const std::string tube_radius_label { "channel-radius" };
  const std::string arc_label { "arc" };
  for( auto const& param : params )
  {
    if( param.name() == half_length_label )
    {
      half_length = param.get_float( "Cell membrane region", core::strngs::fsregn(), true, false );
      mask |= 1ul;
    }
    else if( param.name() == radius_label )
    {
      radius = param.get_float( "Cell membrane region", core::strngs::fsregn(), true, false );
      mask |= 2ul;
    }
    else if( param.name() == offset_label )
    {
      offset = param.get_float( "Cell membrane region", core::strngs::fsregn(), true, false );
      mask |= 4ul;
    }
  
    else if( param.name() == tube_radius_label )
    {
      tube_radius = param.get_float( "Cell membrane region", core::strngs::fsregn(), true, false );
      mask |= 8ul;
    }
  
    else if( param.name() == arc_label )
    {
      arc_radius = param.get_float( "Cell membrane region", core::strngs::fsregn(), true, false );
      mask |= 16ul;
    }
  }
  {
    std::string missing_params;
    // check for required parameters.
    if( ( mask & 1ul ) == 0ul ) missing_params = half_length_label;
    if( ( mask & 2ul ) == 0ul ) missing_params += (missing_params.empty() ? "": " ") + radius_label;
    if( ( mask & 4ul ) == 0ul ) missing_params += (missing_params.empty() ? "": " ") + offset_label;
    if( ( mask & 8ul ) == 0ul ) missing_params += (missing_params.empty() ? "": " ") + tube_radius_label;
    if( ( mask & 16ul ) == 0ul ) missing_params += (missing_params.empty() ? "": " ") + arc_label;
    if( not missing_params.empty() )
    {
      UTILITY_CHECK( params.back().name() == core::strngs::fsend(), "Missing guard entry in vector." );
      throw core::input_error::missing_parameters_error( "Cell membrane", core::strngs::fsregn(), missing_params, params.back().filename(), params.back().line_number() );
    }
  }
  
  // build region object.
  boost::shared_ptr< geometry::base_region > current;
  current.reset( new membrane_cylinder_region( label, radius, half_length, offset, tube_radius, arc_radius ) );
  return current;

}

// Create and add a region_definition to the meta object.
void membrane_cylinder_region::add_definition(region_meta & meta)
{
  std::string desc( "A Cylindrical region centred on the origin with the linear axis in z direction. All walls are considered hard. The cylinder has a membrane splitting the cell in the centre providing two compartments. A tube with rounded ends links these two compartments. This region may be used as a system region." );
  std::unique_ptr< region_definition > cylinder_defn( new region_definition( type_label(), desc, &membrane_cylinder_region::region_factory ) );
  cylinder_defn->add_definition( { "radius", "distance in Angstrom", ">0", "required", "The radius of the cylinder in the XY plane." } );
  cylinder_defn->add_definition( { "half-length", "distance in Angstrom", ">0", "required", "Half the length of the cylinder. The cylinder is centred on the origin so: offset <= |z| <= half-length + offset." } );
  cylinder_defn->add_definition( { "channel-length", "distance in Angstrom", ">0", "required", "Half the length of the channel between the two cell compartments." } );
  cylinder_defn->add_definition( { "channel-radius", "distance in Angstrom", ">0", "required", "The radius of the channel between the two cell compartments." } );
  cylinder_defn->add_definition( { "arc-radius", "distance in Angstrom", ">0", "required", "The radius of the arc that rounds the channel ends into the membrane." } );
  meta.add_definition( cylinder_defn );

}


} // namespace geometry

BOOST_CLASS_EXPORT_IMPLEMENT( geometry::membrane_cylinder_region );