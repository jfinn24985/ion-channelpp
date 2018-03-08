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

#include "geometry/tubular_grid.hpp"
#include "utility/random.hpp"

// manual include
// for pi()
#include "core/constants.hpp"
//-
namespace geometry {

// Initialise the tubular grid with the given length and radius.
//
// This leaves a minimum of 'spacing' distance between any grid
// point and the cylinder edge. Will have a grid point at origin (0,0,0)
// or no points.
//
//\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
//\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
//\param spacing : No two grid points will be less than spacing apart and closer than spacing/2 to cylinder bounds.
//\param shuffle : whether to provide grid points in linear (false)
//   or random (true) order
circle_grid::circle_grid(double radius, double spacing)
: spacing_(spacing)
, width_()
, xy_size_()
{
   // Calculate radius in integral units of spacing
   const int iradius( std::floor((radius - spacing/2.0)/ spacing) );
   UTILITY_CHECK( iradius * spacing <= radius - spacing/2, "Integral radius is too large.");
   UTILITY_CHECK( (iradius + 1) * spacing > radius - spacing/2, "Integral radius is too small.");

   // xi : x index of grid
   int xi = iradius;
   int r2( std::floor( std::pow( (radius - spacing/2)/spacing, 2 ) ) );
   int x2{ iradius * iradius };

   this->width_.resize(iradius + 1);

   int y2 = 0;
   // yi : y index of grid
   for (int yi = 0; yi != iradius + 1 and xi > 0; ++yi)
   {
      while (x2 + y2 > r2)
      {
         x2 -= xi;
         --xi;
         x2 -= xi;
      }
      // test before add to array keeps (xi,yi) inside radius
      this->width_[yi] = xi;
      // DEBUG: std::cout << "| ["<< xi << ", " << yi << "] | = " << (x2 + y2) << " < " << r2 << "\n";
      y2 += 2 * yi + 1;
   }

   // number of gridpoints on the xy plane
   this->xy_size_ = 2 * this->width_[0] + 1;
   for (int i = 1; i != iradius + 1; ++i)
   {
      this->xy_size_ += 4 * this->width_[i] + 2;
   }
}


// Set the  pnt.x and pnt.y based on the given index 
// into the circle grid
//
// @require index < size
void circle_grid::set_xy(coordinate & pnt, std::size_t index) const
{
UTILITY_REQUIRE( index < this->size(), "Index out of range" );
// adjust the xindex, yindex
size_t xindex( index );
std::size_t yindex = 0;
int yincrement = 1; // incremental value to move up and down the width array
while (xindex >= 2 * this->width_[yindex] + 1)
{
   // Detect end of an x row
   xindex -= 2 * this->width_[yindex] + 1;
   UTILITY_CHECK(yincrement == 1 or yindex >= 0, "Went off the end of width array");
   yindex += yincrement;
   if (yindex == this->width_.size())
   {
      // Detect half way through the xy slice
      yincrement = -1;
      yindex += yincrement;
   }
}

// set x,y coords in circle grid
pnt.x = this->spacing_ * (double(xindex) - double(this->width_[yindex]));
pnt.y = this->spacing_ * (-1.0 * double(yincrement) * double(yindex));

}

//  Minimum value the spacing can reach when attempting
//  to build a grid with a target number of positions.
const double tubular_grid::min_spacing = { 0.1 };

// Initialise the tubular grid with the given length and radius.
//
// This leaves a minimum of 'spacing' distance between any grid
// point and the cylinder edge. Point closest to origin
// will be (0,0,+/-spacing/2) or no points.
//
//\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
//\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
//\param spacing : No two grid points will be less than spacing apart and closer than spacing/2 to cylinder bounds.
tubular_grid::tubular_grid(double tubelength, double radius, double spacing)
: spacing_( spacing )
, zmax_( std::floor( tubelength/(2.0*spacing) ) * 2.0 )
, circle_( radius, spacing )
, selector_()	   
{
   // total number of grid points
   std::size_t size_ = this->circle_.size() * this->zmax_;

   // set up selector grid
   this->selector_.resize( size_ );

   for (std::size_t ii = 0; ii != size_; ++ii)
   {
      this->selector_[ii] = ii;
   }
}


// Initialise the tubular grid with the given length and radius. Vary grid spacing until
// we have approximately npart grid points.
//
// This leaves a minimum of 'spacing' distance between any grid
// point and the cylinder edge. Point closest to origin
// will be (0,0,+/-spacing/2) or no points.
//
//\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
//\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
//\param npart : Target number of grid points.
tubular_grid::tubular_grid(double tubelength, double radius, std::size_t npart)
: spacing_( 1.0 )
, zmax_( 0 )
, circle_( radius, 1.0 )
, selector_()	   
{
   // Need to adjust spacing until we get above npart
   const double volume{ core::constants::pi() * radius * radius * tubelength };
   double spacing{ std::cbrt( volume / npart ) };
   const double scale{ 0.95 };
   {
      const circle_grid circ( radius, spacing ); 
      UTILITY_CHECK(circ.size() * static_cast<std::size_t>( std::floor( (tubelength/(2.0*spacing)) ) * 2.0 ) < npart, "Initial guess spacing should always be too large");
   }
   while(true)
   {
      spacing *= scale;
      UTILITY_CHECK(spacing >= min_spacing, "Grid spacing below minimum spacing is probably an error");
      std::size_t zmax( std::floor( tubelength/(2.0*spacing) ) * 2.0 );
      circle_grid circ( radius, spacing ); 

      // total number of grid points
      std::size_t sz( circ.size() * zmax );
      // DEBUG : std::cout << "SPC " << spacing << " PART " << sz << " TARG " << npart << "\n";
      if ( sz >= npart )
      {
         this->spacing_ = spacing;
         this->zmax_ = zmax;
         this->circle_ = std::move( circ );
         break;
      }
   }

   // set up selector grid
   this->selector_.resize (this->zmax_ * this->circle_.size());

   for (std::size_t ii = 0; ii != this->selector_.size(); ++ii)
   {
      this->selector_[ii] = ii;
   }
}


// Initialise the tubular grid with the given length and radius.
//
// This leaves a minimum of 'spacing' distance between any grid
// point and the cylinder edge. Point closest to origin
// will be (0,0,+/-spacing/2) or no points.
//
//\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
//\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
//\param spacing : No two grid points will be less than spacing apart and closer than spacing/2 to cylinder bounds.
//\param shuffler : object to randomly reorder the access sequence.
tubular_grid::tubular_grid(double tubelength, double radius, double spacing, utility::random_distribution & shuffler)
: tubular_grid( tubelength, radius, spacing )
{
   // Randomise the selector entries
   shuffler.shuffle(selector_.begin(), selector_.end());
}


// Initialise the tubular grid with the given length and radius. Vary grid spacing until
// we have approximately npart grid points.
//
// This leaves a minimum of 'spacing' distance between any grid
// point and the cylinder edge. Point closest to origin
// will be (0,0,+/-spacing/2) or no points.
//
//\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
//\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
//\param npart : Target number of grid points.
//\param shuffler : object to randomly reorder the access sequence.
tubular_grid::tubular_grid(double tubelength, double radius, std::size_t npart, utility::random_distribution & shuffler)
: tubular_grid( tubelength, radius, npart )
{
   // Randomise the selector entries
   shuffler.shuffle(selector_.begin(), selector_.end());
}


tubular_grid::~tubular_grid() = default;

// Get the next location from the grid.
//
// @require size > 0
bool tubular_grid::next(coordinate & pnt) 
{
// Attempt to call next on empty grid
if (this->selector_.empty())
{
   return false;
}

// Get grid index from selector
std::size_t xyindex = this->selector_.back();

// Reduce xyindex by circle grid size until we get zindex
std::size_t zindex = 0;
while (xyindex >= this->circle_.size())
{
   xyindex -= this->circle_.size();
   ++zindex;
}
// create particle at current grid point
this->circle_.set_xy(pnt, xyindex);
pnt.z = this->spacing_ * (double(zindex) - double(this->zmax_/2) + 0.5);
// Pop used index off selector
this->selector_.pop_back();
return true;

}

// Create a cubic grid of points within a bisected tube of the given 
// half length, bisection offset and radius. NOTE: the outer extent
// of the tube is offset + length/2 in -ve and +ve z direction.
//
// NOTES:
//
// * Grid z coordinate will be either +ve or -ve
//
// * Z coordinate range offset < |z| < length/2 + offset
//
// * XY coordinates in circle of given radius
//
// * Sequence of coordinates generated by next are in random
// a order.
//

split_tube_grid::split_tube_grid(double offset, double length, double radius, std::size_t npart, utility::random_distribution & ranf)
: tubular_grid( length, radius, npart, ranf)
, zoffset_( offset )
{}


// Create a cubic grid of points within a bisected tube of the given 
// half length, bisection offset and radius.  NOTE: the outer extent
// of the tube is -/+(offset + length/2) in z direction.
//
// NOTES:
//
// * Grid z coordinate will be either +ve or -ve
//
// * Z coordinate range offset < |z| < length/2 + offset
//
// * XY coordinates in circle of given radius
//
// * Sequence of coordinates generated by next are sequential
// in z.
//

split_tube_grid::split_tube_grid(double offset, double length, double radius, std::size_t npart)
: tubular_grid( length, radius, npart )
, zoffset_( offset )
{}


// Create a cubic grid of points within a bisected tube of the given 
// half length, bisection offset and radius. NOTE: the outer extent
// of the tube is +/-(offset + length/2) in z direction.
//
// NOTES:
//
// * Grid z coordinate will be either +ve or -ve
//
// * Z coordinate range offset < |z| < length/2 + offset
//
// * XY coordinates in circle of given radius
//
// * Sequence of coordinates generated by next are in random
// a order.
//

split_tube_grid::split_tube_grid(double offset, double length, double radius, int npart, utility::random_distribution & ranf)
: tubular_grid( length, radius, npart, ranf)
, zoffset_( offset )
{}


// Create a cubic grid of points within a bisected tube of the given 
// half length, bisection offset and radius.  NOTE: the outer extent
// of the tube is +/-(offset + length/2) in z direction.
//
// NOTES:
//
// * Grid z coordinate will be either +ve or -ve
//
// * Z coordinate range offset < |z| < length/2 + offset
//
// * XY coordinates in circle of given radius
//
// * Sequence of coordinates generated by next are sequential
// in z.
//

split_tube_grid::split_tube_grid(double offset, double length, double radius, int npart)
: tubular_grid( length, radius, npart )
, zoffset_( offset )
{}


// Create a cubic grid of points within a bisected tube of the given 
// half length, bisection offset and radius. NOTE: the outer extent
// of the tube is +/-(offset + length/2) in z direction.
//
// NOTES:
//
// * Grid z coordinate will be either +ve or -ve
//
// * Z coordinate range offset < |z| < length/2 + offset
//
// * XY coordinates in circle of given radius
//
// * Sequence of coordinates generated by next are in random
// a order.
//

split_tube_grid::split_tube_grid(double offset, double length, double radius, double spacing, utility::random_distribution & ranf)
: tubular_grid( length, radius, spacing, ranf)
, zoffset_( offset )
{}


// Create a cubic grid of points within a bisected tube of the given 
// half length, bisection offset and radius.  NOTE: the outer extent
// of the tube is +/-(offset + length/2) in z direction.
//
// NOTES:
//
// * Grid z coordinate will be either +ve or -ve
//
// * Z coordinate range offset < |z| < length/2 + offset
//
// * XY coordinates in circle of given radius
//
// * Sequence of coordinates generated by next are sequential
// in z.
//

split_tube_grid::split_tube_grid(double offset, double length, double radius, double spacing)
: tubular_grid( length, radius, spacing )
, zoffset_( offset )
{}


// Get the next location from the grid.
//
// @require size > 0
bool split_tube_grid::next(coordinate & pnt) 
{
  const bool result{ this->tubular_grid::next( pnt ) };
  if (result) pnt.z += (pnt.z < 0.0 ? -this->zoffset_ : this->zoffset_);
  return result; 
}


} // namespace geometry

BOOST_CLASS_EXPORT_IMPLEMENT(geometry::tubular_grid);
BOOST_CLASS_EXPORT_IMPLEMENT(geometry::split_tube_grid);