#ifndef IONCH_UTILITY_MATHUTIL_H
#define IONCH_UTILITY_MATHUTIL_H

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

#include <cstddef>

namespace utility {

//Container of several maths utility functions
//
// (Unit test suite "math_test_suite")
namespace mathutil
 {
    //Calculate if line segment x1,y1 : x2,y2 and circle x0,y0,radius
    //intersect.
    //
    //TODO: No test method
    bool linesegment_circle_intersect(double x1, double y1, double x2, double y2, double x0, double y0, double radius);


namespace implementation
{
    // Nearest power of 2
    //
    // For existing powers of 2, returns the input value
    //
    // Code should demangle to:
    // next2_i does:
    //  --a;
    //  a |= (a >> 1);
    
    constexpr inline int next2_i(int a)
    {
      return (a-1) | ((a-1) >> 1);
    }

    // Nearest power of 2
    //
    // For existing powers of 2, returns the input value
    //
    // Code should demangle to:
    // next2_h does
    //  a |= (a >> 2);
    
    constexpr inline int next2_h(int a)
    {
      return next2_i(a) | (next2_i(a) >> 2);
    }

    // Nearest power of 2
    //
    // For existing powers of 2, returns the input value
    //
    // Code should demangle to:
    // next2_g does
    //  a |= (a >> 4);
    //}
    
    constexpr inline int next2_g(int a)
    {
      return next2_h(a) | (next2_h(a) >> 4);
    }

    // Nearest power of 2
    //
    // For existing powers of 2, returns the input value
    //
    // Code should demangle to:
    // next2_f does
    //  a |= (a >> 8);
    
    constexpr inline int next2_f(int a)
    {
      return next2_g(a) | (next2_g(a) >> 8);
    }

}
    // Nearest power of 2
    //
    // For existing powers of 2, returns the input value
    //
    // Code should demangle to:
    // {
    //  --a;
    //  a |= (a >> 1);
    //  a |= (a >> 2);
    //  a |= (a >> 4);
    //  a |= (a >> 8);
    //  a |= (a >> 16);
    //  return a + 1;
    //}
    
    constexpr inline int next2(int a)
    {
      return (implementation::next2_f(a) | (implementation::next2_f(a) >> 16)) + 1;
    }

    // Nearest multiple of 64
    //
    // For an existing multiple of 64, returns the input value
    constexpr inline int next64(int a)
    {
      return ((a & 63) == 0 ? a : (a + 64) & ~63);
    }

    //Is val in the range [min, max] ?
    template< typename NumberType > constexpr inline bool in_closed_range(NumberType a_min, NumberType a_max, NumberType a_val)
    {
      return (a_min <= a_val and a_val <= a_max);
    }

    //Is val in the range (min, max) ?
    template< typename NumberType > constexpr inline bool in_open_range(NumberType a_min, NumberType a_max, NumberType a_val)
    {
      return (a_min < a_val and a_val < a_max);
    }

    //Is val in the range [min, max) ?
    template< typename NumberType > constexpr inline bool in_half_up_range(NumberType a_min, NumberType a_max, NumberType a_val)
    {
      return (a_min <= a_val and a_val < a_max);
    }

    //Is val in the range (min, max] ?
    template< typename NumberType > constexpr inline bool in_half_down_range(NumberType a_min, NumberType a_max, NumberType a_val)
    {
      return (a_min < a_val and a_val <= a_max);
    }

    // Return position in [begin,end) of ith occurance of val in
    // arr[*begin] or end if no element found.
    //
    // CONCEPT REQUIRE: is_comparable(ValueType, ArrayType::value_type)
    // CONCEPT REQUIRE: is_convertible(typeof(*begin), ArrayType::size_type)
    // UNDEFINED iff: not in_half_up_range(0, arr.size, *iter) for iter in [begin,end)
    
    template< typename IterType, typename ArrayType, typename ValueType > IterType find_n_of(IterType begin, IterType end, const ArrayType & arr, const ValueType & val, std::size_t ith)
    {
       for ( ; begin != end; ++begin)
       {
         if (val == arr[*begin])
         {
            --ith;
            if (ith == 0) break;
         }
       }
      return begin;
    }

    // Return position in [begin,end) of ith occurance of val in
    // *begin or end if no element found.
    //
    // CONCEPT REQUIRE: is_convertible(typeof(*begin), ValueType)
    
    template< typename IterType, typename ValueType > IterType find_n(IterType begin, IterType end, const ValueType & val, std::size_t ith)
    {
       for ( ; begin != end; ++begin)
       {
         if (val == *begin)
         {
            --ith;
            if (ith == 0) break;
         }
       }
      return begin;
    }


};

} // namespace utility
#endif
