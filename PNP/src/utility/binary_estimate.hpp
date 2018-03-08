#ifndef IONCH_UTILITY_BINARY_ESTIMATE_HPP
#define IONCH_UTILITY_BINARY_ESTIMATE_HPP

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

// Manage a value that is derived from repeated binary (pass/fail) 
// observations.
//
// This class uses a successive addition algorithm to calculate the
// mean and variance of a sample value. This is reported to be
// numerically more stable than the traditional sum and sum of
// squares methods.
//
// The default Counter_Type is size_t.  Generally this should be an
// unsigned integer of approximately the same size as the Float_Type
class binary_estimate
 {
   private:
    //The number of trials performed
    std::size_t count_;

    //The number of 'positive' results
    std::size_t heads_;

    //The cumulative variance
    double variance_;


   public:
    //Add a pass or fail trial.
    void append(bool success);

    binary_estimate()
    : count_(0ul)
    , heads_(0ul)
    , variance_(0.0)
    {}
    binary_estimate(const binary_estimate & other) 
    : count_ (other.count_)
    , heads_ (other.heads_)
    , variance_ (other.variance_)
    {}

    binary_estimate(binary_estimate && other) 
    : count_( other.count_ )
    , heads_( other.heads_ )
    , variance_( other.variance_ )
    {}

    ~binary_estimate();
    void swap(binary_estimate & other)
    {
      std::swap (count_, other.count_);
      std::swap (heads_, other.heads_);
      std::swap (variance_, other.variance_);
    }

    binary_estimate & operator =(binary_estimate other)
    {
      this->swap(other);
      return *this;
    }

    //Get the number of trials
    std::size_t count() const
    {
      return this->count_;
    }

    // Compare two binary estimates for equality
    bool equivalent(const binary_estimate & source) const
    {
      return (this->count_ == source.count_) and (this->heads_ == source.heads_) and (this->variance_ == source.variance_);
    }

    //Add a fail trial
    void fail()
    {
      this->append (false);
    }

    //Number of failed attempts
    std::size_t fail_count() const
    {
      return this->count_ - this->heads_;
    }

    double mean() const
    {
      return (this->count_ == 0 ? 0.0 : double(this->heads_)/double(this->count_));
    }

friend inline bool operator==(const binary_estimate & lhs, const binary_estimate & rhs)
{
  return lhs.equivalent( rhs );
}

  friend class boost::serialization::access;


   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & count_;
        ar & heads_;
        ar & variance_;
      }


   public:
    //Add a successful trial
    void success()
    {
      this->append (true);
    }

    //Number of successful attempts
    std::size_t success_count() const
    {
      return this->heads_;
    }

    void reset()
    {
      this->count_ = 0;
      this->heads_ = 0;
      this->variance_ = 0;
    }

    //Get the number of trials
    std::size_t size() const
    {
      return this->count_;
    }

    double stddev() const
    {
      return (this->count_ > 1 ? std::sqrt(this->variance()) : 0.0);
    }

    //Get the variance of the sample.
    double variance() const
    {
      return (this->count_ > 1 ? this->variance_/(double(this->count_ - 1)) : 0.0); 
    }


};

} // namespace utility
#endif
