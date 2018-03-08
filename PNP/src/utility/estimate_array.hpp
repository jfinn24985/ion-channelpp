#ifndef IONCH_UTILITY_ESTIMATE_ARRAY_HPP
#define IONCH_UTILITY_ESTIMATE_ARRAY_HPP

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

#include "utility/mean_algorithm.hpp"
#include <boost/serialization/vector.hpp>
#include <cstddef>

// manual includes
#include "utility/utility.hpp"
// -
namespace utility {

// Manage mean and (biased) variance of a 1D set of data
//
// (TODO: Unit test suite "estimate_test_suite")
//
// This array is meant to be used with an digitizer object that
// maps a range of a field into a digital range. It provides
// methods to extend (||resize|| and ||reindex||) the array if
// the digitizer needs to record values outside the originally
// expected range. It also provides methods to reduce (||resize||
// and ||remove||) the array if the recorded values lie in
// a subset of the originally expected range.  ||reindex||
// can add places at the beginning and/or end of the original
// range. ||resize|| can extend or remove data from the top of
// the original range. ||remove|| erases data at the beginning
// of the original range. As ||reindex|| and ||remove|| take or
// add data to the beginning of the array the digitizer object
// must be adjusted to maintain the correct mapping from field
// range to digital range.

class estimate_array : private mean_algorithm
 {
   private:
    // Array of means
    std::vector<double> means_;

    // Array of variance data
    std::vector<double> vars_;

    std::size_t count_;


   public:
    estimate_array() : means_(), vars_(), count_() {};

    estimate_array(std::size_t sz) : means_(sz), vars_(sz), count_() {}

    estimate_array(const estimate_array & source)
    : means_(source.means_)
    , vars_(source.vars_)
    , count_(source.count_)
    {}

    estimate_array(estimate_array && source)
    : means_( std::move( source.means_ ) )
    , vars_( std::move( source.vars_ ) )
    , count_( std::move( source.count_ ) )
    {}

    ~estimate_array() = default;

    estimate_array & operator=(estimate_array source)
    {
      this->swap(source);
      return *this;
    }


  friend class boost::serialization::access;


   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & means_; ar & vars_; ar & count_;
      }


   public:
    void swap(estimate_array & other);

    // Append an array of data values to the current estimator array
    //
    // distance(begin,end) < 0 is undefined
    //
    // \pre distance(begin, end) == size : this is tested at end of
    // method, so on error the array will be in an undefined state.
    template<typename IterType>
      void append(IterType begin, IterType end)
    {
      ++this->count_;
      std::size_t idx = 0;
      for( idx = 0; idx != this->size() and begin != end; ++idx, ++begin )
      {
        mean_algorithm::append( this->means_[ idx ], this->vars_[ idx ], *begin, this->count_ );
      }
      UTILITY_REQUIRE( begin == end and idx == this->size(), "Input data size does not match and array size" );
    }

    // Append an array of values data to the current estimator array
    //
    // distance(begin,end) < 0 is undefined
    //
    // \pre distance(begin, end) == size : this is tested before the
    // array is updated providing a strong exception safety guarrantee.
    template<typename IterType>
      void append_safe(IterType begin, IterType end)
    {
      std::vector< double > tmp( begin, end );
      append( tmp );
    }

    // Append an array of values data to the current estimator array
    //
    // \pre arr.size == size
    void append(const std::vector< double > & arr);

    // Remove all elements of the array. All data is lost.
    //
    // \post size = 0. count = 0
    
    void clear()
    {
      this->count_ = 0;
      this->means_.clear(); this->vars_.clear();
    }

    // The number of sample cycles.
    std::size_t count() const
    {
      return this->count_;
    }

    // Are there any elements?
    bool empty() const
    {
      return this->means_.empty();
    }

    // The geometric means of the samples at a given index
    double mean(std::size_t idx) const
    {
#if defined DEBUG && DEBUG != 0
      return this->means_.at( idx );
#else
      return this->means_[ idx ];
#endif
    }
    // Merge two estimate arrays of the same size. The indexing digitizers are assumed
    // to be the same. The data in other is "moved" into this object.
    //
    // \pre size = other.size
    // \post count = old.count + oldother.count and other.count = 0
    void merge(estimate_array & other);

    // Merge two estimate arrays of the same size. The indexing digitizers are assumed
    // to be the same. The data in other is "moved" into this object.
    //
    // \pre size = other.size
    // \post count = old.count + oldother.count
    void merge(estimate_array && other);

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
    
    void reindex(std::size_t extend, std::size_t zero);

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
    
    void remove(std::size_t zero);

    // The index of the array is usually mapped to some external
    // field by a digitizer. The digitizer maps a range of the field
    // into an index starting at zero. If the digitizer realises that
    // all values above a certain index are never accessed it may
    // want to remove the empty values at the end of the array if
    // sz is less than size. Alternatively it may want to extend the
    // array if sz is greater than sz.
    //
    // \param sz : The index of an existing element that becomes
    //     the new last index. For sz < size all data between 
    //     [ sz, size ) is lost. For sz > size all data
    //     [ oldsize, size ) === 0.
    //
    // \pre size != sz
    //
    // \post newsize = sz
    //
    // \post newindex = oldindex
    
    void resize(std::size_t sz)
    {
      this->means_.resize( sz ); this->vars_.resize( sz );
    }

    // Set all data to zero. The size of the array is unchanged.
    //
    // \post count = 0, mean[] === 0, var[] === 0, size = oldsize
    
    void reset();

    // The number of elements
    std::size_t size() const
    {
      return this->means_.size();
    }

    // The variance at element idx. var = 0 if count < 2
    double variance(std::size_t idx) const
    {
#if defined DEBUG && DEBUG != 0
      return mean_algorithm::variance( this->vars_.at( idx ), this->count_ );
#else
      return mean_algorithm::variance( this->vars_[idx], this->count_ );
#endif
    }


};

} // namespace utility
#endif
