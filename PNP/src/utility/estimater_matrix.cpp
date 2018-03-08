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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "utility/estimater_matrix.hpp"

namespace utility {

void estimate_2d::reset() 
{
  estimate_2d::index_type idx;
  for (std::size_t i = 0; i != this->array_.shape()[0]; ++i)
  {
    idx[0] = i;
    for (std::size_t j = 0; j != this->array_.shape()[1]; ++j)
    {
      idx[1] = j;
      this->array_(idx).reset();
    }
  }
  this->count_ = 0;

}

void estimate_2d::swap(estimate_2d & other) 
{
  std::swap(this->array_, other.array_);
  std::swap(this->count_, other.count_);
}

void estimate_3d::reset() 
{
  estimate_3d::index_type idx;
  for (std::size_t i = 0; i != this->array_.shape()[0]; ++i)
  {
    idx[0] = i;
    for (std::size_t j = 0; j != this->array_.shape()[1]; ++j)
    {
      idx[1] = j;
      for (std::size_t k = 0; k != this->array_.shape()[2]; ++k)
      {
        idx[2] = k;
        this->array_(idx).reset();
      }
    }
  }
  this->count_ = 0;
  

}

void estimate_3d::swap(estimate_3d & other) 
{
  std::swap(this->array_, other.array_);
  std::swap(this->count_, other.count_);
}


} // namespace utility
