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

#include "utility/basic_histogram.hpp"

namespace utility {

// Increase a sample bin count by adv.
//
// \pre is_sampling and idx < size
void basic_histogram::advance_bin_safe(std::size_t idx, std::size_t adv)
{
  UTILITY_REQUIRE( this->is_sampling(), "Cannot increment a bin when no sampling." );
  UTILITY_REQUIRE( idx < this->size(), "Index out of range." );
  this->advance_bin( idx, adv );
}
// Initialize the sampling subobject. This will reset any
// sample data if called while is_sampling.
//
// \post is_sampling OR size = 0
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

void basic_histogram::begin_sample()
{
  if( this->sample_.size() != this->data_.size() )
  {
    this->sample_.resize( this->data_.size() );
  }
  std::fill( this->sample_.begin(), this->sample_.end(), std::size_t( 0ul ) );

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

void basic_histogram::end_sample()
{
  if( this->sample_.size() == this->data_.size() )
  {
    this->data_.append( this->sample_.begin(), this->sample_.end() );
    this->sample_.clear();
  }
}
// Increment a sample bin
//
// \pre is_sampling and idx < size (checked)
void basic_histogram::increment_bin_safe(std::size_t idx)
{
  UTILITY_REQUIRE( this->is_sampling(), "Cannot increment a bin when no sampling." );
  UTILITY_REQUIRE( idx < this->size(), "Index out of range." );
  this->increment_bin( idx );
}

// Merge two histograms. Returns false if merge was not possible.
// It is not an error if either object is_sampling, although the
// current sample will not be included in the merge.
//
// \return size == source.size
// \post count = old.count + oldsource.count and source.count = 0
bool basic_histogram::merge(basic_histogram & source)
{
  if( this->size() == source.size() )
  {
    // same size, ok to merge
    this->data_.merge( source.data_ );
    return true;
  }
  return false;

}

// Merge two histograms. Returns false if merge was not possible.
// It is not an error if either object is_sampling, although the
// current sample will not be included in the merge.
//
// \return size == source.size
// \post count = old.count + oldsource.count and source.count = 0
bool basic_histogram::merge(basic_histogram && source)
{
  if( this->size() == source.size() )
  {
    // same size, ok to merge
    this->data_.merge( std::move( source.data_ ) );
    return true;
  }
  return false;

}

// Reset the objects state, size should remain constant. This
// will discard any current sampling data and set state to
// _not_ is_sampling. 
void basic_histogram::reset()
{
  this->sample_.clear();
  this->data_.reset();

}

// Change size of histogram. Increasing size initializes
// bew elements to zero, decreasing size discards data.
void basic_histogram::resize(std::size_t sz)
{
  if( this->data_.size() == this->sample_.size() )
  {
    this->sample_.resize( sz );
  }
  this->data_.resize( sz );
}

// The current value of a bin during sampling
//
// \pre is_sampling and idx < size
std::size_t basic_histogram::sample_count(std::size_t idx) const
{
  UTILITY_REQUIRE( this->is_sampling(), "Can only get sample count during sampling." );
  UTILITY_REQUIRE( this->size() > idx, "Index out of range." );
  return this->sample_[ idx ];
}

// Releases and then resets the internal data.
estimate_array basic_histogram::release_data()
{
  estimate_array output( this->data_.size() );
  output.swap( this->data_ );
  return std::move( output );
}


} // namespace utility
