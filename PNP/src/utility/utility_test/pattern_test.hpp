#ifndef IONCH__PATTERN_TEST_HPP
#define IONCH__PATTERN_TEST_HPP

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
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <type_traits>
#include "utility/archive.hpp"
// -
//Test standard patterns
struct pattern_test 
{
    //Function to conditionally include test if less than operator is defined
    //
    //Test not ( a < b or b < a )
    template< class test_class, bool has_lt > static inline typename std::enable_if< has_lt==true >::type check_lt_on_eq(const test_class & a, const test_class & b)
    {
        BOOST_CHECK( not ( a < b ) );
        BOOST_CHECK( not ( b < a ) );
    }

    //Function to conditionally NOT include test if less than operator is NOT defined
    template< class test_class, bool has_lt > static inline typename std::enable_if< has_lt==false >::type check_lt_on_eq(const test_class & a, const test_class & b)
    {}

    //Function to conditionally include test if less than operator is defined
    //
    //Test ( a < b or b < a )
    template< class test_class, bool has_lt > static inline typename std::enable_if< has_lt==true >::type check_lt_on_ne(const test_class & a, const test_class & b)
    {
        BOOST_CHECK( ( a < b ) or ( b < a ) );
    }

    //Function to conditionally NOT include test if less than operator is NOT defined
    template< class test_class, bool has_lt > static inline typename std::enable_if< has_lt==false >::type check_lt_on_ne(const test_class & a, const test_class & b)
    {}

    //Function to conditionally include serialization test if serialization is defined
    //
    //Tests
    //oarchive << a
    //iarchive >> b
    //b == a
    template< class test_class, bool has_io > static inline typename std::enable_if< has_io==true >::type check_serialize(const test_class & a)
    {
      std::stringstream store;
      {
        boost::archive::text_oarchive oa( store );
        oa << a;
      }
      {
        test_class obj3;
        boost::archive::text_iarchive ia( store );
        ia >> obj3;
        BOOST_CHECK( a == obj3 );
      }
    }

    //Function to conditionally NOT include serialization test if serialization is NOT defined
    template< class test_class, bool has_io > static inline typename std::enable_if< has_io==false >::type check_serialize(const test_class & a)
    {}

    // Test of a class matching canonical class pattern.
    //
    // Assumes the following
    // - constructors include: default, copy, move and assignment
    // - equivalence test using '=='
    // - parameter a /= test_class{}
    //
    // If has_lessthan is true assumes:
    // - if a == b then not ( ( a < b ) or ( b < a ) )
    // - if a /= b then ( a < b ) or ( b < a )
    //
    // If has_serialize is true assumes boost serialization
    // - oa << a
    // - ia >> b
    // - then a == b
    //
    
    template< class test_class, bool has_lessthan, bool has_serialize > static inline void canonical(test_class const& a, test_class const& b);

    // Unit test for objects that follow comparable/ordering pattern.
    //
    // Tested operators
    // - equality == and !=
    // - ordering < <= > >=
    //
    // Expect params to have order big > mid > sml
    // and dup to equal big, mid or sml.
    template< class test_class > static inline void comparable(const test_class & big, const test_class & mid, const test_class & sml, const test_class & dup);


};
// Test of a class matching canonical class pattern.
//
// Assumes the following
// - constructors include: default, copy, move and assignment
// - equivalence test using '=='
// - parameter a /= test_class{}
//
// If has_lessthan is true assumes:
// - if a == b then not ( ( a < b ) or ( b < a ) )
// - if a /= b then ( a < b ) or ( b < a )
//
// If has_serialize is true assumes boost serialization
// - oa << a
// - ia >> b
// - then a == b
//

