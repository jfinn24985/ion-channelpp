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

#include "evaluator/icc_surface_grid.hpp"

// manual includes
#include "core/constants.hpp"
#include "core/strngs.hpp"
#include "utility/fuzzy_equals.hpp"
#include "utility/utility.hpp"
// -
#include <array>
// -
namespace evaluator {

//  The version of the patch file this class understands
const std::size_t icc_surface_grid::patch_file_version_ = 1;

icc_surface_grid::icc_surface_grid()
: area_()
, deps_()
, eps_in_()
, eps_out_()
, intgs_()
, ux_()
, uy_()
, uz_()
, xyz_()
, dx_zaxis_( 1.6 )
, dx_radial_( 1.6 )
, nsub0_( 10 )
, npatch_( 0 )
, npatch_alloc_( 0 )
{}


icc_surface_grid::~icc_surface_grid()
{

}

// Write the tile definitions to disk.
void icc_surface_grid::write_grid(std::ostream & output) const
{
  UTILITY_REQUIRE( output.good (), "Invalid stream for writing." );
  
  output << core::strngs::comment_begin() << " " << patch_file_version_ << " " << core::strngs::comment_end() << "\n"; // file version
  output << core::strngs::comment_begin() << " " << this->npatch_ << " " << core::strngs::comment_end() << "\n";
  for ( std::size_t idx = 0; idx != this->npatch_; ++idx )
  {
     output << idx << " "
      << this->xyz_.x( idx ) << " "
      << this->xyz_.y( idx ) << " "
      << this->xyz_.z( idx ) << " "
      << this->area_[ idx ] << " "
      << this->ux_[ idx ] << " "
      << this->uy_[ idx ] << " "
      << this->uz_[ idx ] << " "
      << this->deps_[ idx ] << " "
      << this->eps_in_[ idx ] << " "
      << this->eps_out_[ idx ] << "\n";
  }

}

// Total area of all surface elements
double icc_surface_grid::surface_area() const
{
  double sum{ 0.0 };
  for( const double ar : this->area_ )
  {
    sum += ar;
  }
  return sum;

}

// Do we have integrators available to be able to generate
// the matrix?
//
// \return not intg_.empty and intg_.size = size
bool icc_surface_grid::have_integrators() const
{
  return not this->intgs_.empty() and this->intgs_.size() == this->npatch_;
}

// ----------------------------------------
// Calculate tiles along a cylinder. When is_internal is false
// the calculation of patches is changed to create larger
// patches.  The changes are:
//  dx = (is_internal ? dxf : dxw/sqrt(rl2))
//  nzl = max( ( a_z_min ), int(dll/dx) + 1)
//  nfil = max( a_r_min, int(cir/dx) + 1)
//
//
// @param a_zleft : the z-value of left end of cylinder
// @param a_zright : the z-value of right end of cylinder
// @param a_radius : the radius of the cylinder
// @param a_deps : the change in permitivity across boundary
// @param a_z_min : the minimum number of tile in the z direction
//       (Fortran ionch used 10 for inner and ?4? for outer
// @param is_internal : whether the tiles face towards (true) the
//       z-axis or away. 
void icc_surface_grid::add_line(double a_zleft, double a_zright, double a_radius, double a_epsin, double a_epsout, std::size_t a_z_min, std::size_t a_r_min, bool is_internal)
{
//os_log << "Cylinder tiles:\n";
//os_log << " start (z,r) = " << a_zleft << " " << a_radius << "\n";
//os_log << "   end (z,r) = " << a_zright << " " << a_radius << "\n";
const int ulsign (is_internal ? -1 : 1);
// Make outer tiles larger along z-axis in proportion to radius
const double dx(is_internal ? this->dx_zaxis_ : this->dx_zaxis_ * std::sqrt( a_radius ) );
// calculate number of tiles in z coordinate (minimum of a_z_min)
const int nzl_( std::max< int >(std::floor( (a_zright - a_zleft) / dx) + 1, a_z_min) );
const double tile_zwidth_((a_zright - a_zleft) / double(nzl_));
//os_log << "mesh size (z) = " << nzl_ << "\n";

// in cylinder all slices have same circumference and and so all
// tiles have the same angular size and area. Use of dx makes outer
// tiles larger than inner tiles.
const double circumference( 2 * core::constants::pi() * a_radius );
const int nfil_( std::max< std::size_t >( a_r_min, std::floor( circumference / dx ) + 1 ) );
const double dfil_(2 * core::constants::pi() / double(nfil_));
// CORRECT : 
const double area_( a_radius * dfil_ * tile_zwidth_);
// F95 BUG : const double area_( dfil_ * tile_zwidth_);

//os_log << "mesh size (r) = " << nfil_ << "\n";

double zzl1 = 0.0;
double zzl2 = 0.0; // z-axial edges of the tiles
for (int j = 0; j != nzl_; ++j)
{
   //   'j'th ring
   if (0 == j)
   {
      zzl1 = a_zleft;
      zzl2 = a_zleft + tile_zwidth_;
   }
   else if (nzl_ - 1 == j)
   {
      zzl1 = zzl2; // zzl2 from last iteration
      zzl2 = a_zright;
   }
   else
   {
      zzl1 = zzl2; // zzl2 from last iteration
      zzl2 = zzl1 + tile_zwidth_;
   }

   const double zlc_( zzl1 + tile_zwidth_/2 );
   double phil1 = 0.0;
   double phil2 = 0.0; // angular edges of the tiles

   // ensure we have enough space
   if (this->npatch_ + nfil_ >= this->npatch_alloc_)
   {
      this->resize( this->npatch_ + nfil_ );
   }
   for (int k = 0; k != nfil_; ++k)
   {
      if (0 == j) // Only need to calculate cos/sin(fik) once
      {
         const double fik = (k + 0.50) * dfil_;
         this->xyz_.set_x( this->npatch_, a_radius * std::cos( fik ) );
         this->xyz_.set_y( this->npatch_, a_radius * std::sin( fik ) );
         this->ux_[ this->npatch_ ] = ulsign * std::cos( fik );
         this->uy_[ this->npatch_ ] = ulsign * std::sin( fik );
      }
      else
      {
         // For j /= 0, these are the same as nfil_ tiles ago
         this->xyz_.set_x( this->npatch_, this->xyz_.x( this->npatch_ - nfil_ ) );
         this->xyz_.set_y( this->npatch_, this->xyz_.y( this->npatch_ - nfil_ ) );
         this->ux_[ this->npatch_ ] = this->ux_[ this->npatch_ - nfil_ ];
         this->uy_[ this->npatch_ ] = this->uy_[ this->npatch_ - nfil_ ];
      }
      this->xyz_.set_z( this->npatch_, zlc_ );
      this->uz_[ this->npatch_ ] = 0.0;
      this->area_[ this->npatch_ ] = area_;
      this->deps_[ this->npatch_ ] = 2 * (a_epsout - a_epsin) / (a_epsin + a_epsout);
      this->eps_in_[ this->npatch_ ] = a_epsin;
      this->eps_out_[ this->npatch_ ] = a_epsout;

      if (k==0)
      {
         phil1 = 0.0;
         phil2 = dfil_;
      }
      else if (k + 1 == nfil_)
      {
         phil1 = phil2; // phil2 from last iteration
         phil2 = 2 * core::constants::pi();
      }
      else
      {
         phil1 = phil2; // phil2 from last iteration
         phil2 = phil1 + dfil_;
      }
      // Integrator wants edges of tile. For cylinder tiles this is
      // zlc +/- 1/2 tile_zwidth and fik +/- 1/2 dfil
      // (we also store radius so we don't need to recalculate it from
      // prx and pry.)
      UTILITY_CHECK( this->intgs_.size () == this->npatch_, "Integrator array : index mismatch");
      intgs_.push_back( new cylinder_integrator( this->nsub0_, this->npatch_, a_radius, zzl1, zzl2, phil1, phil2 ) );

      ++this->npatch_;
   }
}

}

//
//
// \param is_outer : is the media on the outer side of the
//    arc or inner side. (For the four "corners" of the standard 
//    toroid will be true.)
void icc_surface_grid::add_arc(double za0, double ra0, double ra, double ta1, double ta2, double a_epsin, double a_epsout, std::size_t a_z_min, std::size_t a_r_min, bool is_outer)
{
// NOTE: This is reverse of F95 value.  We do this
// because the F95 assigns the negative value of the
// calculated vector to the normal vector. Here we
// fold this negation into the uasign value 
const int uasign_ (is_outer ? 1 : -1);
//os_log << "Arch tiles:\n";
//os_log << "  center (z,r) = " << za0 << " " << ra0 << "\n";
//os_log << "    arc radius = " << ra << "\n";
//os_log << " angles (begin,end) = " << ta1 << " " << ta2 << "\n";
const double dta = ta2 - ta1;
//os_log << "arc sweep (phi)= " << dta << "\n";
UTILITY_CHECK( utility::feq( core::constants::pi(), 2*dta ),"Only 1/4 circle arcs tested" );
const double dla = ra * dta;
//os_log << "arc length (z) = " << dla << "\n";

// LOOP OVER THETA ANGLE
//theta is measured along channel axis;
//
const double dx( std::max( this->dx_zaxis_, this->dx_zaxis_ * std::sqrt( ra0 ) ) );
//
const int nta_( std::max< int >( std::floor( dla / dx ) + 1, a_z_min) );

//os_log << " mesh size (z) = " << nta_ << "\n";

const double dtta = dta / double(nta_);
//loop over arc theta
double tta1 = 0.0;
double tta2 = 0.0;
for (int j = 0; j != nta_; ++j)
{
   //os_log << (j + 1) << "th ring\n";
   //each section of the arc has length dtta(arcnumber,ring);
   // if it's the first ring, set to beginning theta of
   // the arc keep in mind it seems that we are counting
   // arcs from right to left, since we start at theta1,
   // hopefully this doesn't make much of a difference
   // (i.e. doesn't matter if go ta1 to ta2 or vice
   // versa)
   tta1 = (0 == j ? ta1 : tta2);
   tta2 = (nta_ == j + 1 ? ta2 : tta1 + dtta);

   // find the center theta (which is not just the average of
   // tta1,tta2)
   const double ara ((ra0 * dtta) - (ra * (std::cos (tta2) - std::cos (tta1))));
   UTILITY_CHECK (not std::isnan(ara) and not utility::feq(ara, 0.0)
                , "ara is NaN or zero");

   const double szaml (0.5* ra0 * (tta2*tta2 - tta1*tta1)
                       - ra * (tta2 * std::cos(tta2) - tta1 * std::cos(tta1))
                       + ra * (std::sin(tta2) - std::sin(tta1)));

   const double tac (szaml / ara);
   // radius of ring centres
   const double rc (ra0 + ra * std::sin (tac));
   // divide circumference at ring centre
   const int nfia_( std::max< int >( a_r_min,  std::floor(2 * core::constants::pi() * rc/dx ) + 1 ) );
   const double dfia_( 2 * core::constants::pi() / double(nfia_) );

   //os_log << "theta (low,mid,high)  = " << tta1 << " " << tac << " " << tta2 << "\n";
   //os_log << "mesh size (r) = " << nfia_ << "\n";

   const double tile_area_ (ra * ara * dfia_);
   const double zlc_ (za0 + ra * std::cos (tac));
   const double uzc_ (uasign_ * std::cos (tac));
   const double ua_sintac_ (uasign_ * std::sin (tac));
   double phil1 = 0.0;
   double phil2 = 0.0;
   // ensure space
   if (this->npatch_ + nfia_ >= this->npatch_alloc_)
   {
      this->resize( this->npatch_ + nfia_ );
   }
   // loop over all the slices of phi;
   for (int k = 0; k != nfia_; ++k)
   {
      const double fik = (k+0.5) * dfia_;

      this->xyz_.set_x( this->npatch_, rc * std::cos( fik ) );
      this->xyz_.set_y( this->npatch_, rc * std::sin( fik ) );
      this->xyz_.set_z( this->npatch_, zlc_ );

      this->ux_[ this->npatch_ ] = ua_sintac_ * std::cos( fik );
      this->uy_[ this->npatch_ ] = ua_sintac_ * std::sin( fik );
      this->uz_[ this->npatch_ ] = uzc_;

      this->area_[ this->npatch_ ] = tile_area_;
      this->deps_[ this->npatch_ ] = (2 * (a_epsout - a_epsin) / (a_epsin + a_epsout) );
      this->eps_in_[ this->npatch_ ] = a_epsin;
      this->eps_out_[ this->npatch_ ] = a_epsout;

      phil1 = (0 == k ? 0.0 : phil2);
      phil2 = (k + 1 == nfia_ ? 2 * core::constants::pi() : phil1 + dfia_);

      // Integrator wants edges of tile. For arc tiles this is
      // found along z-axis by za0,ra0,ra,tta1 + tta2 and fik
      // +/- 1/2 dfil around radial.

      UTILITY_CHECK( this->intgs_.size () == this->npatch_, "Integrator array : index mismatch");
      this->intgs_.push_back( new arc_integrator( this->nsub0_, this->npatch_, za0, ra0, ra, tta1, tta2, phil1, phil2 ) );
      
      ++this->npatch_;
   }
}

}

//
//
//
// \param is_to_positive : is the media on the more positive side of the wall?
void icc_surface_grid::add_wall(double a_z, double a_rl1, double a_rl2, double a_epsin, double a_epsout, std::size_t min_phi, std::size_t min_rad, bool is_to_positive)
{
//os_log << "Wall tiles:\n";
//os_log << " start (z,r1,r2) = " << a_z << " " << a_rl1 << " " << a_rl2 << "\n";
const double dll = a_rl2 - a_rl1;
//os_log << "  length (r) = " << dll << "\n";
const int ulsign (is_to_positive ? 1 : -1);

// Number of tiles around ring at centre of wall
const double cir( 2 * core::constants::pi() * (a_rl1 + (dll/2.0) ) );
const int nfiu( std::max< int >( min_phi, std::floor( cir / this->dx_radial_ + 1 ) ) );

// Angular extent of tiles
const double dfiu(2 * core::constants::pi() / nfiu);
// Calculate radial tile number
const double ratio1( (1 + dfiu/2)/(1 - dfiu/2) );
////os_log << " Ratio old = " << ratio1 << "\n";
const bool at_axis ( a_rl1 == 0.0 );
double min_r( at_axis ? this->dx_zaxis_: a_rl1 );
const int nrl_( std::max< int >( min_rad, std::floor( std::log( a_rl2 / min_r ) / std::log( ratio1 ) ) + (at_axis ? 2 : 1) ) );
const double ratio2 = std::pow(a_rl2 / min_r, (1 / double(nrl_)));
////os_log << " Ratio new = " << ratio2 << "\n";
//os_log << "mesh size (radial) = " << nrl_ << "\n";

double rrl1 = 0.0;
double rrl2 = 0.0;
for (int j = 0; j != nrl_; ++j)
{
   rrl1 = (0 == j ? a_rl1 : rrl2);
   rrl2 = (0 == j and at_axis ? this->dx_zaxis_ : (nrl_ == j + 1 ? a_rl2 : rrl1 * ratio2) );

   // DIFF2FORTRAN div arr by 2 and div szaml by 3 moved to rc/parea calc
   const double arr   (std::pow( rrl2, 2 ) - std::pow( rrl1, 2 ));
   const double szaml (std::pow( rrl2, 3 ) - std::pow( rrl1, 3 ));
   const double rc    ((2 * szaml) / (3 * arr));
   const double area  (arr * dfiu / 2);
   //os_log << " r center = " << rc << "\n";

   // --- loop over phi angle --------------------;
   //os_log << " r (low,mid,high) = " << rrl1 << " " << rc << " " << rrl2 << "\n";
   //os_log << "mesh size (phi) = " << nfiu << "\n";
   double phi1 = 0.0;
   double phi2 = 0.0;
   // ensure we have enough space
   if (this->npatch_ + nfiu >= this->npatch_alloc_)
   {
      this->resize( this->npatch_ + nfiu );
   }
   for (int k = 0; k != nfiu; ++k)
   {
      const double fik = (k + 0.5) * dfiu;
      phi1 = (0 == k ? 0.0 : phi2);
      phi2 = (nfiu == k + 1 ? core::constants::pi() * 2 : phi1 + dfiu);

      this->xyz_.set_x( this->npatch_, rc * std::cos( fik ) );
      this->xyz_.set_y( this->npatch_, rc * std::sin( fik ) );
      this->xyz_.set_z( this->npatch_, a_z );

      this->ux_[ this->npatch_ ] = 0;
      this->uy_[ this->npatch_ ] = 0;
      this->uz_[ this->npatch_ ] = ulsign;

      // DIFF2FORTRAN div arr by 2 moved to parea calc
      this->area_[ this->npatch_ ] = area;
      this->deps_[ this->npatch_ ] = 2 * (a_epsout - a_epsin) / (a_epsin + a_epsout);
      this->eps_in_[ this->npatch_ ] = a_epsin;
      this->eps_out_[ this->npatch_ ] = a_epsout;

      UTILITY_CHECK( this->intgs_.size() == this->npatch_, "Integrator array : index mismatch");
      this->intgs_.push_back( new wall_integrator( this->nsub0_, this->npatch_, a_z, rrl1, rrl2, rc, phi1, phi2 ) );

      ++this->npatch_;
   }
}

}

// Remove the tile definitions.
void icc_surface_grid::clear()
{
  this->area_.clear();
  this->deps_.clear();
  this->eps_in_.clear();
  this->eps_out_.clear();
  this->ux_.clear();
  this->uy_.clear();
  this->uz_.clear();
  this->xyz_.clear();
  this->intgs_.clear();
  this->npatch_ = 0;

}

//  After generating the A matrix the integrator objects are no-longer
//  needed and, optionally, can be removed.
void icc_surface_grid::clear_integrators()
{
  this->intgs_.clear();
}

// Integrate grid
//
// \pre not empty
// \pre have_integrators
void icc_surface_grid::generate_matrix(boost::multi_array< double, 2 > & a_matrix)
{
// NOTE: it is at this point that we know the required size of the A matrix!
UTILITY_REQUIRE( this->npatch_ > 0, "No shapes added to grid." );
UTILITY_REQUIRE( this->npatch_ <= a_matrix.shape()[ 0 ] and this->npatch_ <= a_matrix.shape()[ 1 ],
             "Attempt to calculate ICC matrix before required memory was allocated");
UTILITY_REQUIRE( not this->intgs_.empty(), "clear_integrators has been called, unable to create matrix." );
// ----  Fill matrix -----------------------------------
const std::size_t np10 = this->npatch_ / 10;
// fill A matrix
for (std::size_t percent = 0; percent != 10; ++percent)
{
   const std::size_t begin = percent * np10;
   const std::size_t end   = (9 == percent ? this->npatch_ : begin + np10);
   for (std::size_t ipatch = begin; ipatch != end; ++ipatch)
   {
      std::array< std::size_t, 2 > idx;
      idx[ 0 ] = ipatch;
      const double pxi{ this->xyz_.x( ipatch ) };
      const double pyi{ this->xyz_.y( ipatch ) };
      const double pzi{ this->xyz_.z( ipatch ) };
      const double pux{ this->ux_[ ipatch ] };
      const double puy{ this->uy_[ ipatch ] };
      const double puz{ this->uz_[ ipatch ] };
      // scale integration by change in eps over boundary
      const double scale_eps{ this->deps_[ ipatch ] };
      for (std::size_t jpatch = 0; jpatch != this->npatch_; ++jpatch)
      {
         idx[ 1 ] = jpatch;
         a_matrix( idx ) = this->intgs_[ jpatch ]( ipatch, pxi, pyi, pzi, pux, puy, puz ) * scale_eps;
      }
      //  for A_ii add average eps over boundary
      idx[ 1 ] = ipatch;
      a_matrix( idx ) += 1.0;
   }
}

}

// Resize the allocated memory.
void icc_surface_grid::resize(std::size_t npatch)
{
  const std::size_t sz( utility::mathutil::next64( npatch ) );
  this->npatch_alloc_ = sz;
  this->area_.resize( sz, 0.0 );
  this->deps_.resize( sz, 0.0 );
  this->eps_in_.resize( sz, 0.0 );
  this->eps_out_.resize( sz, 0.0 );
  this->ux_.resize( sz, 0.0 );
  this->uy_.resize( sz, 0.0 );
  this->uz_.resize( sz, 0.0 );
  this->xyz_.resize( sz );
  

}

// Set the patch/tile delta to divide up the circumference
// of circles normal to and centred on the Z-axis (only 
// useful before generating A matrix)
//
// \pre empty
void icc_surface_grid::set_dxf(double val)
{
  UTILITY_REQUIRE( this->empty(), "Cannot change integration settings after defining the geometry." );
  this->dx_radial_ = val;
}


// Set the patch/tile delta that divides lines and arcs that
// are parallel or radial to the z-axis.  (only useful before 
// generating A matrix) 
//
// \pre empty
void icc_surface_grid::set_dxw(double val)
{
  UTILITY_REQUIRE( this->empty(), "Cannot change integration settings after defining the geometry." );
  this->dx_zaxis_ = val;
}


// Set the radial delta 
//
// \pre empty
void icc_surface_grid::set_nsub0(std::size_t val)
{
  UTILITY_REQUIRE( this->empty(), "Cannot change integration settings after defining the geometry." );
  this->nsub0_ = val;
}



} // namespace evaluator
