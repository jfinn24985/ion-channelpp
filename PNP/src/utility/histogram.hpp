#ifndef IONCH_UTILITY_HISTOGRAM_HPP
#define IONCH_UTILITY_HISTOGRAM_HPP

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

#include "utility/digitizer.hpp"
#include "utility/estimate_array.hpp"
#include <boost/serialization/vector.hpp>
#include <cstddef>

// manual includes
#include <iterator>
// -
namespace utility {

// A combination of digitizer and data array.
//
// Sampling can be done in one of two ways. If one has access to all
// the sample data at once, then the sample(iter, iter) method can be
// used. If sampling is done piecemeal, then begin_sample, 
// sample_datum and end_sample can be used.
class histogram
 {
   private:
    // Object that converts a continuous value into a digital index. 
    digitizer converter_;

    // The histogram of observation counts.
    estimate_array data_;

    // The counts of values being sampled (not archived, copied
    // or swapped).  This vector is used to record the bin counts
    // during sampling.  It is only used during the ||sample|| method
    // so is neither copied or archived.  Making it an attribute
    // allows it to be adjusted if the range needs to be extended
    // during sampling.
    
    std::vector<std::size_t> sample_;

    // Whether the histogram can automatically extend if the
    // sampled range exceeds the expected range.
    bool extendable_;

    // Is this histogram sampling?
    bool sampling_;


   public:
    class bin_iterator: public std::iterator<std::random_access_iterator_tag,double>
     {
       private:
        histogram const* target_;

        // The current bin index
        std::ptrdiff_t index_;

    friend histogram;
       protected:
        bin_iterator(histogram const* targ, std::ptrdiff_t index)
        : target_( targ )
        , index_( index )
        {}


       public:
        bin_iterator()
        : target_()
        , index_(0L)
        {}

        ~bin_iterator() {}

        bin_iterator(bin_iterator && source)
        : target_( std::move( source.target_ ) )
        , index_( std::move( source.index_ ) )
        {}

        bin_iterator(const bin_iterator & source)
        : target_( source.target_ )
        , index_( source.index_ )
        {}

        bin_iterator & operator=(bin_iterator source)
        {
          this->swap( source );
          return *this;
        }

        void swap(bin_iterator & other)
        {
          std::swap( this->target_, other.target_ );
          std::swap( this->index_, other.index_ );
        }


       private:
        void advance(std::ptrdiff_t diff)
        {
          this->index_ += diff;
        }

        void decrement()
        {
          --this->index_;
        }

        bool equivalent(const bin_iterator & other) const
        {
          return ( this->index_ == other.index_ && this->target_ == other.target_ );
        }

        // Establishes only partial ordering, as (a < b) == (b < a)
        // if a and b are not for the same histogram.
        
        bool less(const bin_iterator & other) const
        {
          return ( this->index_ < other.index_ && this->target_ == other.target_ );
        }

        // Establishes only partial ordering, as (a < b) == (b < a)
        // if a and b are not for the same histogram.
        
        bool less_eq(const bin_iterator & other) const
        {
          return ( this->index_ <= other.index_ && this->target_ == other.target_ );
        }

        void increment()
        {
          ++this->index_;
        }


       public:
        double mean() const
        {
          return this->target_->bin_mean( this->index_ );
        }

        double variance() const
        {
          return this->target_->bin_variance( this->index_ );
        }
        

        double minimum() const
        {
          return this->target_->bin_minimum( this->index_ );
        }

        double midpoint() const
        {
          return this->target_->bin_midpoint( this->index_ );
        }

        double maximum() const
        {
          return this->target_->bin_maximum( this->index_ );
        }

        double width() const
        {
          return this->target_->bin_width();
        }

        const double operator*() const
        {
          return this->mean();
        }

        bin_iterator & operator+=(std::ptrdiff_t diff)
        {
          this->advance( diff );
          return *this;
        }

        bin_iterator & operator-=(std::ptrdiff_t diff)
        {
          this->advance( -diff );
          return *this;
        }

