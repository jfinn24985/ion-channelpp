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

#include "utility/binary_estimate.hpp"

namespace utility {

//Add a pass or fail trial.
void binary_estimate::append(bool success) 
{
  if (0 != this->count_)
  {
    if (success)
    {
      std::size_t tails (this->count_ - this->heads_);
      this->variance_ += double(tails*tails)/double(this->count_*(this->count_+1));
    }
    else
    {
      this->variance_ += double(this->heads_*this->heads_)/double(this->count_*(this->count_+1));
    }
  }
  else
  {
    this->variance_ = double(success ? 1 : 0);
  }
  // Increment counter here as we need the old count when
  // calculating the variance.
  ++(this->count_);
  // Increment heads on success
  if (success)
  {
    ++(this->heads_);
  }

}

binary_estimate::~binary_estimate() = default;

} // namespace utility
