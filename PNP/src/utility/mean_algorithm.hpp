#ifndef IONCH_UTILITY_MEAN_ALGORITHM_HPP
#define IONCH_UTILITY_MEAN_ALGORITHM_HPP

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

#include <cstddef>

namespace utility {

// Class containing algorithm for calculating mean. This
// class is meant to be privately inherited by classes
// that wish to contain a running average.
//
// see wiki and corresponding reference below.
//
// https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance

class mean_algorithm
 {
   public:
    // Add a single sample value to the estimate
    //
    // \req count > 0: count == 0 is undefined (will result in X/0)
    //
    // NOTE: correct usage is (so count should never be 0!)
    //
    //  ++count
    // append( mean, variance, val, count )
    //
    // count is not updated in this method to allow sets of mean, 
    // variance pairs to share the same count.
    static void append(double & mean, double & variance, double val, std::size_t count);

    // Calculate the estimate's sample variance
    static double variance(double variance, std::size_t count)
    {
      return (count <= 1 ? 0.0 : variance/double(count-1));
    }
    

    // Merge together two sets of statistics
    //
    // see wiki and corresponding reference below
    //
    // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
    // 
    // Chan, Tony F.; Golub, Gene H.; LeVeque, Randall J. (1979),
    // "Updating Formulae and a Pairwise Algorithm for Computing
    // Sample Variances." (PDF), Technical Report STAN-CS-79-773,
    // Department of Computer Science, Stanford University.
    // 
    
    static void merge(double & lh_mean, double & lh_var, std::size_t & lh_count, double rh_mean, double rh_var, std::size_t rh_count);


};

} // namespace utility
#endif
