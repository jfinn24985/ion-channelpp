#ifndef IONCH_UTILITY_FUZZY_EQUALS_HPP
#define IONCH_UTILITY_FUZZY_EQUALS_HPP

//  FUZZY EQUALs (feq) UTILITY FUNCTION
//
//  A template function for testing if two floating point numbers are
//  equal within a certain cutoff range.  This comparison is basically
//  based on ignoring a certain number of the least significant bits
//  of the floating point significand.
//
// (Unit test suite "fuzzyequals_test_suite")
//
//Authors: Justin Finnerty
//
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

// manual includes
#include "utility/config.hpp"
// - 
#include <limits>
// manual includes
namespace utility {

// Traits class to get integer of same size as floating point type
//
// (Unit test suite "fuzzyequals_test_suite")
template<class Float_type>
struct ftoi 
{
    typedef int(* int_type)();


};
// Traits class to get integer of same size as float
//
// (Unit test suite "fuzzyequals_test_suite")
template<>
struct ftoi<float> 
{
    typedef int32_t int_type;


};
// Traits class to get integer of same size as double
//
// (Unit test suite "fuzzyequals_test_suite")
template<>
struct ftoi<double> 
{
    typedef int64_t int_type;


};

  template < class Float_type>
  inline bool feq (Float_type lhs, Float_type rhs, int MaxUlps=32ul )
  {
    // REMOVED if test as it (may) offer performance advantage [icpc: 15/17][g++: 22/20]
    // if (lhs == rhs) { return true; }
  
    // Make sure maxUlps is non-negative and small enough that the
    // default NAN won't compare as equal to anything.
    typename ftoi<Float_type>::int_type zr (1);

    zr <<= std::numeric_limits<Float_type>::digits;

    const union f_
    {
      Float_type f; 
      typename ftoi<Float_type>::int_type i; 
      f_ (Float_type a) : f(a) {}
      f_ (typename ftoi<Float_type>::int_type a) : i(a) {}
    } lhsu (lhs), rhsu (rhs), zero (zr);

    // Make lhs_int lexicographically ordered as a twos-complement int
    const typename ftoi<Float_type>::int_type lhs_int (lhsu.f < 0 ? zero.i - lhsu.i : lhsu.i);
  
    // Make rhs_uint lexicographically ordered as a twos-complement int
    const typename ftoi<Float_type>::int_type rhs_int (rhsu.f < 0 ? zero.i - rhsu.i : rhsu.i);

    // std::cout << "fULPs = " << std::abs (lhs_int - rhs_int) << " for " << lhs_int << " and " << rhs_int << "\n";
    return MaxUlps >= (lhs_int > rhs_int ? lhs_int - rhs_int : rhs_int - lhs_int);
  }


} // namespace utility
#endif
