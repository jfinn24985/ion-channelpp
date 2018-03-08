//----------------------------------------------------------------------
//DATA-SET SUMMARY/REDUCTION CLASSES
//
//These classes provide classes to manage statistical sampling such
//as histograms and single value estimates and to calculate mean and
//variance of the estimations.
//
//Authors: Justin Finnerty
//
//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or (at
//your option) any later version.
//
//This program is distributed in the hope that it will be useful, but
//WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------


#ifndef DEBUG
#define DEBUG 0
#endif

#include "utility/digitizer.hpp"

// manual includes
#include "utility/fuzzy_equals.hpp"
// -
#include <cmath>
// -
namespace utility {

//Create a digitizer from the range and the number of bins.
//
//Object is invalid if acount == 0
//
//Integers will go from 0 near amin to size near amax.
//Note that amin > amax is well-defined. 
digitizer::digitizer(double aminimum, double amaximum, std::size_t acount)
: minimum_(std::min(aminimum,amaximum))
, step_(acount > 0 ? std::abs(amaximum - aminimum)/double(acount) : 0)
, width_(std::abs(amaximum - aminimum))
, size_(acount)
{
  UTILITY_REQUIRE(acount > 0, "Can not have zero bin number");
  UTILITY_REQUIRE(amaximum > aminimum, "Minimum of range must be less than the maximum");
}


//Create a digitizer from the range and the number of bins. Overload
// to allow integers (mainly literals) to be used in place of size_ts.
//
//Object is invalid if acount <= 0
//
//Integers will go from 0 near amin to size near amax.
//Note that amin > amax is well-defined. 
digitizer::digitizer(double aminimum, double amaximum, int acount)
: digitizer(aminimum, amaximum, std::size_t(acount))
{
  UTILITY_REQUIRE(acount > 0, "Can not have negative bin number");
}

// Create a histogram from a range and the desired bin width.
//
// NOTE: axis maximum will be increased so that a whole number of
// bins of the given width fits in the range.
digitizer::digitizer(double aminimum, double amaximum, double abinwidth)
: minimum_(std::min(aminimum,amaximum))
, step_(std::abs(abinwidth))
, width_(std::abs(amaximum - aminimum))
, size_()
{
  UTILITY_REQUIRE(amaximum > aminimum, "Minimum of range must be less than the maximum");
  UTILITY_REQUIRE(abinwidth > 0.0, "Step size must be greater than zero");
  size_ = std::max(1ul, std::size_t(std::ceil(width_/step_)));
  // MINIMUM SIZE IS 1
  if( size_ > 1ul )
  {
    // Check size
    // CASE 1: size_ is 1 too large: step*(size - 1 ) > width
    while( width_ <= ((step_ * size_) - step_) )
    {
      // Check if method accidentally gave a slightly
      // higher bin number
      --size_;
      UTILITY_REQUIRE(size_ > 0, "Number of bins must be greater than zero");
    }
    // CASE 2: FEQ(step*(size-1), width)
    if( utility::feq( width_, (step_ * size_) - step_ ) )
    {
      // Check if method accidentally gave a slightly
      // higher bin number
      --size_;
      UTILITY_REQUIRE(size_ > 0, "Number of bins must be greater than zero");
    }  
  }
  // Adjust width to match step and size.
  width_ = step_ * size_;
}


// Extend the allowable range above the current maximum. The new
// maximum will be maxval rounded up to the next bin edge. The
// number of new bins is returned.
// 
// \pre maxval > maximum
//
// \return number of new bins
//
// \post maximum >= maxval
std::size_t digitizer::extend_up(double maxval)
{
  UTILITY_REQUIRE( maxval >= this->maximum(), "extend_up can not reduce range." );
  const std::size_t add = ( maxval == this->maximum() ? 1ul : std::size_t( std::ceil( ( maxval - this->maximum() ) / this->bin_width() ) ) );
  const double change = add * this->step_;
  this->width_ += change;
  this->size_ += add;
  return add;

}

// Extend the allowable range below the current minimum. The new
// minimum will be minval rounded up to the next bin edge. The
// number of new bins is returned.
// 
// \pre minval < minimum
//
// \return number of new bins
//
// \post minimum <= minval

std::size_t digitizer::extend_down(double minval)
{
  UTILITY_REQUIRE( minval < this->minimum(), "extend_down can not reduce range." );
  const std::size_t add = std::ceil( ( this->minimum() - minval ) / this->bin_width() );
  const double change = add * this->bin_width();
  this->minimum_ -= change;
  this->width_ += change;
  this->size_ += add;
  return add;

}

// Decrease the allowable range to below the current maximum. The
// new maximum will be maxval rounded up to the next bin edge. The
// number of new bins removed is returned.  If maxval is greater
// than bin_minimum( size -1 ) then this will result in no change
// and zero will be returned. At least one bin must remain.
//
// \pre bin_maximum( 0 ) <= maxval <= maximum
//
// \return number of new bins removed : [ 0, size ]
//
// \post maximum >= maxval and minimum != maximum

std::size_t digitizer::shrink_up(double maxval)
{
  UTILITY_REQUIRE( maxval < this->maximum(), "shrink_up can not extend range." );
  const std::size_t del = std::floor( ( this->maximum() - maxval ) / this->bin_width() );
  UTILITY_REQUIRE( del < this->size(), "shrink_up can not clear range." );
  if( del > 0 )
  {
    const double change = del * this->step_;
    this->width_ -= change;
    this->size_ -= del;
  }
  return del;
  

}

// Decrease the allowable range to above the current minimum. The
// new minimum will be minval rounded down to the next bin edge. The
// number of new bins removed is returned.  If minval is less
// than bin_maximum( 0 ) then this will result in no change
// and zero will be returned. At least one bin must remain.
//
// \pre minimum < minval <= bin_minimum( size - 1 )
//
// \return number of new bins removed : [ 0, size ]
//
// \post minimum <= minval and minimum != maximum

std::size_t digitizer::shrink_down(double minval)
{
  UTILITY_REQUIRE( minval > this->minimum(), "shrink_down can not extend range." );
  const std::size_t del = std::floor( ( minval - this->minimum() ) / this->bin_width() );
  UTILITY_REQUIRE( del < this->size(), "shrink_down can not clear range." );
  if( del > 0 )
  {
    const double change = del * this->step_;
    this->minimum_ += change;
    this->width_ -= change;
    this->size_ -= del;
  }
  return del;
  

}


} // namespace utility
