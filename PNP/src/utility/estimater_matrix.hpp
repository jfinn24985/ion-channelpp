#ifndef IONCH_UTILITY_ESTIMATER_MATRIX_HPP
#define IONCH_UTILITY_ESTIMATER_MATRIX_HPP

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

// Manual includes
#include "utility/archive.hpp"
#include "utility/utility.hpp"
//-
#include "utility/basic_mean.hpp"
#include <array>
#include <cstddef>
#include "utility/multi_array.hpp"

namespace utility {

// Manage the mean and (biased) variance of a 2D array of data. 
//
// (Unit test suite "estimater_test_fw")

class estimate_2d
 {
   public:
    typedef std::array< std::size_t, 2ul > index_type;


   private:
    //The 2D array of data
    boost::multi_array< basic_mean, 2 > array_;

    //Sampling count
    std::size_t count_;


   public:
    // Append an array of values data to the current estimator array
    //
    // distance(begin,end) < 0 is undefined
    // distance(begin,end) < size, zero appended for missing elements
    // distance(begin,end) > size, extra elements ignored
    //
    // returns iterator after the last element used
    template<typename IterType>
      IterType append(IterType begin, IterType end){
      ++this->count_;
      estimate_2d::index_type idx;
      for (std::size_t i = 0; i != this->array_.shape()[0]; ++i)
      {
        idx[0] = i;
        for (std::size_t j = 0; j != this->array_.shape()[1] and begin != end; ++j, ++begin)
        {
          idx[1] = j;
          this->array_(idx).append (*begin, this->count_);
        }
      }
      return begin;
    }
    std::size_t count() const
    {
      return this->count_;
    }

    estimate_2d() : array_(), count_() {};

    estimate_2d(index_type sz) : array_(sz), count_() {}

    estimate_2d(const estimate_2d & source)
    : array_(source.array_)
    , count_(source.count_)
    {}

    ~estimate_2d() = default;

    double mean(index_type idx) const
    {
      return this->array_(idx).mean();
    }
    estimate_2d & operator=(estimate_2d source)
    {
      this->swap(source);
      return *this;
    }

    void resize(index_type sz)
    {
      this->array_.resize(sz);
    }

    void reset();


  friend class boost::serialization::access;


   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & array_;
        ar & count_;
      }


   public:
    const std::size_t *const shape() const
    {
      return this->array_.shape();
    }

    std::size_t size() const
    {
      return this->array_.num_elements();
    }

    void swap(estimate_2d & other);

    double variance(index_type idx) const
    {
      return this->array_(idx).variance(this->count_);
    }


};
// Manage the mean and (biased) variance of a 2D array of data. 
//
// (Unit test suite "estimate_test_suite")

class estimate_3d
 {
   public:
    typedef boost::multi_array< basic_mean, 3ul>::const_iterator const_iterator;

    typedef std::array< std::size_t, 3ul > index_type;


   private:
    //The 2D array of data
    boost::multi_array< basic_mean, 3 > array_;

    //Sampling count
    std::size_t count_;


   public:
    // Append an array of values data to the current estimator array
    //
    // distance(begin,end) < 0 is undefined
    // distance(begin,end) < size, zero appended for missing elements
    // distance(begin,end) > size, extra elements ignored
    //
    // returns iterator after the last element used
    template<typename IterType>
      IterType append(IterType begin, IterType end)
    {
      ++this->count_;
      estimate_3d::index_type idx;
      for (std::size_t i = 0; i != this->array_.shape()[0]; ++i)
      {
        idx[0] = i;
        for (std::size_t j = 0; j != this->array_.shape()[1]; ++j)
        {
          idx[1] = j;
          for (std::size_t k = 0; k != this->array_.shape()[2] and begin != end; ++k, ++begin)
          {
            idx[2] = k;
            this->array_(idx).append (*begin, this->count_);
          }
        }
      }
      return begin;
    }
    const_iterator begin() const
    {
      return this->array_.begin();
    }

    std::size_t count() const
    {
      return this->count_;
    }

    const_iterator end() const
    {
      return this->array_.end();
    }

    estimate_3d() : array_(), count_() {};

    estimate_3d(index_type sz) : array_(sz), count_() {}

    estimate_3d(const estimate_3d & source)
    : array_(source.array_)
    , count_(source.count_)
    {}

    ~estimate_3d() = default;

    estimate_3d & operator=(estimate_3d source)
    {
      this->swap(source);
      return *this;
    }

    double mean(index_type idx) const
    {
      return this->array_(idx).mean();
    }
    void resize(index_type sz)
    {
      this->array_.resize(sz);
    }

    void reset();


  friend class boost::serialization::access;


   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & array_;
        ar & count_;
      }


   public:
    const std::size_t *const shape() const
    {
      return this->array_.shape();
    }

    std::size_t size() const
    {
      return this->array_.num_elements();
    }

    void swap(estimate_3d & other);

    double variance(index_type idx) const
    {
      return this->array_(idx).variance(this->count_);
    }


};

} // namespace utility
#endif
