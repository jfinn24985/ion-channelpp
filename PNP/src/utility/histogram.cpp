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

#include "utility/histogram.hpp"

// manual includes
#include "utility/fuzzy_equals.hpp"
// -
namespace utility {

// Construct object with the given range and bin width.
// Indicate if the histogram can automatically expand if the
// sampled range exceeds the expected range.
//
// \param minval, maxval : expected observation range.
//
// \param bincount : the number of bins in the range.
//
// \param extendable : whether to allow extending of range.

histogram::histogram(double minval, double maxval, double binwidth, bool extendable)
: converter_( minval, maxval, binwidth )
, data_( converter_.size() )
, sample_()
, extendable_( extendable )
, sampling_( false )
{}


// Construct object with the given range and desired number
// of bins.  Indicate if the histogram can automatically expand
// if the sampled range exceeds the expected range.
//
// \param minval, maxval : expected observation range.
//
// \param bincount : the number of bins in the range.
//
// \param extendable : whether to allow extending of range.

histogram::histogram(double minval, double maxval, std::size_t bincount, bool extendable)
: converter_( minval, maxval, bincount )
, data_( converter_.size() )
, sample_()
, extendable_( extendable )
, sampling_( false )
{}


// Construct object with the given range and desired number
// of bins.  Indicate if the histogram can automatically expand
// if the sampled range exceeds the expected range.
//
// \param minval, maxval : expected observation range.
//
// \param bincount : the number of bins in the range.
//
// \param extendable : whether to allow extending of range.

histogram::histogram(double minval, double maxval, int bincount, bool extendable)
: converter_( minval, maxval, bincount )
, data_( converter_.size() )
, sample_()
, extendable_( extendable )
, sampling_( false )
{}


// Initialize the sampling subobject.
//
// The correct way to use this method is in combination with
// sample_datum and end_sample. e.g.
// '''
//   begin_sample (call once)
//   ...
//   sample_datum (0,*) calls
//   ...
//   end_sample (call once)
// '''
//

void histogram::begin_sample()
{
  if( this->sample_.size() != this->data_.size() )
  {
    this->sample_.resize( this->data_.size() );
  }
  std::fill( this->sample_.begin(), this->sample_.end(), std::size_t( 0ul ) );
  this->sampling_ = true;

}

// Copy data from the sampling subobject to the estimate
// array object.
//
// The correct way to use this method is in combination with
// sample_datum and end_sample. e.g.
// '''
//   begin_sample (call once)
//   ...
//   sample_datum (0,*) calls
//   ...
//   end_sample (call once)
// '''
//

void histogram::end_sample()
{
  this->data_.append( this->sample_.begin(), this->sample_.end() );
  this->sampling_ = false;
}

// Extend the histogram to include the given value. This
// method is independent of is_auto_extendable.
//
// \pre not in_range( val )
void histogram::extend(double val)
{
  // Two options val > max or val < min
  if( val > this->maximum() )
  {
    // resize up
    const std::size_t oldsize = this->converter_.size();
    const std::size_t add = this->converter_.extend_up( val );
    UTILITY_CHECK( oldsize + add == this->converter_.size(), "Digitizer size change differs from advised change." );
    this->data_.resize( oldsize + add );
    this->sample_.resize( oldsize + add );
  }
  else if( val < this->minimum() )
  {
    const std::size_t oldsize = this->converter_.size();
    const std::size_t add = this->converter_.extend_down( val );
    UTILITY_CHECK( oldsize + add == this->converter_.size(), "Digitizer size change differs from advised change." );
    this->data_.reindex( 0, add );
    this->sample_.resize( oldsize + add );
    std::copy_backward( this->sample_.begin(), this->sample_.begin() + oldsize, this->sample_.end() );
    std::fill( this->sample_.begin(), this->sample_.begin() + add, 0.0 );
  }
  else if( val == this->maximum() )
  {
    // resize up
    const std::size_t oldsize = this->converter_.size();
    const std::size_t add = this->converter_.extend_up( val );
    UTILITY_CHECK( oldsize + add == this->converter_.size(), "Digitizer size change differs from advised change." );
    this->data_.resize( oldsize + add );
    this->sample_.resize( oldsize + add );
    UTILITY_CHECK( add != 0, "== should increase size." );
  }
  else
  {
    UTILITY_REQUIRE( this->in_range( val ), "Attend to extend range with value inside existing range." );
  }

}

// Merge two histograms. If the ranges of the digitizers are
// not the same but they have some equivalent bins, then the
// histograms are adjusted to give equivalent digitizers and
// data structures before merging.
//
// Returns false if merge was not possible.

bool histogram::merge(histogram & source)
{
  if( not this->converter_.equivalent( source.converter_ ) )
  {
    if( this->bin_width() != source.bin_width() )
    {
      return false;
    }
    if( not utility::feq( this->minimum(), source.minimum() ) )
    {
      // if this has smaller range, adjust this object.
      if( this->minimum() > source.minimum() )
      {
        const double diff = this->minimum() - source.minimum();
        if( not utility::feq( diff/this->bin_width(), std::nearbyint( diff/this->bin_width() ) ) )
        {
          return false;
        }
        this->extend( source.minimum() + this->bin_width() / 2.0 );
        UTILITY_CHECK( utility::feq( this->minimum(), source.minimum() ), "Should have the same minima" );
      }
      else
      {
        // need to alter source
        const double diff = source.minimum() - this->minimum();
        if( not utility::feq( diff/this->bin_width(), std::nearbyint( diff/this->bin_width() ) ) )
        {
          return false;
        }
        source.extend( this->minimum() + this->bin_width() / 2.0 );
        UTILITY_CHECK( utility::feq( this->minimum(), source.minimum() ), "Should have the same minima" );
      }
    }
    if( not utility::feq( this->maximum(), source.maximum() ) )
    {
      // if this has smaller range, adjust this object.
      if( this->maximum() < source.maximum() )
      {
        const double diff = source.maximum() - this->maximum();
        if( not utility::feq( diff/this->bin_width(), std::nearbyint( diff/this->bin_width() ) ) )
        {
          return false;
        }
        this->extend( source.maximum() - this->bin_width() / 2.0 );
        UTILITY_CHECK( utility::feq( this->maximum(), source.maximum() ), "Should have the same maxima" );
      }
      else
      {
        // need to alter source
        const double diff = this->maximum() - source.maximum();
        if( not utility::feq( diff/this->bin_width(), std::nearbyint( diff/this->bin_width() ) ) )
        {
          return false;
        }
        source.extend( this->maximum() - this->bin_width() / 2.0 );
        UTILITY_CHECK( utility::feq( this->maximum(), source.maximum() ), "Should have the same maxima" );
      }
    }
  }
  // should now be equivalent size
  UTILITY_CHECK( this->data_.size() == source.data_.size(), "Sizes should be the same!" );
  this->data_.merge( source.data_ );
  return true;

}

// Reset the objects state, size should remain constant
void histogram::reset()
{
  this->sampling_ = false;
  this->sample_.clear();
  this->data_.reset();

}

// Add one datum to the sampling subobject.
//
// The correct way to use this method is in combination with
// sample_datum and end_sample. e.g.
// '''
//   begin_sample (call once)
//   ...
//   sample_datum (0,*) calls
//   ...
//   end_sample (call once)
// '''
//

void histogram::sample_datum(double val)
{
  if( not this->converter_.in_range( val ) )
  {
    UTILITY_REQUIRE( this->extendable_, "Sampled value out of expected range." );
    this->extend( val );
  }
  ++this->sample_[ this->converter_.convert( val ) ];

}

// Releases and then resets the internal data.
estimate_array histogram::release_data()
{
  estimate_array output( this->data_.size() );
  output.swap( this->data_ );
  return std::move( output );
}


} // namespace utility
