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

#include "utility/basic_mean.hpp"

namespace utility {

//Add a sample value to the estimate
void basic_mean::append(double val, std::size_t count) 
{
  if (count == 1)
  {
    this->mean_ = val;
    this->variance_ = 0.0;
  }
  else
  {
    const double av_old(this->mean_);
    this->mean_ += (val - av_old)/double(count);
    this->variance_ += (val - av_old)*(val - this->mean_);
  }

}


} // namespace utility
