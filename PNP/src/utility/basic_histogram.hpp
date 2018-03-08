#ifndef IONCH_UTILITY_BASIC_HISTOGRAM_HPP
#define IONCH_UTILITY_BASIC_HISTOGRAM_HPP

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

#include "utility/estimate_array.hpp"
#include <boost/serialization/vector.hpp>
#include <cstddef>

namespace utility {

// A combination of fixed-size data array and sampler, independent of the
// converter/digitizer used. This is intended to be used as _private_ base
// class (e.g. of template histograph class) and is not defined with virtual
// functions. It is intended to allow canonical derived classes.
//
// Sampling is done piecemeal, by first calling begin_sample, then
// increment_bin for each datum and finishing with end_sample.
class basic_histogram
 {
   private:
    // The histogram of observation counts.
    estimate_array data_;

    // The counts of values being sampled (not archived, copied
    // or swapped).  This vector is used to record the bin counts
    // during sampling.  It is only used during the ||sample|| method
    // so is neither copied or archived.  Making it an attribute
    // allows it to be adjusted if the range needs to be extended
    // during sampling.
    //
    // The basic_histogram "is_sampling" if sample_.size = data_.size
    
    std::vector<std::size_t> sample_;


   public:
    // For serialization
    basic_histogram()
    : data_()
    , sample_()
    {}

    // Create histogram of the given size.
    basic_histogram(std::size_t sz)
    : data_( sz )
    , sample_()
    {}

    basic_histogram(basic_histogram && source)
    : data_( std::move( source.data_ ) )
    , sample_( std::move( source.sample_ ) )
    {}
    basic_histogram(const basic_histogram & source)
    : data_( source.data_ )
    , sample_( source.sample_ )
    {}

    ~basic_histogram() = default;

    basic_histogram & operator=(basic_histogram source)
    {
      this->swap( source );
      return *this;
    }

    void swap(basic_histogram & source)
    {
      std::swap( this->data_, source.data_ );
      std::swap( this->sample_, source.sample_ );
    }
    


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & data_;
        ar & sample_;
      };


   public:
    // Increase a sample bin count by adv.
    //
    // \pre is_sampling and idx < size (not checked)
    void advance_bin(std::size_t idx, std::size_t adv)
    {
      this->sample_[ idx ] += adv;
    }

    // Increase a sample bin count by adv.
    //
    // \pre is_sampling and idx < size
    void advance_bin_safe(std::size_t idx, std::size_t adv);

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
    
    void begin_sample();

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
    
    void end_sample();

    // Increment a sample bin
    //
    // \pre is_sampling and idx < size (not checked)
    void increment_bin(std::size_t idx)
    {
      ++this->sample_[ idx ];
    }
    // Increment a sample bin
    //
    // \pre is_sampling and idx < size (checked)
    void increment_bin_safe(std::size_t idx);

    // Is this in sampling phase?
    bool is_sampling() const
    {
      return (not this->data_.empty()) and (this->sample_.size() == this->data_.size());
    }
    // Merge two histograms. Returns false if merge was not possible.
    // It is not an error if either object is_sampling, although the
    // current sample will not be included in the merge.
    //
    // \return size == source.size
    // \post count = old.count + oldsource.count and source.count = 0
    bool merge(basic_histogram & source);

    // Merge two histograms. Returns false if merge was not possible.
    // It is not an error if either object is_sampling, although the
    // current sample will not be included in the merge.
    //
    // \return size == source.size
    // \post count = old.count + oldsource.count and source.count = 0
    bool merge(basic_histogram && source);

    // Reset the objects state, size should remain constant. This
    // will discard any current sampling data and set state to
    // _not_ is_sampling. 
    void reset();

    // Change size of histogram. Increasing size initializes
    // bew elements to zero, decreasing size discards data.
    void resize(std::size_t sz);

    // Mean of the given bin (see estimater_array::mean). This excludes
    // any data currently being sampled.
    //
    // \pre idx < size (tested if DEBUG > 0 only)
    double bin_mean(std::size_t idx) const
    {
      return this->data_.mean( idx );
    }

    // Variance of the given bin (see estimater_array::variance). This excludes
    // any data currently being sampled.
    //
    // \pre idx < size (tested if DEBUG > 0 only)
    double bin_variance(std::size_t idx) const
    {
      return this->data_.variance( idx );
    }

    // Count of the number of samples (see estimater_array::count). This excludes
    // any data currently being sampled.
    std::size_t count() const
    {
      return this->data_.count();
    }

    // The current value of a bin during sampling
    //
    // \pre is_sampling and idx < size
    std::size_t sample_count(std::size_t idx) const;

    // Number of bins (see digitizer::size and estimater_array::size)
    std::size_t size() const
    {
      return this->data_.size();
    }

    // Releases and then resets the internal data.
    estimate_array release_data();


};

} // namespace utility
#endif
