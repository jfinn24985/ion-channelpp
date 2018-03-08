#ifndef IONCH_UTILITY_BASIC_MEAN_HPP
#define IONCH_UTILITY_BASIC_MEAN_HPP

//----------------------------------------------------------------------
//DATA-SET SUMMARY/REDUCTION CLASSES
//
//These classes provide classes to manage statistical sampling in
//2 or 3D histograms or fields and to calculate mean and
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

namespace utility {

//  Part class to manage a mean/variance pair of
//  a sequence.  This class relies on an external
//  class to maintain the sample count

class basic_mean
 {
   private:
    //Geometric mean value
    double mean_;

    //Sum of differences squared 
    //
    //(NOTE: real variance == this value/(count-1))
    double variance_;


   public:
    //Add a sample value to the estimate
    void append(double val, std::size_t count);

    basic_mean() : mean_(0.0), variance_(0.0) {};

    basic_mean(const basic_mean & other)
    : mean_ (other.mean_)
    , variance_ (other.variance_)
    {}

    basic_mean(basic_mean && other)
    : mean_( std::move( other.mean_ ) )
    , variance_( std::move( other.variance_ ) )
    {}

    bool equivalent(const basic_mean & other) const
    {
      return this->mean_ == other.mean_ and this->variance_ == other.variance_;
    }

    //Get the estimate's geometric mean
    double mean() const
    {
      return this->mean_;
    }

    basic_mean& operator=(basic_mean other)
    {
      this->swap(other);
      return *this;
    }

    void reset()
    {
      this->mean_ = 0.0;
      this->variance_ = 0.0;
    }


  friend class boost::serialization::access;


   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & mean_;
        ar & variance_;
      }


   public:
    void swap(basic_mean & other)
    {
      std::swap(this->mean_, other.mean_);
      std::swap(this->variance_, other.variance_);
    }

    //Get the estimate's sample variance
    double variance(std::size_t count) const
    {
      return (count <= 1 ? 0.0 : this->variance_/double(count-1));
    }

friend inline bool operator==(const basic_mean &lhs, const basic_mean &rhs)
{
  return lhs.equivalent( rhs );
}


};

} // namespace utility
#endif
