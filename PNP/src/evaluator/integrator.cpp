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

#include "evaluator/integrator.hpp"

// manual includes
#include "core/constants.hpp"
#include "utility/fuzzy_equals.hpp"
#include "utility/utility.hpp"
// -
#include <array>
#include <cmath>
// -
namespace evaluator {

integrator::integrator()
: index_()
, nsub_other_()
, nsub_self_()
{}


// The only constructor
integrator::integrator(std::size_t a_nsubO, std::size_t a_nsubS, std::size_t a_index)
: index_(a_index)
, nsub_other_(a_nsubO)
, nsub_self_(a_nsubS)
{
  UTILITY_REQUIRE( (a_nsubS % 2) == 0, "Number of subelements in self integration must be even." );
}

//  Should only be used for serialization
arc_integrator::arc_integrator()
: integrator()
, za0_()
, ra0_()
, radius_()
, theta1_()
, theta2_()
, phi1_()
, phi2_()
{}


// Integrate over arc tiles.
double arc_integrator::operator ()(std::size_t pchx, double pxi, double pyi, double pzi, double pux, double puy, double puz) const
{
//
//  calculate ( r_ij . n_i ) / | r_ij |^3
//
//  NOTE: direction of vector r_ij is important for the
//  dot product. The vector goes from tile i to tile j.
//  In this method we are integrating over tile j.
const std::size_t nsub( pchx == this->index() ? this->nsub_self() : this->nsub_other() );
const double z0 (this->za0_); // distance along rotation axis
const double r0 (this->ra0_); // distance from rotation axis
const double r  (this->radius_); // radius of arc
const double t1 (this->theta1_); // start angle along arc
const double t2 (this->theta2_); // end angle along arc
const double fi1(this->phi1_); // start angle along rotation
const double fi2(this->phi2_); // end angle along rotation
// LOCALS
double aij = 0.0; // RESULT

UTILITY_CHECK(not utility::feq(0.0, t2 - t1), "Arc tile has zero width in z direction");
UTILITY_CHECK(not utility::feq(0.0, fi2 - fi1), "Arc tile has zero width in z direction");
const double dtsub = (t2 - t1) / double(nsub);
const double dfisub = (fi2 - fi1) / double(nsub);

// --- double loop over subtiles ----------------------
// --- loop over theta angle --------------------------

for (std::size_t i = 0; i != nsub; ++i)
{
   const double tt1 = t1  + i * dtsub;
   const double tt2 = tt1 + dtsub;
   const double ar = r0 * dtsub - r * (std::cos(tt2) - std::cos(tt1));
   const double szaml = 0.5 * r0 * (std::pow( tt2, 2 ) - std::pow( tt1, 2 ))
                        - r * (tt2 * std::cos(tt2) - tt1 * std::cos(tt1))
                        + r * (std::sin(tt2) - std::sin(tt1));
   const double ttc = szaml / ar;
   UTILITY_CHECK (not std::isnan(ttc), "Error in integrating an arc, offset angle = NaN");
   UTILITY_CHECK (tt1 < ttc and ttc < tt2, "Error in integrating an arc, offset angle outset t1 and t2");
   const double rc = r0 + r * std::sin(ttc);
   const double area = r * ar * dfisub;
   UTILITY_CHECK ((r0-r)*dfisub*r*dtsub <= area, "Error in integrating an arc, area too small");
   UTILITY_CHECK ((r0+r)*dfisub*r*dtsub >= area, "Error in integrating an arc, area too large");
   const double pzj = z0 + r * std::cos(ttc);

   // --- loop over phi angle ----------------------------
   for (std::size_t j = 0; j != nsub; ++j)
   {
      const double fij = fi1 + (j + 0.50) * dfisub;
      const double pxj = rc * std::cos(fij);
      const double pyj = rc * std::sin(fij);
      // Direction is i to j
      const double pxij = pxj - pxi;
      const double pyij = pyj - pyi;
      const double pzij = pzj - pzi;
      const double rijsq = std::pow( pxij, 2 ) + std::pow( pyij, 2 ) + std::pow( pzij, 2 );
      const double rij3  = rijsq * std::sqrt(rijsq);
      aij += (pxij * pux + pyij * puy + pzij * puz) * area / rij3;
   }
}
UTILITY_CHECK (not std::isnan(aij), "Error in integrating an arc, result = NaN");
return aij / (4 * core::constants::pi());

}

// Should only be used for serialization and testing
cylinder_integrator::cylinder_integrator()
: integrator()
, radius_()
, zmin_()
, zmax_()
, phimin_()
, phimax_()
{}


// Integrate over cylinder tiles.
double cylinder_integrator::operator ()(std::size_t pchx, double pxi, double pyi, double pzi, double pux, double puy, double puz) const
{
//
//  calculate ( r_ij . n_i ) / | r_ij |^3
//
//  NOTE: direction of vector r_ij is important for the
//  dot product. The vector goes from tile i to tile j.
//  In this method we are integrating over tile j.

// our tile info
const std::size_t nsub( pchx == this->index() ? this->nsub_self() : this->nsub_other() );
// All radials are the same because all patches on a cylinder
// parallel to z axis 
const double r0 (this->radius_);
// z2 and z1 are end and begin along z axis
const double z1 (this->zmin_);
const double z2 (this->zmax_);
// fi2 and fi1 are the angles of the end and begin around
// the rotation (z) axis
const double fi1 (this->phimin_);
const double fi2 (this->phimax_);

// split up each ring into pieces along z
const double dzsub = (z2 - z1) / double(nsub);

// and around the phi angle
const double dfisub = (fi2 - fi1) / double(nsub);

double aij = 0.0; // RESULT

// --- double loop over subtiles ------
// each patch is going to have nsub*nsub tiny patches
for (std::size_t i = 0; i != nsub; ++i)
{
   // --- loop along rotation axis ------
   // get the centroid 
   // (z coordinate of all patches in a ring are the same)
   const double pzj = z1 + (i + 0.5) * dzsub;
   // Radial from rotation axis is constant and
   // so is area
   const double area( r0 * dfisub * dzsub );

   // --- loop around rotation axis -----
   for (std::size_t j = 0; j != nsub; ++j)
   {
      // loop over tiny patches around phi angle
      // fij is center of tiny patch around phi angle (axis of rotation)
      const double fij = fi1 + (j+0.50) * dfisub;
      // pxj, pyz, and pzj are centroid of tiny patch
      const double pxj   = r0 * std::cos(fij);
      const double pyj   = r0 * std::sin(fij);
      // Direction is i to j
      const double pxij  = pxj - pxi;
      const double pyij  = pyj - pyi;
      const double pzij  = pzj - pzi;
      const double rijsq = std::pow( pxij, 2 ) + std::pow( pyij, 2 ) + std::pow( pzij, 2 );
      // rij is the distance from the patch centroid to the tiny patch
      const double rij3  = rijsq * std::sqrt(rijsq);
      aij += (pxij * pux + pyij * puy + pzij * puz) * area / rij3;
   }
}
UTILITY_CHECK (not std::isnan(aij), "Error in integrating a cylinder, result = NaN");
return aij / (4 * core::constants::pi());

}

wall_integrator::wall_integrator()
: integrator()
, z_()
, ra0_()
, ra1_()
, radius_()
, phi1_()
, phi2_()
{}


// Virtual method for performing the integration.
double wall_integrator::operator ()(std::size_t pchx, double pxi, double pyi, double pzi, double pux, double puy, double puz) const
{
//
//  calculate ( r_ij . n_i ) / | r_ij |^3
//
//  NOTE: direction of vector r_ij is important for the
//  dot product. The vector goes from tile i to tile j.
//  In this method we are integrating over tile j.
const std::size_t nsub( pchx == this->index() ? this->nsub_self() : this->nsub_other() );
const double z0 (this->z_);
const double r1 (this->ra0_);
const double r2 (this->ra1_);
const double fi1(this->phi1_);
const double fi2(this->phi2_);

const double drsub = (r2 - r1) / double(nsub);
const double dfisub = (fi2 - fi1) / double(nsub);

double aij = 0.0; // RESULT
// --- double loop over subtiles ------
// --- loop over theta angle ----------

for (std::size_t i = 0; i != nsub; ++i)
{
   const double rr1 = r1 + (i * drsub);
   const double rr2 = rr1 + drsub;
   const double ar = 0.50 * (std::pow( rr2, 2 ) - std::pow( rr1, 2 ));
   const double szaml = (std::pow( rr2, 3 ) - std::pow( rr1, 3 )) / 3.0;

   const double rc   = szaml / ar;
   const double area = rc * drsub * dfisub;

   const double pzj  = z0;

   // --- loop over phi angle ------------
   for (std::size_t j = 0; j != nsub; ++j)
   {
      const double fij = fi1 + (j + 0.5) * dfisub;
      const double pxj = rc * std::cos(fij);
      const double pyj = rc * std::sin(fij);
      // Direction is i to j
      const double pxij  = pxj - pxi;
      const double pyij  = pyj - pyi;
      const double pzij  = pzj - pzi;
      const double rijsq = std::pow( pxij, 2 ) + std::pow( pyij, 2 ) + std::pow( pzij, 2 );
      // rij is the distance from the patch centroid to the tiny patch
      const double rij3  = rijsq * std::sqrt(rijsq);
      aij += (pxij * pux + pyij * puy + pzij * puz) * area / rij3;
   }
}
UTILITY_CHECK (not std::isnan(aij), "Error in integrating a disk/wall, result = NaN");
return aij / (4 * core::constants::pi());

}


} // namespace evaluator

BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::arc_integrator );
BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::cylinder_integrator );
BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::wall_integrator );