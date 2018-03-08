#ifndef IONCH_UTILITY_FIXED_SIZE_HISTOGRAM_HPP
#define IONCH_UTILITY_FIXED_SIZE_HISTOGRAM_HPP

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

#include "utility/basic_histogram.hpp"
#include <cstddef>

namespace utility {

// A combination of digitizer and data array.
//
// Sampling can be done in one of two ways. If one has access to all
// the sample data at once, then the sample(iter, iter) method can be
// used. If sampling is done piecemeal, then begin_sample, 
// sample_datum and end_sample can be used.
template<class Digitizer>
class fixed_size_histogram : private basic_histogram
 {
   public:
    // The digitizer type.
    typedef Digitizer digitizer_type;

    // Data type of the digitizer
    typedef typename digitizer_type::data_type data_type;


   private:
    // Analog to digital converter. Converts between a data_type and an
    // index.
    digitizer_type converter_;


   public:
    class bin_iterator: public std::iterator<std::random_access_iterator_tag,double>
     {
       private:
        fixed_size_histogram<Digitizer> const* target_;

        // The current bin index
        std::ptrdiff_t index_;

       friend fixed_size_histogram< Digitizer >;


       protected:
        bin_iterator(fixed_size_histogram<Digitizer> const* targ, std::ptrdiff_t index)
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
        

        data_type minimum() const
        {
          return this->target_->bin_minimum( this->index_ );
        }

        data_type midpoint() const
        {
          return this->target_->bin_midpoint( this->index_ );
        }

        data_type maximum() const
        {
          return this->target_->bin_maximum( this->index_ );
        }

        data_type width() const
        {
          return this->target_->bin_width();
        }

        const bin_iterator* operator->() const
        {
          return this;
        }

        const bin_iterator& operator*() const
        {
          return *this;
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
    fixed_size_histogram()
    : basic_histogram()
    , converter_()
    {}

    // Construct object with the digitizer.
    
    fixed_size_histogram(const digitizer_type & converter)
    : basic_histogram( converter.size() )
    , converter_( converter )
    {}

    // Construct object with the digitizer.
    
    fixed_size_histogram(digitizer_type && converter)
    : basic_histogram( converter.size() )
    , converter_( std::move( converter ) )
    {}

    fixed_size_histogram(fixed_size_histogram<Digitizer> && source)
    : basic_histogram( std::move( source ) )
    , converter_( std::move( source.converter_ ) )
    {}
    fixed_size_histogram(const fixed_size_histogram<Digitizer> & source)
    : basic_histogram( source )
    , converter_( source.converter_ )
    {}

    ~fixed_size_histogram() = default;

    fixed_size_histogram<Digitizer> & operator=(fixed_size_histogram<Digitizer> source)
    {
      this->swap( source );
      return *this;
    }

    void swap(fixed_size_histogram<Digitizer> & source)
    {
      this->basic_histogram::swap( source );
      std::swap( this->converter_, source.converter_ );
    }
    


  friend class boost::serialization::access;
   private:
    template<class Archive> inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< basic_histogram >(*this);
      ar & converter_;
    };

   public:
    using basic_histogram::begin_sample;
    using basic_histogram::bin_mean;
    using basic_histogram::bin_variance;
    using basic_histogram::count;
    using basic_histogram::end_sample;
    using basic_histogram::is_sampling;
    using basic_histogram::reset;
    using basic_histogram::release_data;
    using basic_histogram::size;

   public:
    // Merge two histograms. If the ranges of the digitizers are
    // not the same but they have some equivalent bins, then the
    // histograms are adjusted to give equivalent digitizers and
    // data structures before merging.
    //
    // Returns false if merge was not possible.
    
    bool merge(fixed_size_histogram<Digitizer> & source)
    {
      if( this->converter_ == source.converter_ )
      {
        return this->basic_histogram::merge( source );
      }
      return false;
    }

    // Merge two histograms. If the ranges of the digitizers are
    // not the same but they have some equivalent bins, then the
    // histograms are adjusted to give equivalent digitizers and
    // data structures before merging.
    //
    // Returns false if merge was not possible.
    
    bool merge(fixed_size_histogram<Digitizer> && source)
    {
      if( this->converter_ == source.converter_ )
      {
        return this->basic_histogram::merge( std::move( source ) );
      }
      return false;
    }

    // Sample the given range of data. The arguments are only expected
    // to satisfy a model of the input iterator. If observable is not
    // auto_extendable then out-of-range values raise an exception.
    
    template< class InIter > inline void sample(InIter begin, InIter end)
    {
      this->begin_sample();
      for( ; begin != end; ++begin )
      {
        this->sample_datum( static_cast< data_type >( *begin ) );
      }
      this->end_sample();
    }
    // Sample the given range of data. The arguments are only expected
    // to satisfy a model of the input iterator. If observable is not
    // auto_extendable then out-of-range values raise an exception.
    
    template< class InIter > inline void sample_safe(InIter begin, InIter end)
    {
      this->begin_sample();
      for( ; begin != end; ++begin )
      {
        this->sample_datum_safe( static_cast< data_type >( *begin ) );
      }
      this->end_sample();
    }
    // Add one datum to the sampling subobject.
    //
    // \pre in_range( val )
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
    
    void sample_datum_safe(data_type val)
    {
      UTILITY_REQUIRE( this->converter_.in_range( val ), "Sampled value out of expected range." );
      this->increment_bin( this->converter_.convert( val ) );
    }

    // Add one datum to the sampling subobject.
    //
    // \pre in_range( val ) (NOT CHECKED)
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
    
    void sample_datum(data_type val)
    {
      this->increment_bin( this->converter_.convert( val ) );
    }

    // Maximum of the given bin (see digitizer::bin_maximum)
    data_type bin_maximum(std::size_t idx) const
    {
      return this->converter_.bin_maximum( idx );
    }

    // Midpoint of the given bin (see digitizer::bin_midpoint)
    data_type bin_midpoint(std::size_t idx) const
    {
      return this->converter_.bin_midpoint( idx );
    }

    // Minimum of the given bin (see digitizer::bin_minimum)
    data_type bin_minimum(std::size_t idx) const
    {
      return this->converter_.bin_minimum( idx );
    }

    // Width of a bin (see digitizer::width)
    data_type bin_width() const
    {
      return this->converter_.bin_width();
    }

    // Maximum of the data range (see digitizer::maximum)
    data_type maximum() const
    {
      return this->converter_.maximum();
    }

    // Minimum of the data range (see digitizer::minimum)
    data_type minimum() const
    {
      return this->converter_.minimum();
    }

    // Test if value is in current data range (see digitizer::in_range)
    bool in_range(data_type val) const
    {
      return this->converter_.in_range( val );
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

    const digitizer_type& axis() const
    {
      return this->converter_;
    }


};

} // namespace utility
#endif
