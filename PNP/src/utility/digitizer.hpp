#ifndef IONCH_UTILITY_DIGITIZER_HPP
#define IONCH_UTILITY_DIGITIZER_HPP

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

#include "utility/archive.hpp"

#include <cstddef>

#include "utility/utility.hpp"
namespace utility {

// Convert a real valued type in range min/max into a set of digitized
// integers from 0 to size. Values outside the range min/max have undefined
// values.  (See open_digitizer for class where values outside range map
// to well-defined values.)
//
// Range to histogram map definition
//
// Define a mapping of a continuous range [X0,XN) into a discrete histogram
// bin range [0, size). Therefore a value in range (same for bins) if 
// minimum <= value < maximum
//
// ENSURE minimum < maximum
// ENSURE bin count > 0

class digitizer
 {
   public:
    // The expected type to convert to digital integer.
    typedef double data_type;


   private:
    //The lowest real value to digitise (digital values below this are undefined)
    double minimum_;

    // The width of each bin
    double step_;

    // The distance between the minimum and maximum
    // points on the axis
    double width_;

    // The number of bins on this axis
    std::size_t size_;


   public:
    // Default constructor
    digitizer()
    : minimum_()
    , step_()
    , width_()
    , size_()
    {}

    //Create a digitizer from the range and the number of bins.
    //
    //Object is invalid if acount == 0
    //
    //Integers will go from 0 near amin to size near amax.
    //Note that amin > amax is well-defined. 
    digitizer(double aminimum, double amaximum, std::size_t acount);

    //Create a digitizer from the range and the number of bins. Overload
    // to allow integers (mainly literals) to be used in place of size_ts.
    //
    //Object is invalid if acount <= 0
    //
    //Integers will go from 0 near amin to size near amax.
    //Note that amin > amax is well-defined. 
    digitizer(double aminimum, double amaximum, int acount);

    // Create a histogram from a range and the desired bin width.
    //
    // NOTE: axis maximum will be increased so that a whole number of
    // bins of the given width fits in the range.
    digitizer(double aminimum, double amaximum, double abinwidth);

    digitizer(digitizer && source)
    : minimum_( std::move( source.minimum_ ) )
    , step_( std::move( source.step_ ) )
    , width_( std::move( source.width_ ) )
    , size_( std::move( source.size_ ) )
    {}

    digitizer(const digitizer & source)
    : minimum_( source.minimum_ )
    , step_( source.step_ )
    , width_( source.width_ )
    , size_( source.size_ )
    {}

    digitizer & operator=(digitizer source)
    {
      this->swap( source );
      return *this;
    }

    void swap(digitizer & source)
    {
      std::swap( this->minimum_, source.minimum_ );
      std::swap( this->step_, source.step_ );
      std::swap( this->width_, source.width_ );
      std::swap( this->size_, source.size_ );
    }


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & minimum_;
        ar & width_;
        ar & step_;
        ar & size_;
      };


   public:
    // The end of a particular bin
    inline double bin_maximum(std::size_t bin) const
    {
      return this->minimum_ + (bin + 1) * this->step_;
    }

    // The mid-point of a particular bin
    //
    // @require bin < size()
    double bin_midpoint(std::size_t bin) const
    {
      return this->minimum_ + bin * this->step_ + this->step_ / 2;
    }

    // The beginning of a particular bin
    //
    // @require bin < size()
    double bin_minimum(std::size_t bin) const
    {
      return this->minimum_ + bin * this->step_;
    }

    // The width of each bin
    double bin_width() const
    {
      return this->step_;
    }

    // The bin number a value maps on to.
    //
    //Note: mapping uses half closed intervals
    //
    //bin_minimum(x) <= r < bin_maximum(x), 
    //therefore bin number is x+1 when r == bin_maximum(x)
    inline std::size_t convert(double val) const
    {
      UTILITY_REQUIRE(this->in_range(val), "Target value outside of range.");
      return std::min(this->size()
                      , std::size_t(std::floor((val - this->minimum_)/this->step_)));
    }

    bool equivalent(const digitizer & axis) const
    {
      return this->minimum_ == axis.minimum_ and
        this->step_ == axis.step_ and
        this->width_ == axis.width_ and
        this->size_ == axis.size_;
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
    std::size_t extend_up(double maxval);

    // Extend the allowable range below the current minimum. The new
    // minimum will be minval rounded up to the next bin edge. The
    // number of new bins is returned.
    // 
    // \pre minval < minimum
    //
    // \return number of new bins
    //
    // \post minimum <= minval
    
    std::size_t extend_down(double minval);

    // Check is value is in the histogram range
    //
    //true if minimum < val <= maximum
    bool in_range(double val) const
    {
      return (this->minimum_ <= val) and (val < this->minimum_ + this->width_);
    }

    // The end of the histogram range
    double maximum() const
    {
      return this->minimum_ + this->width_;
    }

    // The beginning of the histogram range
    double minimum() const
    {
      return this->minimum_;
    }

    friend inline bool operator==(const digitizer & lhs, const digitizer & rhs)
    {
      return lhs.equivalent( rhs );
    }

    friend inline bool operator!=(const digitizer & lhs, const digitizer & rhs)
    {
      return not lhs.equivalent( rhs );
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
    
    std::size_t shrink_up(double maxval);

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
    
    std::size_t shrink_down(double minval);

    // The number of bins in the range
    inline std::size_t size() const {
        return this->size_;
      };

    // The total width of the histogram range
    //
    // width = maximum - minimum
    double width() const
    {
      return this->width_;
    }


};

} // namespace utility
#endif
