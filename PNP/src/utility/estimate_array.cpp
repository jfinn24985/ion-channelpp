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

#include "utility/estimate_array.hpp"

// manual includes
#include "utility/utility.hpp"
// -
namespace utility {

void estimate_array::swap(estimate_array & other) 
{
  std::swap(this->means_, other.means_);
  std::swap(this->vars_, other.vars_);
  std::swap(this->count_, other.count_);
}

// Append an array of values data to the current estimator array
//
// \pre arr.size == size
void estimate_array::append(const std::vector< double > & arr)
{
  UTILITY_REQUIRE( arr.size() == this->size(), "Input data size does not match and array size" );
  ++this->count_;
  for( std::size_t idx = 0; idx != this->size(); ++idx )
  {
    mean_algorithm::append( this->means_[ idx ], this->vars_[ idx ], arr[ idx ], this->count_ );
  }
  

}

// Merge two estimate arrays of the same size. The indexing digitizers are assumed
// to be the same. The data in other is "moved" into this object.
//
// \pre size = other.size
// \post count = old.count + oldother.count and other.count = 0
void estimate_array::merge(estimate_array & other)
{
  UTILITY_CHECK( this->size() == other.size(), "Can not merge estimate arrays of different size." );
  std::size_t lhcount = 0;
  for( std::size_t idx = 0; idx != this->size(); ++idx )
  {
    lhcount = this->count_;
    mean_algorithm::merge( this->means_[ idx ], this->vars_[ idx ], lhcount, other.means_[ idx ], other.vars_[ idx ], other.count_ );
  }
  other.reset();
  this->count_ = lhcount;

}

// Merge two estimate arrays of the same size. The indexing digitizers are assumed
// to be the same. The data in other is "moved" into this object.
//
// \pre size = other.size
// \post count = old.count + oldother.count
void estimate_array::merge(estimate_array && other)
{
  UTILITY_CHECK( this->size() == other.size(), "Can not merge estimate arrays of different size." );
  std::size_t lhcount = 0;
  for( std::size_t idx = 0; idx != this->size(); ++idx )
  {
    lhcount = this->count_;
    mean_algorithm::merge( this->means_[ idx ], this->vars_[ idx ], lhcount, other.means_[ idx ], other.vars_[ idx ], other.count_ );
  }
  this->count_ = lhcount;

}

// The index of the array is usually mapped to some external
// field by a digitizer. The digitizer maps a range of the field
// into an index starting at zero. If a value in the field lies
// outside the digitizer's original range then the original index
// must be changed. It needs to be extended if the new value is
// above the original range. It needs to be remapped if the new
// value is below the original range.
//
// This method extends and if necessary moves data to allow the
// digitizer remaps its range. Note that this invalidates all
// subrange to index mappings that may have been saved. In other
// words the digitizer should always be used to access the array
// from a value in the field. Likewise copies of the digitizer
// also become invalid.
//
// This operation never loses data.
//
// \param extend : Extend indices beyond the current top of
//     the array. This does not move data.
//
// \param zero : Extend the indices below the current zero
//     of the array. This involves moving data.
//
// \post newsize = oldsize + extend + zero
//
// \post newindex = oldindex + zero

void estimate_array::reindex(std::size_t extend, std::size_t zero)
{
  const std::size_t add = extend + zero;
  UTILITY_REQUIRE( 0 != add, "Can not reindex with extend + zero == 0" );
  const std::size_t oldsize = this->means_.size();
  this->means_.resize( oldsize + add );
  this->vars_.resize( oldsize + add );
  if( 0 != zero and 0 != oldsize )
  {
    {
      auto first = this->means_.begin();
      auto last = first;
      auto result = first;
      std::advance( last, oldsize );
      std::advance( result, oldsize + zero );
      std::copy_backward( first, last, result );
      last = first;
      std::advance( last, zero );
      std::fill( first, last, 0.0 );
    }
    {
      auto first = this->vars_.begin();
      auto last = first;
      auto result = first;
      std::advance( last, oldsize );
      std::advance( result, oldsize + zero );
      std::copy_backward( first, last, result );
      last = first;
      std::advance( last, zero );
      std::fill( first, last, 0.0 );
    }
  }
  

}

// The index of the array is usually mapped to some external
// field by a digitizer. The digitizer maps a range of the field
// into an index starting at zero. If the digitizer realises that
// all values below a certain index are never accessed it may
// want to reindex the array be removing empty values at the
// beginning of the array. 
//
// \param zero : The index of an existing element that becomes
//     the new zeroth. All data between [ 0, zero ) is lost.
//
// \pre size > zero
//
// \post newsize = oldsize - zero
//
// \post newindex = oldindex - zero

void estimate_array::remove(std::size_t zero)
{
  UTILITY_REQUIRE( 0 != zero, "Can not remove with zero == 0" );
  const std::size_t oldsize = this->means_.size();
  UTILITY_REQUIRE( oldsize > zero, "Can not remove with zero =< size" );
  {
    auto result = this->means_.begin();
    auto first = result;
    auto last = this->means_.end();
    std::advance( first, zero );
    std::copy( first, last, result );
  }
  {
    auto result = this->vars_.begin();
    auto first = result;
    auto last = this->vars_.end();
    std::advance( first, zero );
    std::copy( first, last, result );
  }
  this->means_.resize( oldsize - zero );
  this->vars_.resize( oldsize - zero );

}

// Set all data to zero. The size of the array is unchanged.
//
// \post count = 0, mean[] === 0, var[] === 0, size = oldsize

void estimate_array::reset() 
{
  std::fill( this->means_.begin(), this->means_.end(), 0.0 );
  std::fill( this->vars_.begin(), this->vars_.end(), 0.0 );
  this->count_ = 0;

}


} // namespace utility
