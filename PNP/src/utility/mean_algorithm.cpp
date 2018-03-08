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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "utility/mean_algorithm.hpp"

// manual includes
#include "utility/fuzzy_equals.hpp"
#include "utility/utility.hpp"
// -
#include <cmath>
// -
namespace utility {

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
void mean_algorithm::append(double & mean, double & variance, double val, std::size_t count)
{
  if( count == 1 )
  {
    mean = val;
    variance = 0.0;
  }
  else
  {
    const double av_old( mean );
    mean += ( val - av_old )/double( count );
    variance += ( val - av_old )*( val - mean );
  }
  

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

void mean_algorithm::merge(double & lh_mean, double & lh_var, std::size_t & lh_count, double rh_mean, double rh_var, std::size_t rh_count)
{
  // Cases
  //  lh_count : rh_count : action
  //      0    :  ---     : swap(lh,rh)
  //      1    :   0      : no op
  //      1    :   1      : append(lh, rhmean)
  //      1    :  >1      : append(rh, lhmean), swap(lh,rh)
  //     >1    :   0      : no op
  //     >1    :   1      : append(lh, rhmean)
  //     >1    :  >1      : MERGE
  
  if( rh_count == 0 ) return;
  if( rh_count == 1 )
  {
    UTILITY_REQUIRE( utility::feq( rh_var, 0.0 ), "Variance should be zero for count <= 1" );
    ++lh_count;
    append( lh_mean, lh_var, rh_mean, lh_count );
    return;
  }
  if( lh_count == 1 )
  {
    ++rh_count;
    append( rh_mean, rh_var, lh_mean, rh_count );
    lh_mean = rh_mean;
    lh_var = rh_var;
    lh_count = rh_count;
    return;
  }
  else
  {
    //  m_(l+r) = m_l.n_l/(n_l + n_r) + m_r.n_r/(n_l + n_r)
    //  d = m_r - m_l
    //  d^2 = (m_r - m_l)^2 == m_r^2 - 2.m_r.m_l + m_l^2
    //  v_(l+r) = v_l + v_r + d^2.(n_l.n_r/(n_l+n_r))
    //  n_(l+r) = (n_l + n_r)
    const std::size_t lh_count_old( lh_count );
    const double delta_sq( std::pow( rh_mean, 2 ) - 2 * rh_mean * lh_mean + std::pow( lh_mean, 2 ) );
    lh_count += rh_count;
    lh_mean *= ( double( lh_count_old )/double( lh_count ) );
    lh_mean += rh_mean * ( double( rh_count )/double( lh_count ) );
    lh_var += rh_var + ( delta_sq * rh_count * lh_count_old / double( lh_count ) );
  }
  
  

}


} // namespace utility