        bin_iterator operator--(int i)
        {
          bin_iterator tmp(*this);
          this->decrement();
          return tmp;
        }

        bin_iterator & operator--()
        {
          this->decrement();
          return *this;
        }

        bin_iterator operator++(int i)
        {
          bin_iterator tmp(*this);
          this->increment();
          return tmp;
        }

        bin_iterator & operator++()
        {
          this->increment();
          return *this;
        }

        friend inline bool operator==(const bin_iterator & lhs, const bin_iterator & rhs)
        {
          return lhs.equivalent( rhs );
        }
        

        friend inline bool operator!=(const bin_iterator & lhs, const bin_iterator & rhs)
        {
          return not lhs.equivalent( rhs );
        }
        

        // Establishes only partial ordering, as (a < b) == (b < a)
        // if a and b are not for the same histogram.
        
        friend inline bool operator<(const bin_iterator & lhs, const bin_iterator & rhs)
        {
          return lhs.less( rhs );
        }
        

        // Establishes only partial ordering, as (a < b) == (b < a)
        // if a and b are not for the same histogram.
        
        friend inline bool operator<=(const bin_iterator & lhs, const bin_iterator & rhs)
        {
          return lhs.less_eq( rhs );
        }
        

        // Establishes only partial ordering, as (a < b) == (b < a)
        // if a and b are not for the same histogram.
        
        friend inline bool operator>(const bin_iterator & lhs, const bin_iterator & rhs)
        {
          return rhs.less( lhs );
        }
        

        // Establishes only partial ordering, as (a < b) == (b < a)
        // if a and b are not for the same histogram.
        
        friend inline bool operator>=(const bin_iterator & lhs, const bin_iterator & rhs)
        {
          return rhs.less_eq( lhs );
        }
        

        friend inline bin_iterator operator+(std::ptrdiff_t diff, bin_iterator iter)
        {
          iter.advance( diff );
          return iter;
        }

        friend inline bin_iterator operator+(bin_iterator iter, std::ptrdiff_t diff)
        {
          iter.advance( diff );
          return iter;
        }

        friend inline bin_iterator operator-(std::ptrdiff_t diff, bin_iterator iter)
        {
          iter.advance( -diff );
          return iter;
        }

        friend inline bin_iterator operator-(bin_iterator iter, std::ptrdiff_t diff)
        {
          iter.advance( -diff );
          return iter;
        }

        // This doesn't check to see if the two iterators are for
        // the same histogram.
        friend inline std::ptrdiff_t operator-(const bin_iterator & lhs, const bin_iterator & rhs)
        {
          return lhs.index_ - rhs.index_;
        }


    };
    
    // For serialization
    histogram()
    : converter_()
    , data_()
    , sample_()
    , extendable_()
    , sampling_( false )
    {}

    // Construct object with the given range and bin width.
    // Indicate if the histogram can automatically expand if the
    // sampled range exceeds the expected range.
    //
    // \param minval, maxval : expected observation range.
    //
    // \param bincount : the number of bins in the range.
    //
    // \param extendable : whether to allow extending of range.
    
    histogram(double minval, double maxval, double binwidth, bool extendable = false);

    // Construct object with the given range and desired number
    // of bins.  Indicate if the histogram can automatically expand
    // if the sampled range exceeds the expected range.
    //
    // \param minval, maxval : expected observation range.
    //
    // \param bincount : the number of bins in the range.
    //
    // \param extendable : whether to allow extending of range.
    
    histogram(double minval, double maxval, std::size_t bincount, bool extendable = false);

    // Construct object with the given range and desired number
    // of bins.  Indicate if the histogram can automatically expand
    // if the sampled range exceeds the expected range.
    //
    // \param minval, maxval : expected observation range.
    //
    // \param bincount : the number of bins in the range.
    //
    // \param extendable : whether to allow extending of range.
    
    histogram(double minval, double maxval, int bincount, bool extendable = false);

