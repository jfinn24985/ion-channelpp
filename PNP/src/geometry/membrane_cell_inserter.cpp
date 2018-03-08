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

#include "geometry/membrane_cell_inserter.hpp"
#include "utility/archive.hpp"

#include "utility/random.hpp"

// - manual includes
#include "core/constants.hpp"
#include "utility/fuzzy_equals.hpp"
namespace geometry {

membrane_cell_inserter::membrane_cell_inserter(double radius, double cell_radius, double cell_hlength, double channel_radius, double channel_hlength, double arc_radius)
: radius_( radius )
, reduced_cell_radius_( cell_radius - radius )
, compartment_factor_( std::pow( cell_radius - radius, 2 ) * core::constants::pi() * ( cell_hlength - 2 * radius ) )
, cell_offset_( channel_hlength + radius )
, reduced_channel_radius_( channel_radius - radius )
, inner_channel_factor_( std::pow( channel_radius - radius, 2 ) * core::constants::pi() * ( channel_hlength - arc_radius ) )
, vestibule_factor_( volume_of_rotation( channel_radius + arc_radius, arc_radius + radius ) )
, vestibule_outer_radius_( channel_radius + arc_radius )
, vestibule_radius_( arc_radius + radius )
, vestibule_offset_( channel_hlength - arc_radius )
{}


void membrane_cell_inserter::swap(membrane_cell_inserter & source)
{
  std::swap( this->radius_, source.radius_ );
  std::swap( this->reduced_cell_radius_, source.reduced_cell_radius_ );
  std::swap( this->compartment_factor_, source.compartment_factor_ );
  std::swap( this->cell_offset_, source.cell_offset_ );
  std::swap( this->reduced_channel_radius_, source.reduced_channel_radius_ );
  std::swap( this->inner_channel_factor_, source.inner_channel_factor_ );
  std::swap( this->vestibule_factor_, source.vestibule_factor_ );
  std::swap( this->vestibule_outer_radius_, source.vestibule_outer_radius_ );
  std::swap( this->vestibule_radius_, source.vestibule_radius_ );
  std::swap( this->vestibule_offset_, source.vestibule_offset_ );

}

bool membrane_cell_inserter::compare_equal(const membrane_cell_inserter & source) const
{
  return this->radius_            == source.radius_ and
    this->reduced_cell_radius_    == source.reduced_cell_radius_ and
    this->compartment_factor_     == source.compartment_factor_ and
    this->cell_offset_            == source.cell_offset_ and
    this->reduced_channel_radius_ == source.reduced_channel_radius_ and
    this->inner_channel_factor_   == source.inner_channel_factor_ and
    this->vestibule_factor_       == source.vestibule_factor_ and
    this->vestibule_outer_radius_ == source.vestibule_outer_radius_ and
    this->vestibule_radius_       == source.vestibule_radius_ and
    this->vestibule_offset_       == source.vestibule_offset_;
  

}

// Generate a random position uniformly in this space.
coordinate membrane_cell_inserter::generate_position(utility::random_distribution & rgnr)
{
  coordinate pos{};
  constexpr double pi = core::constants::pi();
  const double full_factor{ this->compartment_factor_ + this->inner_channel_factor_ + this->vestibule_factor_ };
  const double zfactor = rgnr.uniform( -full_factor, full_factor );
  const double abs_zfactor{ std::abs( zfactor ) };
  
  double max_radius {};
  // calculate z pos and radius
  if ( abs_zfactor >= this->inner_channel_factor_ + this->vestibule_factor_ )
  {
    // in compartment volume
    pos.z = abs_zfactor - (this->inner_channel_factor_ + this->vestibule_factor_);
    // Now linear in compartment volume, reduce to length
    pos.z /= (std::pow( this->reduced_cell_radius_, 2 ) * pi);
    // in cell compartment
    pos.z += this->cell_offset_;
    if (zfactor < 0)
    {
      pos.z = -pos.z;
    }
    max_radius = this->reduced_cell_radius_;
  }
  else if ( abs_zfactor <= this->inner_channel_factor_ )
  {
    pos.z = (abs_zfactor / (std::pow( this->reduced_channel_radius_, 2 ) * pi));
    if (zfactor < 0)
    {
      pos.z = -pos.z;
    }
    // fully in channel
    max_radius = this->reduced_channel_radius_;
  }
  else
  {
    // in channel vestibule: z is non-linear in volume
    //
    // NOTE abs_zfactor = volume_of_rotation(0, R, r, 0, z)
    //  where R = channel_radius + arc_radius
    //        r = arc_radius + radius
    //        z = unknown z coordinate
    //
    // derivative of volume_of_rotation is:
    //  pi{R^2 + r^2 - z^2 - R(sqrt(r^2 - z^2) - 2*z/sqrt(r^2 - z^2) + rcos(z/r)}
    const double RR = this->vestibule_outer_radius_;
    const double rr = this->vestibule_radius_;
    auto deriv= [RR,rr,pi](double zz)->double
    {
      double sqt = std::sqrt(rr*rr - zz*zz);
      return pi * ( RR*RR + rr*rr - zz*zz - RR * ( sqt - (2*zz/sqt) + rr *std::cos(zz/rr)));
    };
    double vol { abs_zfactor - this->inner_channel_factor_ };
    UTILITY_CHECK( vol < this->vestibule_factor_, "Error in logic, not in expected volume" );
    // first guess is linear approx
    double zguess = rr * std::sqrt( vol / this->vestibule_factor_ );
    // do four Newton/Raphson steps
    //  f(z) = volume_of_rotation(0, RR, rr, 0, z) - vol = 0
    //  f'(z) = deriv(z)
    for (std::size_t ii = 0; ii < 4; ++ii)
    {
      zguess -= ((zguess == 0.0 ? 0.0 : volume_of_rotation(0.0, RR, rr, 0.0, zguess)) - vol)/deriv(zguess);
      zguess = zguess < 0.0 ? 0.0 : ( zguess > rr ? rr : zguess );
    }
    // zguess is bound to [ 0, this->arc_radius_ + radius ]
    pos.z = this->vestibule_offset_ + zguess;
    if (zfactor < 0)
    {
      pos.z = -pos.z;
    }
    //
    // closest approach at pos.z is when 
    //
    //  ddr * ddr + z * z = r * r 
    // 
    //  where ddr is distance from arc centrepoint. This
    //  gives final radius as
    //
    //  max_radius = channel_radius + arc_radius - ddr
    const double ddr_sq { std::pow(rr, 2) - std::pow(zguess,2) };
    max_radius = RR - std::sqrt( ddr_sq );
  }
  // volume distribution is linear in square of radius and phi
  const double r( std::sqrt(rgnr.uniform( std::pow(max_radius, 2 ) ) ) );
  const double phi( rgnr.uniform( 0.0, core::constants::pi() * 2 ) );
  pos.x = r * std::cos( phi );
  pos.y = r * std::sin( phi );
  //UTILITY_ENSURE( this->is_inside( pos, radius ), "Error in calculating new position." );
  return pos;

}

// Calculate the volume of rotation _underneath_ an arc between z_0
// and z_1.
//
// This calculates the volume of rotation underneath an arc whose
// centrepoint is (axial_disp,radial_disp) and has the given arc_radius.
// The volume is the slice between the z_0 and z_1 .
//
// \pre radial_disp > arc_radius

double membrane_cell_inserter::volume_of_rotation(double axial_disp, double radial_disp, double arc_radius, double z_0, double z_1)
{
  UTILITY_REQUIRE(arc_radius < radial_disp, "The arc radius must be smaller than the radial offset.");
  UTILITY_REQUIRE(not utility::feq(z_0,z_1), "Z0 can not equal Z1.");
  UTILITY_REQUIRE(z_0 < z_1, "Z0 must be less than Z1.");
  const double z1bar_{ utility::feq(z_1, axial_disp + arc_radius) ? arc_radius : z_1 - axial_disp };
  const double z0bar_{ utility::feq(z_0, axial_disp) ? 0.0 : z_0 - axial_disp };
  UTILITY_REQUIRE(z0bar_ >= 0.0, "Z0 must be within one arc_radius of axial_disp.");
  UTILITY_REQUIRE(z1bar_ <= arc_radius, "Z1 must be within one arc_radius of axial_disp.");
  
  const double arc_r2 (std::pow(arc_radius,2));
  if (utility::feq(z0bar_, 0.0))
  {
     // Handle case where x0 = xc --> z0bar_ = 0
     return core::constants::pi()
            * (
               (std::pow(radial_disp,2) + arc_r2) * (z_1 - z_0)
               - std::pow(z1bar_,3)/3.0
               - radial_disp * (z1bar_ * std::sqrt(arc_r2 - std::pow(z1bar_,2))
                                + arc_r2 * asin(z1bar_/arc_radius))
            );
  }
  else if (utility::feq(z1bar_, arc_radius))
  {
     // handle case where x1 = xc + r --> z1bar_ = r)
     return core::constants::pi()
            * (
               (std::pow(radial_disp,2) + arc_r2) * (z_1 - z_0)
               + (std::pow(z0bar_,3) - std::pow(z1bar_,3))/3.0
               - radial_disp * (arc_r2 * core::constants::pi() / 4.0)
               + radial_disp * (z0bar_ * std::sqrt(arc_r2 - std::pow(z0bar_,2))
                                + arc_r2 * std::asin(z0bar_/arc_radius))
            );
  }
  else
  {
     return core::constants::pi()
            * (
               (std::pow(radial_disp,2) + arc_r2) * (z_1 - z_0)
               + (std::pow(z0bar_,3) - std::pow(z1bar_,3))/3.0
               - radial_disp * (z1bar_ * std::sqrt(arc_r2 - std::pow(z1bar_,2))
                                + arc_r2 * std::asin(z1bar_/arc_radius))
               + radial_disp * (z0bar_ * sqrt(arc_r2 - std::pow(z0bar_,2))
                                + arc_r2 * std::asin(z0bar_/arc_radius))
            );
  }
  

}


} // namespace geometry