template< class test_class, bool has_lessthan, bool has_serialize > inline void pattern_test::canonical(test_class const& a, test_class const& b)
{
  // default constructable
  {
    test_class obj_1{}, obj_2{};
    BOOST_CHECK( obj_1 == obj_2 );
    pattern_test::check_lt_on_eq< test_class, has_lessthan >( obj_1, obj_2 );
    pattern_test::check_serialize< test_class, has_serialize >( obj_1 );
  }
  // Copy constructor
  {
      test_class dummy{};
      BOOST_CHECK( not ( dummy == a ) );
      pattern_test::check_lt_on_ne< test_class, has_lessthan >( a, dummy );
      //, "First argument to function should not give the same state as default constructor." );
      test_class obj_a{ a };
      test_class obj_b{ b };
      BOOST_CHECK( obj_a == a );
      BOOST_CHECK( obj_b == b );
      pattern_test::check_lt_on_eq< test_class, has_lessthan >( obj_a, a );
      pattern_test::check_lt_on_eq< test_class, has_lessthan >( obj_b, b );
      pattern_test::check_serialize< test_class, has_serialize >( obj_a );
      pattern_test::check_serialize< test_class, has_serialize >( obj_b );
      if ( obj_a == obj_b )
      {
        BOOST_CHECK( a == b );
        pattern_test::check_lt_on_eq< test_class, has_lessthan >( obj_a, obj_b );
      }
      else
      {
        BOOST_CHECK( not ( a == b ) );
        pattern_test::check_lt_on_ne< test_class, has_lessthan >( obj_a, obj_b );
      }
  }
  // Assignment
  {
    {
      test_class obj_1{};
      test_class obj_2{};
      obj_2 = obj_1;
      BOOST_CHECK( obj_1 == obj_2 );
      pattern_test::check_lt_on_eq< test_class, has_lessthan >( obj_1, obj_2 );
    }
    {
      test_class obj_1{ a };
      test_class obj_2{};
      obj_2 = obj_1;
      BOOST_CHECK( obj_1 == obj_2 );
      pattern_test::check_lt_on_eq< test_class, has_lessthan >( obj_1, obj_2 );
    }
  }
  // Move constructor
  {
    {
      test_class obj_1{};
      test_class obj_2{ obj_1 };
      test_class obj_3{ std::move( obj_1 ) };
      BOOST_CHECK( obj_2 == obj_3 );
      pattern_test::check_lt_on_eq< test_class, has_lessthan >( obj_2, obj_3 );
    }
    {
      test_class obj_1{ a };
      test_class obj_2{ obj_1 };
      test_class obj_3{ std::move( obj_1 ) };
      BOOST_CHECK( obj_2 == obj_3 );
      pattern_test::check_lt_on_eq< test_class, has_lessthan >( obj_2, obj_3 );
    }
  }
  
  
  

}

// Unit test for objects that follow comparable/ordering pattern.
//
// Tested operators
// - equality == and !=
// - ordering < <= > >=
//
// Expect params to have order big > mid > sml
// and dup to equal big, mid or sml.
template< class test_class > inline void pattern_test::comparable(const test_class & big, const test_class & mid, const test_class & sml, const test_class & dup)
{
  // big, mid, sml
  BOOST_CHECK( big != mid );
  BOOST_CHECK( big != sml );
  BOOST_CHECK( mid != sml );
  BOOST_CHECK( big > mid );
  BOOST_CHECK( big > sml );
  BOOST_CHECK( mid > sml );
  BOOST_CHECK( big >= sml );
  BOOST_CHECK( big >= mid );
  BOOST_CHECK( mid >= sml );
  BOOST_CHECK( mid < big );
  BOOST_CHECK( sml < big );
  BOOST_CHECK( sml < mid );
  BOOST_CHECK( sml <= big );
  BOOST_CHECK( mid <= big );
  BOOST_CHECK( sml <= mid );
  BOOST_CHECK( ( big == dup ) or ( mid == dup ) or ( sml == dup ) );
  if( big == dup )
  {
    BOOST_CHECK( big >= dup );
    BOOST_CHECK( big <= dup );
  }
  else if ( mid == dup )
  {
    BOOST_CHECK( mid >= dup );
    BOOST_CHECK( mid <= dup );
  }
  else if ( sml == dup )
  {
    BOOST_CHECK( sml >= dup );
    BOOST_CHECK( sml <= dup );
  }

}

#endif