    histogram(histogram && source)
    : converter_( std::move( source.converter_ ) )
    , data_( std::move( source.data_ ) )
    , sample_()
    , extendable_( std::move( source.extendable_ ) )
    , sampling_( std::move( source.sampling_ ) )
    {}
    histogram(const histogram & source)
    : converter_( source.converter_ )
    , data_( source.data_ )
    , sample_()
    , extendable_( source.extendable_ )
    , sampling_( source.sampling_ )
    {}

    ~histogram() = default;

    histogram & operator=(histogram source)
    {
      this->swap( source );
      return *this;
    }

    void swap(histogram & source)
    {
      std::swap( this->converter_, source.converter_ );
      std::swap( this->data_, source.data_ );
      std::swap( this->extendable_, source.extendable_ );
      std::swap( this->sampling_, source.sampling_ );
    }
    


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & converter_;
        ar & data_;
        ar & extendable_;
        ar & sampling_;
      };


   public:
    bool auto_extendable() const
    {
      return this->extendable_;
    }

    void auto_extendable(bool doit)
    {
      this->extendable_ = doit;
    }

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

    // Extend the histogram to include the given value. This
    // method is independent of is_auto_extendable.
    //
    // \pre not in_range( val )
    void extend(double val);

    // Is this in sampling phase?
    bool is_sampling() const
    {
      return this->sampling_;
    }
    // Merge two histograms. If the ranges of the digitizers are
    // not the same but they have some equivalent bins, then the
    // histograms are adjusted to give equivalent digitizers and
    // data structures before merging.
    //
    // Returns false if merge was not possible.
    
    bool merge(histogram & source);

    // Reset the objects state, size should remain constant
    void reset();

    // Sample the given range of data. The arguments are only expected
    // to satisfy a model of the input iterator. If observable is not
    // auto_extendable then out-of-range values raise an exception.
    
    template< class InIter > inline void sample(InIter begin, InIter end)
    {
      this->begin_sample();
      for( ; begin != end; ++begin )
      {
        this->sample_datum( static_cast< double >( *begin ) );
      }
      this->end_sample();
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
    
    void sample_datum(double val);

    // Maximum of the given bin (see digitizer::bin_maximum)
    double bin_maximum(std::size_t idx) const
    {
      return this->converter_.bin_maximum( idx );
    }

    // Midpoint of the given bin (see digitizer::bin_midpoint)
    double bin_midpoint(std::size_t idx) const
    {
      return this->converter_.bin_midpoint( idx );
    }

    // Minimum of the given bin (see digitizer::bin_minimum)
    double bin_minimum(std::size_t idx) const
    {
      return this->converter_.bin_minimum( idx );
    }

    // Width of a bin (see digitizer::width)
    double bin_width() const
    {
      return this->converter_.bin_width();
    }

    // Mean of the given bin (see estimater_array::mean)
    double bin_mean(std::size_t idx) const
    {
      return this->data_.mean( idx );
    }

    // Variance of the given bin (see estimater_array::variance)
    double bin_variance(std::size_t idx) const
    {
      return this->data_.variance( idx );
    }

    // Count of the number of samples (see estimater_array::count)
    std::size_t count() const
    {
      return this->data_.count();
    }

    // Maximum of the data range (see digitizer::maximum)
    double maximum() const
    {
      return this->converter_.maximum();
    }

    // Minimum of the data range (see digitizer::minimum)
    double minimum() const
    {
      return this->converter_.minimum();
    }

    // Test if value is in current data range (see digitizer::in_range)
    bool in_range(double val) const
    {
      return this->converter_.in_range( val );
    }
    

    // Number of bins (see digitizer::size and estimater_array::size)
    std::size_t size() const
    {
      return this->converter_.size();
    }

    // Iterator to access the first bin
    bin_iterator begin() const
    {
      return bin_iterator( this, 0 );
    }

    // Iterator to the one-past-last bin
    bin_iterator end() const
    {
      return bin_iterator( this, this->size() );
    }

    // Releases and then resets the internal data.
    estimate_array release_data();

    const digitizer& axis() const
    {
      return this->converter_;
    }


};

} // namespace utility
#endif
