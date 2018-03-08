#ifndef IONCH_UTILITY_ESTIMATER_HPP
#define IONCH_UTILITY_ESTIMATER_HPP

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

// Manual includes
#include "utility/archive.hpp"
#include "utility/mean_algorithm.hpp"
#include <cstddef>

namespace utility {

// Manage the mean and (sample) variance of a single data set. 
//
// Sample Mean     : sum(x)/n
// Sample Variance : sum((x-mean)**2)/n-1
//
// (Unit test suite "estimate_test_suite")

class estimater : private mean_algorithm
 {
   private:
    // Geometric mean value
    double mean_;

    // Sum of differences squared 
    //
    // (NOTE: real variance == this value/(count-1))
    double variance_;

    // Number of samples taken
    std::size_t count_;


   public:
    estimater()
    : mean_()
    , variance_()
    , count_(0ul)
    {}

    estimater(const estimater & other)
    : mean_( other.mean_ )
    , variance_( other.variance_ ) 
    , count_( other.count_ ) 
    {}

    estimater(estimater && other)
    : mean_( std::move( other.mean_ ) )
    , variance_( std::move( other.variance_ ) )
    , count_( std::move( other.count_ ) )
    {}

    inline estimater& operator=(estimater other)
    {
      this->swap( other );
      return *this;
    }


  friend class boost::serialization::access;


   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) 
    {
      ar & mean_;
      ar & variance_;
      ar & count_;
    }


   public:
    inline void swap(estimater & other)
    {
      std::swap( this->mean_, other.mean_ );
      std::swap( this->variance_, other.variance_ );
      std::swap( this->count_, other.count_ );
    }

    //Add a sample value to the estimate
    void append(double val) 
    {
      ++this->count_;
      mean_algorithm::append(this->mean_, this->variance_, val, this->count_);
    }

    //The number of samples taken
    std::size_t count() const
    {
      return this->count_;
    }

    //Test if two estimaters are equivalent
    bool equivalent(const estimater & other) const
    {
      return (this->count_ == other.count_) and (this->mean_ == other.mean_) and (this->variance_ == other.variance_);
    }

    template<typename IterType>
      void insert(IterType begin, IterType end)
    {
      for (;begin != end; ++begin) this->append(static_cast< double >(*begin));
    }

    inline double mean() const 
    {
      return this->mean_;
    }

    // Merge two estimates of the same statistic together.
    void merge(const estimater & other)
    {
      mean_algorithm::merge( this->mean_, this->variance_, this->count_, other.mean_, other.variance_, other.count_ );
    }

    friend bool operator==(const estimater & lhs, const estimater & rhs)
    {
      return lhs.equivalent( rhs );
    }

    friend bool operator!=(const estimater & lhs, const estimater & rhs)
    {
      return not lhs.equivalent( rhs );
    }

    void reset()
    {
      this->mean_ = 0.0;
      this->variance_ = 0.0;
      this->count_ = 0;
    }

    inline double variance() const 
    {
      return mean_algorithm::variance(this->variance_, this->count_);
    }


};

} // namespace utility
#endif
