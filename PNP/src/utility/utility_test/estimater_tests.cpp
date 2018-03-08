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

#include <boost/test/unit_test.hpp>

#include "utility/utility_test/estimater_tests.hpp"
#include "utility/basic_histogram.hpp"
#include "utility/digitizer.hpp"
#include "utility/estimater.hpp"
#include "utility/estimate_array.hpp"
#include "utility/fixed_size_histogram.hpp"
#include "utility/histogram.hpp"
#include "utility/utility_test/pattern_test.hpp"
#include "utility/binary_estimate.hpp"
#include "utility/basic_mean.hpp"
#include "utility/estimater_matrix.hpp"

estimater_test_fw estimater_test_fw::exmplr;

estimater_test_fw::estimater_test_fw() 
: var(31ul)
, data4(100, 4.0)
, data5(100, 5.0)
, data6(100, 6.0)
, data_index(100)
, mean(5.2874135167)
, svar (48.9042830931402)
, variance (47.32672557400666)
{
  for (std::size_t ith = 0; ith != 100; ++ith) data_index[ith] = ith;
  var[ 0] =      5.5516621838;
  var[ 1] =      0.2100560162;
  var[ 2] =      0.592851646;
  var[ 3] =      0.7387133442;
  var[ 4] =      0;
  var[ 5] =      0.0603448137;
  var[ 6] =      0.0399201735;
  var[ 7] =      8.3830492441;
  var[ 8] =     10.6720101019;
  var[ 9] =      8.5243113113;
  var[10] =      0.8336070837;
  var[11] =      0.7854093157;
  var[12] =      0.4436615558;
  var[13] =     14.1975433413;
  var[14] =      25.657982967;
  var[15] =      6.8852868479;
  var[16] =      0.3215328372;
  var[17] =      4.4619749979;
  var[18] =      0.017607702;
  var[19] =     21.0264223786;
  var[20] =     19.0367584515;
  var[21] =      6.7498704974;
  var[22] =      0.5602527081;
  var[23] =     13.1845872369;
  var[24] =      0.4063910795;
  var[25] =      0.3934774747;
  var[26] =      1.0065240381;
  var[27] =      0.0302535373;
  var[28] =      0.0973062654;
  var[29] =      8.2189127477;
  var[30] =      4.8215371193;
}




//Series of tests to check estimater lifetime methods
BOOST_AUTO_TEST_CASE( estimater_lifetime )
{
  {
    // Canonical pattern
    BOOST_CHECK( std::is_default_constructible< utility::estimater >::type{} );
    BOOST_CHECK( std::is_copy_constructible< utility::estimater >::type{} );
    BOOST_CHECK( std::is_move_constructible< utility::estimater >::type{} );
    BOOST_CHECK( ( std::is_assignable< utility::estimater, utility::estimater >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< utility::estimater >::type{} );
  }
  
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TEST: default ctor
    utility::estimater var1;
    //
    // Test that initial values are well-defined
    BOOST_REQUIRE( var1.count() == 0 );
    BOOST_REQUIRE( utility::feq( var1.mean(), 0.0 ) );
    BOOST_REQUIRE( utility::feq( var1.variance(), 0.0 ) );
  }
  {
    utility::estimater var1;
  
    // TEST: copy constructor on empty object
    utility::estimater var2( var1 );
  
    // var1 should be unchanged
    BOOST_CHECK_EQUAL( var1.count(), 0 );
    BOOST_CHECK( utility::feq( var1.mean(), 0.0 ) );
    BOOST_CHECK( utility::feq( var1.variance(), 0.0 ) );
    // var2 should have the same data as var1
    BOOST_CHECK_EQUAL( var1.mean(), var2.mean() );
    BOOST_CHECK_EQUAL( var1.variance(), var2.variance() );
    BOOST_CHECK_EQUAL( var1.count(), var2.count() );
  }
  {
    utility::estimater var1;
    // Insert some values (using raw pointers)
    var1.insert( &exemplar.var[0], &exemplar.var[31] );
  
    // Run canonical test
    {
      utility::estimater var3;
      // Insert some values (using raw pointers)
      var3.insert( &exemplar.var[0], &exemplar.var[30] );
  
      pattern_test::canonical< utility::estimater, false, true >( var1, var3 );
    }
  
    // TEST: copy constructor on object with data
    utility::estimater var2( var1 );
  
    // var1 should be unchanged
    BOOST_CHECK_EQUAL( var1.count(), 31 );
    BOOST_CHECK( utility::feq( var1.mean(), exemplar.mean ) );
    BOOST_CHECK( utility::feq( var1.variance(), exemplar.svar, 1<<9 ) );
    // var2 should have the same data as var1
    BOOST_CHECK_EQUAL( var1.mean(), var2.mean() );
    BOOST_CHECK_EQUAL( var1.variance(), var2.variance() );
    BOOST_CHECK_EQUAL( var1.count(), var2.count() );
  }
  {
    utility::estimater var1;
    // Insert some values (using iterators)
    var1.insert( exemplar.var.begin(), exemplar.var.end() );
  
    // TEST: assignment
    utility::estimater var2;
    // insert some values (insert limited set of values using raw pointers)
    var2.insert( &exemplar.var[0], &exemplar.var[11] );
    // assignment should overwrite existing data
    var2 = var1;
  
    // var1 should be unchanged
    BOOST_CHECK_EQUAL( var1.count(), 31 );
    BOOST_CHECK( utility::feq( var1.mean(), exemplar.mean ) );
    BOOST_CHECK( utility::feq( var1.variance(), exemplar.svar, 1<<9 ) );
    // var2 should have the same data as var1
    BOOST_CHECK_EQUAL( var1.mean(), var2.mean() );
    BOOST_CHECK_EQUAL( var1.variance(), var2.variance() );
    BOOST_CHECK_EQUAL( var1.count(), var2.count() );
  }
  {
    std::stringstream store;
    {
      utility::estimater var1;
      var1.insert( exemplar.var.begin(), exemplar.var.end() );
      boost::archive::text_oarchive oa( store );
      // write class instance to archive
      oa << var1;
      // var1 should be unchanged
      BOOST_CHECK_EQUAL( var1.count(), 31 );
      BOOST_CHECK( utility::feq( var1.mean(), exemplar.mean ) );
      BOOST_CHECK( utility::feq( var1.variance(), exemplar.svar, 1<<9 ) );
    }
    {
      utility::estimater var2;
      boost::archive::text_iarchive ia( store );
      // read class instance to archive
      ia >> var2;
      // var2 should have the same data as var1 had
      BOOST_CHECK_EQUAL( var2.count(), 31 );
      BOOST_CHECK( utility::feq( var2.mean(), exemplar.mean ) );
      BOOST_CHECK( utility::feq( var2.variance(), exemplar.svar, 1<<9 ) );
    }
  }

}

//Perform series of tests on the data collection routines of the
//estimater class
BOOST_AUTO_TEST_CASE( estimater_data_collection )
{
  estimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  {
    utility::estimater test_var;
    // TEST: default ctor
    //
    // Test that initial values are well-defined
    BOOST_CHECK_EQUAL(test_var.count(), 0);
    BOOST_REQUIRE(utility::feq(test_var.mean(), 0.0));
    BOOST_REQUIRE(utility::feq(test_var.variance(), 0.0));
  }
  {
    utility::estimater test_var;
    // TEST: append method
    for (auto val : exemplar.var)
    {
      test_var.append(val);
    }
    BOOST_CHECK_EQUAL(test_var.count(), 31);
    BOOST_CHECK(utility::feq(test_var.mean(), exemplar.mean));
    BOOST_CHECK(utility::feq(test_var.variance(), exemplar.svar));
  }
  {
    // TEST: insert method
    utility::estimater test_var;
  
    test_var.insert(&exemplar.var[0], &exemplar.var[31]);
  
    BOOST_CHECK_EQUAL(test_var.count(), 31);
    BOOST_CHECK(utility::feq(test_var.mean(), exemplar.mean));
    BOOST_CHECK(utility::feq(test_var.variance(), exemplar.svar));
  
    // TEST: reset returns to well-defined values
    test_var.reset ();
  
    BOOST_CHECK_EQUAL(test_var.count(), 0);
    BOOST_CHECK(utility::feq(test_var.mean(), 0.0));
    BOOST_CHECK(utility::feq(test_var.variance(), 0.0));
  
    // Check that reinserting value gives the
    // same results after the reset.
    test_var.insert(&exemplar.var[0], &exemplar.var[31]);
  
    BOOST_CHECK_EQUAL(test_var.count(), 31);
    BOOST_CHECK(utility::feq(test_var.mean(), exemplar.mean));
    BOOST_CHECK(utility::feq(test_var.variance(), exemplar.svar));
  }
  {
    // TEST: merge method
    utility::estimater v1, v2, v3, v4;
  
    v1.insert(&exemplar.var[0], &exemplar.var[31]);
    v2.insert(&exemplar.var[0], &exemplar.var[15]);
    v3.insert(&exemplar.var[15], &exemplar.var[31]);
  
    v4 = v2;
    v4.merge( v3 );
    BOOST_CHECK_EQUAL( v1.count(), 31 );
    BOOST_CHECK_EQUAL( v2.count(), 15 );
    BOOST_CHECK_EQUAL( v3.count(), 16 );
    BOOST_CHECK_EQUAL( v4.count(), 31 );
    BOOST_CHECK( utility::feq( v4.mean(), exemplar.mean ) );
    BOOST_CHECK( utility::feq( v4.variance(), exemplar.svar ) );
    BOOST_CHECK( utility::feq( v4.mean(), v1.mean() ) );
    BOOST_CHECK( utility::feq( v4.variance(), v1.variance() ) );
  }
  

}

BOOST_AUTO_TEST_CASE( estimate_array_lifetime )
{
  {
    // Canonical pattern
    BOOST_CHECK( std::is_default_constructible< utility::estimate_array >::type{} );
    BOOST_CHECK( std::is_copy_constructible< utility::estimate_array >::type{} );
    BOOST_CHECK( std::is_move_constructible< utility::estimate_array >::type{} );
    BOOST_CHECK( ( std::is_assignable< utility::estimate_array, utility::estimate_array >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< utility::estimate_array >::type{} );
  }
  
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // Default ctor
    utility::estimate_array var;
  
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 0ul );
  
    // Resize array
    var.resize( 25 );
  
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 25ul );
  }
  {
    // Ctor with size
    utility::estimate_array var( 100 );
  
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 100ul );
  }
  {
    // Test of copy and serialization
    std::stringstream store;
    {
      utility::estimate_array var( 100 );
  
      var.append( exemplar.data4.begin(), exemplar.data4.end() );
      var.append( exemplar.data5.begin(), exemplar.data5.end() );
      var.append( exemplar.data6.begin(), exemplar.data6.end() );
  
      BOOST_CHECK_EQUAL( var.count(), 3ul );
      BOOST_CHECK_EQUAL( var.size(), 100ul );
      for( std::size_t ith = 0; ith != var.size(); ++ith )
      {
        BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
        BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
      }
  
      {
        // copy
        utility::estimate_array var_copy( var );
        BOOST_CHECK_EQUAL( var_copy.count(), 3ul );
        BOOST_CHECK_EQUAL( var_copy.size(), 100ul );
        for( std::size_t ith = 0; ith != var_copy.size(); ++ith )
        {
          BOOST_CHECK_EQUAL( var_copy.mean( ith ), 5.0 );
          BOOST_CHECK_EQUAL( var_copy.variance( ith ), 1.0 );
        }
      }
  
      boost::archive::text_oarchive oa( store );
  
      // write class instance to archive
      oa << var;
      // var should be unchanged
      BOOST_CHECK_EQUAL( var.count(), 3ul );
      BOOST_CHECK_EQUAL( var.size(), 100ul );
      for( std::size_t ith = 0; ith != var.size(); ++ith )
      {
        BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
        BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
      }
      {
        // assign
        utility::estimate_array var_copy;
  
        BOOST_CHECK_EQUAL( var_copy.count(), 0ul );
        BOOST_CHECK_EQUAL( var_copy.size(), 0ul );
  
        var_copy = var;
        BOOST_CHECK_EQUAL( var_copy.count(), 3ul );
        BOOST_CHECK_EQUAL( var_copy.size(), 100ul );
        for( std::size_t ith = 0; ith != var_copy.size(); ++ith )
        {
          BOOST_CHECK_EQUAL( var_copy.mean( ith ), 5.0 );
          BOOST_CHECK_EQUAL( var_copy.variance( ith ), 1.0 );
        }
      }
  
      {
        // move
        utility::estimate_array var_copy1( var );
        BOOST_CHECK_EQUAL( var_copy1.count(), 3ul );
        BOOST_CHECK_EQUAL( var_copy1.size(), 100ul );
        utility::estimate_array var_copy( std::move( var_copy1 ) );
        BOOST_CHECK_EQUAL( var_copy.count(), 3ul );
        BOOST_CHECK_EQUAL( var_copy.size(), 100ul );
        for( std::size_t ith = 0; ith != var_copy.size(); ++ith )
        {
          BOOST_CHECK_EQUAL( var_copy.mean( ith ), 5.0 );
          BOOST_CHECK_EQUAL( var_copy.variance( ith ), 1.0 );
        }
      }
  
    }
    {
      utility::estimate_array var;
      BOOST_CHECK_EQUAL( var.count(), 0ul );
      BOOST_CHECK_EQUAL( var.size(), 0ul );
      boost::archive::text_iarchive ia( store );
      // read class instance from archive
      ia >> var;
      // var should have the same data as var1 had
      BOOST_CHECK_EQUAL( var.size(), 100ul );
      BOOST_CHECK_EQUAL( var.count(), 3ul );
      for( std::size_t ith = 0; ith != var.size(); ++ith )
      {
        BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
        BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
      }
    }
  }

}
BOOST_AUTO_TEST_CASE( estimate_array_methods )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // merge
    utility::estimate_array var1( 100 );
    utility::estimate_array var2( 100 );
    var1.append( exemplar.data4.begin(), exemplar.data4.end() );
    var1.append( exemplar.data5.begin(), exemplar.data5.end() );
    var2.append( exemplar.data6.begin(), exemplar.data6.end() );
  
    BOOST_CHECK_EQUAL( var1.size(), 100ul );
    BOOST_CHECK_EQUAL( var1.count(), 2ul );
    BOOST_CHECK_EQUAL( var2.size(), 100ul );
    BOOST_CHECK_EQUAL( var2.count(), 1ul );
  
    var1.merge( var2 );
    for( std::size_t ith = 0; ith != var1.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var1.mean( ith ), 5.0 );
      BOOST_CHECK_EQUAL( var1.variance( ith ), 1.0 );
    }
  }
  {
    // extend(using reindex) + merge
    utility::estimate_array var1( 100 );
    utility::estimate_array var2( 50 );
    std::vector< double > p1( 100 );
    std::copy( exemplar.data6.begin(), exemplar.data6.begin() + 50, p1.begin() );
    var1.append( exemplar.data4.begin(), exemplar.data4.end() );
    var1.append( exemplar.data5.begin(), exemplar.data5.end() );
    var2.append( exemplar.data6.begin() + 50, exemplar.data6.end() );
  
    BOOST_CHECK_EQUAL( var1.size(), 100ul );
    BOOST_CHECK_EQUAL( var1.count(), 2ul );
    BOOST_CHECK_EQUAL( var2.size(), 50ul );
    BOOST_CHECK_EQUAL( var2.count(), 1ul );
  
    var2.reindex( 0, 50 );
    BOOST_CHECK_EQUAL( var2.size(), 100ul );
    var2.append( p1.begin(), p1.end() );
    BOOST_CHECK_EQUAL( var2.count(), 2ul );
    for( std::size_t ith = 0; ith != var2.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var2.mean( ith ), 3.0 );
    }
     
   
    var1.merge( var2 );
    BOOST_CHECK_EQUAL( var1.count(), 4ul );
    for( std::size_t ith = 0; ith != var1.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var1.mean( ith ), 3.75 );
      BOOST_CHECK_CLOSE( var1.variance( ith ), 6.9166666666667, 0.0000000001 );
    }
  }
  {
    // resize(using resize) + merge
    utility::estimate_array var1( 100 );
    utility::estimate_array var2( 50 );
    std::vector< double > p1( 100 );
    std::copy( exemplar.data6.begin() + 50, exemplar.data6.end(), p1.begin() + 50 );
    var1.append( exemplar.data4.begin(), exemplar.data4.end() );
    var1.append( exemplar.data5.begin(), exemplar.data5.end() );
    var2.append( exemplar.data6.begin(), exemplar.data6.begin() + 50 );
  
    BOOST_CHECK_EQUAL( var1.size(), 100ul );
    BOOST_CHECK_EQUAL( var1.count(), 2ul );
    BOOST_CHECK_EQUAL( var2.size(), 50ul );
    BOOST_CHECK_EQUAL( var2.count(), 1ul );
  
    var2.resize( 100 );
    var2.append( p1.begin(), p1.end() );
    BOOST_CHECK_EQUAL( var2.count(), 2ul );
  
    var1.merge( var2 );
    BOOST_CHECK_EQUAL( var1.count(), 4ul );
    for( std::size_t ith = 0; ith != var1.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var1.mean( ith ), 3.75 );
      BOOST_CHECK_CLOSE( var1.variance( ith ), 6.9166666666667, 0.0000000001 );
    }
  }
  {
    // resize(using reindex) + merge
    utility::estimate_array var1( 100 );
    utility::estimate_array var2( 50 );
    std::vector< double > p1( 100 );
    std::copy( exemplar.data6.begin() + 50, exemplar.data6.end(), p1.begin() + 50 );
    var1.append( exemplar.data4.begin(), exemplar.data4.end() );
    var1.append( exemplar.data5.begin(), exemplar.data5.end() );
    var2.append( exemplar.data6.begin(), exemplar.data6.begin() + 50 );
  
    BOOST_CHECK_EQUAL( var1.size(), 100ul );
    BOOST_CHECK_EQUAL( var1.count(), 2ul );
    BOOST_CHECK_EQUAL( var2.size(), 50ul );
    BOOST_CHECK_EQUAL( var2.count(), 1ul );
  
    var2.reindex( 50, 0 );
    var2.append( p1.begin(), p1.end() );
    BOOST_CHECK_EQUAL( var2.count(), 2ul );
  
    var1.merge( var2 );
    BOOST_CHECK_EQUAL( var1.count(), 4ul );
    for( std::size_t ith = 0; ith != var1.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var1.mean( ith ), 3.75 );
      BOOST_CHECK_CLOSE( var1.variance( ith ), 6.9166666666667, 0.0000000001 );
    }
  }

}
BOOST_AUTO_TEST_CASE( estimate_array_data_collection )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TESTS FOR CORRECTNESS
    utility::estimate_array var( 100 );
  
    // Insure start from zero
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 100ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 0.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 0.0 );
    }
  
    // Add first data points (all 4.0)
    var.append( exemplar.data4.begin(), exemplar.data4.end() );
    BOOST_CHECK_EQUAL( var.count(), 1ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 4.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 0.0 );
    }
  
    // Add second data points (all 5.0)
    var.append( exemplar.data5.begin(), exemplar.data5.end() );
  
    BOOST_CHECK_EQUAL( var.count(), 2ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 4.5 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 0.5 );
    }
  
    // Add third data points (all 6.0)
    var.append( exemplar.data6.begin(), exemplar.data6.end() );
  
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
    }
  
    // Add fourth data points (from 1.0, 2.0, ... 100.0)
    var.append( exemplar.data_index.begin(), exemplar.data_index.end() );
  
    BOOST_CHECK_EQUAL( var.count(), 4ul );
    double val = 0.0;
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      auto sqr = []( double a )
      {
        return a*a;
      };
      const double mean( 3.75+val/4.0 );
      const double variance( ( sqr( 4-mean ) + sqr( 5-mean ) + sqr( 6-mean ) + sqr( val - mean ) )/3.0 );
      BOOST_CHECK( utility::feq( var.mean( ith ),mean ) );
      BOOST_CHECK( utility::feq( var.variance( ith ),variance ) );
      val += 1.0;
    }
  }

}
BOOST_AUTO_TEST_CASE( estimate_array_short_append_safe_test )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TESTS FOR CORRECTNESS
    utility::estimate_array var( 100 );
  
    // Add some data points
    var.append( exemplar.data4.begin(), exemplar.data4.end() );
    var.append( exemplar.data5.begin(), exemplar.data5.end() );
    var.append( exemplar.data6.begin(), exemplar.data6.end() );
  
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
    }
  
    // DATA TOO SHORT for ARRAY
  
    // Attempt to add first data points (all 4.0), should fail without
    // updating array.
    try
    {
      var.append_safe( exemplar.data4.begin(), exemplar.data4.end() - 10 );
      BOOST_ERROR( "Short data set was accepted without error." );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Input data size does not match and array size" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
    }
  }

}
BOOST_AUTO_TEST_CASE( estimate_array_short_append_test )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TESTS FOR CORRECTNESS
    utility::estimate_array var( 100 );
  
    // Add some data points
    var.append( exemplar.data4.begin(), exemplar.data4.end() );
    var.append( exemplar.data5.begin(), exemplar.data5.end() );
    var.append( exemplar.data6.begin(), exemplar.data6.end() );
  
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
    }
  
    // DATA TOO SHORT for ARRAY
  
    // Attempt to add first data points (all 4.0), should fail without
    // updating array.
    try
    {
      var.append( exemplar.data4.begin(), exemplar.data4.end() - 10 );
      BOOST_ERROR( "Short data set was accepted without error." );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Input data size does not match and array size" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
    BOOST_CHECK_EQUAL( var.count(), 4ul );
    for( std::size_t ith = 0; ith != var.size() - 10; ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 4.75 );
      BOOST_CHECK_CLOSE( var.variance( ith ), .91666666666666663, 0.0000001 );
    }
    for( std::size_t ith = var.size() - 10; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
      BOOST_CHECK_CLOSE( var.variance( ith ), 0.66666666666666663, 0.0000001 );
    }
  }

}
BOOST_AUTO_TEST_CASE( estimate_array_long_append_safe_test )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TESTS FOR CORRECTNESS
    utility::estimate_array var( 90 );
  
    // Add some data points
    var.append( exemplar.data4.begin(), exemplar.data4.end() - 10 );
    var.append( exemplar.data5.begin(), exemplar.data5.end() - 10 );
    var.append( exemplar.data6.begin(), exemplar.data6.end() - 10 );
  
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
    }
  
    // DATA TOO SHORT for ARRAY
  
    // Attempt to add first data points (all 4.0), should fail without
    // updating array.
    try
    {
      var.append_safe( exemplar.data4.begin(), exemplar.data4.end() );
      BOOST_ERROR( "Long data set was accepted without error." );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Input data size does not match and array size" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
    }
  }

}
BOOST_AUTO_TEST_CASE( estimate_array_long_append_test )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TESTS FOR CORRECTNESS
    utility::estimate_array var( 90 );
  
    // Add some data points
    var.append( exemplar.data4.begin(), exemplar.data4.end() - 10 );
    var.append( exemplar.data5.begin(), exemplar.data5.end() - 10 );
    var.append( exemplar.data6.begin(), exemplar.data6.end() - 10 );
  
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 5.0 );
      BOOST_CHECK_EQUAL( var.variance( ith ), 1.0 );
    }
  
    // DATA TOO SHORT for ARRAY
  
    // Attempt to add first data points (all 4.0), should fail without
    // updating array.
    try
    {
      var.append( exemplar.data4.begin(), exemplar.data4.end() );
      BOOST_ERROR( "Short data set was accepted without error." );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Input data size does not match and array size" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
    BOOST_CHECK_EQUAL( var.count(), 4ul );
    for( std::size_t ith = 0; ith != var.size() - 10; ++ith )
    {
      BOOST_CHECK_EQUAL( var.mean( ith ), 4.75 );
      BOOST_CHECK_CLOSE( var.variance( ith ), 0.91666666666666663, 0.00000001 );
    }
  }

}
BOOST_AUTO_TEST_CASE( digitizer_lifetime )
{
  {
    // Canonical pattern
    BOOST_CHECK( std::is_default_constructible< utility::digitizer >::type{} );
    BOOST_CHECK( std::is_copy_constructible< utility::digitizer >::type{} );
    BOOST_CHECK( std::is_move_constructible< utility::digitizer >::type{} );
    BOOST_CHECK( ( std::is_assignable< utility::digitizer, utility::digitizer >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< utility::digitizer >::type{} );
  }
  
  {
    // Default ctor
    utility::digitizer var;
    BOOST_CHECK_EQUAL( var.minimum(), 0.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 0.0 );
    BOOST_CHECK_EQUAL( var.size(), 0ul );
    for( int x = -10; x != 11; ++x )
    {
      BOOST_REQUIRE( not var.in_range( double( x )/5.0 ) );
    }
  }
  std::stringstream store;
  {
    // Min/Max/Count ctor
    utility::digitizer var( -1.0, 1.0, 21ul );
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.size(), 21ul );
    BOOST_CHECK( utility::feq( var.bin_width(), 0.095238095238095233 ) );
  
    boost::archive::text_oarchive oa(store);
    // write class instances to archive
    oa << var;
  
    {
      // copy
      utility::digitizer var2( var );
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      BOOST_CHECK( var == var2 );
    }
    {
      // assign
      utility::digitizer var2;
      BOOST_CHECK_EQUAL( var2.minimum(), 0.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 0.0 );
      var2 = var;
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      BOOST_CHECK( var == var2 );
    }
    {
      // move
      utility::digitizer var1( var );
      BOOST_CHECK_EQUAL( var1.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var1.maximum(), 1.0 );
      utility::digitizer var2( std::move( var1 ) );
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      BOOST_CHECK( var == var2 );
    }
  }
  {
    // Min/Max/Step
    utility::digitizer var( -1.0, 1.0, 0.1 );
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.bin_width(), 0.1 );
    BOOST_CHECK_EQUAL( var.size(), 20ul );
  }
  {
    utility::digitizer var;
   
    boost::archive::text_iarchive ia(store);
    // read class instances from archive
    ia >> var;
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.size(), 21ul );
    BOOST_CHECK( utility::feq( var.bin_width(), 0.095238095238095233 ) );
  } 
  

}
BOOST_AUTO_TEST_CASE( digitizer_bad_range_ctor1_test )
{
  // INVALID DIGITISER CTORS
  // Min/Max/Step Min>Max
  try
  {
    utility::digitizer var( 2.0,1.0,0.1 );
    BOOST_ERROR( "Able to construct digitizer with min > max" );
  }
  catch( std::runtime_error const& err )
  {
    const std::string msg( err.what() );
    BOOST_CHECK( msg.find( "Minimum of range must be less than the maximum" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  

}
BOOST_AUTO_TEST_CASE( digitizer_zero_range_ctor1_test )
{
  // INVALID DIGITISER CTORS
  // Min/Max/Step Min=Max
  try
  {
    utility::digitizer var( 1.0,1.0,0.1 );
    BOOST_ERROR( "Able to construct digitizer with min == max" );
  }
  catch( std::runtime_error const& err )
  {
    const std::string msg( err.what() );
    BOOST_CHECK( msg.find( "Minimum of range must be less than the maximum" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}
BOOST_AUTO_TEST_CASE( digitizer_zero_width_ctor1_test )
{
  // INVALID DIGITISER CTORS
  // Min/Max/Step Step=0
  try
  {
    utility::digitizer var( 1.0, 2.0, 0.0 );
    BOOST_ERROR( "Able to construct digitizer with step = 0" );
  }
  catch( std::runtime_error const& err )
  {
    const std::string msg( err.what() );
    BOOST_CHECK( msg.find( "Step size must be greater than zero" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}
BOOST_AUTO_TEST_CASE( digitizer_negative_width_ctor1_test )
{
  // INVALID DIGITISER CTORS
  // Min/Max/Step Step<0.0
  try
  {
    utility::digitizer var( 0.0, 1.0, -0.1 );
    BOOST_ERROR( "Able to construct digitizer with step = 0" );
  }
  catch( std::runtime_error const& err )
  {
    const std::string msg( err.what() );
    BOOST_CHECK( msg.find( "Step size must be greater than zero" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  

}
BOOST_AUTO_TEST_CASE( digitizer_bad_range_ctor2_test )
{
  // INVALID DIGITISER CTORS
  // Min/Max/Count Min>Max
  try
  {
    utility::digitizer var( 2.0,1.0,1ul );
    BOOST_ERROR( "Able to construct digitizer with min > max" );
  }
  catch( std::runtime_error const& err )
  {
    const std::string msg( err.what() );
    BOOST_CHECK( msg.find( "Minimum of range must be less than the maximum" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  

}
BOOST_AUTO_TEST_CASE( digitizer_zero_range_ctor2_test )
{
  // INVALID DIGITISER CTORS
  // Min/Max/Count Min=Max
  try
  {
    utility::digitizer var( 1.0,1.0, 1ul );
    BOOST_ERROR( "Able to construct digitizer with min == max" );
  }
  catch( std::runtime_error const& err )
  {
    const std::string msg( err.what() );
    BOOST_CHECK( msg.find( "Minimum of range must be less than the maximum" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}
BOOST_AUTO_TEST_CASE( digitizer_zero_count_ctor2_test )
{
  // INVALID DIGITISER CTORS
  // Min/Max/Step Step=0
  try
  {
    utility::digitizer var( -1.0,1.0, 0ul );
    BOOST_ERROR( "Able to construct digitizer with count = 0" );
  }
  catch( std::runtime_error const& err )
  {
    const std::string msg( err.what() );
    BOOST_CHECK( msg.find( "Can not have zero bin number" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}
// # test extend_up/down
BOOST_AUTO_TEST_CASE( digitizer_methods )
{
  {
    // Min/Max/Count ctor
    utility::digitizer var( -1.0,1.0,21ul );
    BOOST_CHECK( utility::feq( var.bin_width(), 0.095238095238095233 ) );
    for( int x = -10; x != 11; ++x )
    {
      if( not var.in_range( double( x )/5.0 ) )
      {
        if( -5 <= x and x < 5 )
        {
          std::stringstream ss;
          ss << double( x )/5.0 << " should be in range!";
          BOOST_ERROR( ss.str() );
        }
      }
      else
      {
        if( -5 > x or x >= 5 )
        {
          std::stringstream ss;
          ss << double( x )/5.0 << " should NOT be in range!";
          BOOST_ERROR( ss.str() );
        }
      }
    }
    // Min/Max/Count with literal
    utility::digitizer var2( -1.0, 1.0, 21 );
    BOOST_CHECK( var2.equivalent( var ) );
  }
  {
    // Min/Max/Step
    utility::digitizer var( -1.0,1.0,0.1 );
    BOOST_CHECK_EQUAL( var.size(), 20ul );
    for( int x = -10; x != 11; ++x )
    {
      if( not var.in_range( double( x )/5.0 ) )
      {
        if( -5 <= x and x < 5 )
        {
          std::stringstream ss;
          ss << double( x )/5.0 << " should be in range!";
          BOOST_ERROR( ss.str() );
        }
      }
      else
      {
        if( -5 > x or x >= 5 )
        {
          std::stringstream ss;
          ss << double( x )/5.0 << " should NOT be in range!";
          BOOST_ERROR( ss.str() );
        }
      }
    }
  }
  {
    // extend up/down
    utility::digitizer var( -1.0,1.0, 0.2 );
    BOOST_CHECK_EQUAL( var.size(), 10ul );
    std::vector< double > data;
    for( int x = 5; x != 10; ++x )
    {
      data.push_back( double( -x )/5.0 );
    }
    std::size_t count = 0;
    for( double x : data )
    {
      if( not var.in_range( double( x ) ) )
      {
        BOOST_REQUIRE_LT( x, var.minimum() );
        const utility::digitizer copy( var );
        const std::size_t add = var.extend_down( x );
        ++count;
        BOOST_CHECK_EQUAL( copy.size() + add, var.size() );
        BOOST_CHECK_LT( var.minimum(), copy.minimum() );
        BOOST_CHECK( utility::feq( copy.bin_width(), var.bin_width() ) );
        BOOST_CHECK( utility::feq( copy.maximum(), var.maximum() ) );
        BOOST_CHECK( utility::feq( copy.width() + add * copy.bin_width(), var.width() ) );
        BOOST_CHECK( utility::feq( copy.bin_minimum( 0 ), var.bin_minimum( add ) ) );
        BOOST_CHECK( utility::feq( copy.bin_maximum( 0 ), var.bin_maximum( add ) ) );
        BOOST_CHECK( utility::feq( copy.bin_midpoint( 0 ), var.bin_midpoint( add ) ) );
        BOOST_CHECK( utility::feq( copy.bin_width(), var.bin_width() ) );
      }
    }
    BOOST_CHECK_EQUAL( count, 4 );
    for( int x = 5; x != 10; ++x )
    {
      data[ x - 5 ] = double( x )/5.0;
    }
     for( double x : data )
    {
      if( not var.in_range( double( x ) ) )
      {
        BOOST_REQUIRE_GE( x, var.maximum() );
        const utility::digitizer copy( var );
        const std::size_t add = var.extend_up( x );
        ++count;
        BOOST_CHECK_EQUAL( copy.size() + add, var.size() );
        BOOST_CHECK( utility::feq( var.minimum(), copy.minimum() ) );
        BOOST_CHECK( utility::feq( copy.bin_width(), var.bin_width() ) );
        BOOST_CHECK_LT( copy.maximum(), var.maximum() );
        BOOST_CHECK( utility::feq( copy.width() + add * copy.bin_width(), var.width() ) );
        BOOST_CHECK( utility::feq( copy.bin_minimum( copy.size() - 1 ), var.bin_minimum(  copy.size() - 1 ) ) );
        BOOST_CHECK( utility::feq( copy.bin_maximum( copy.size() - 1 ), var.bin_maximum( copy.size() - 1 ) ) );
        BOOST_CHECK( utility::feq( copy.bin_midpoint( copy.size() - 1 ), var.bin_midpoint( copy.size() - 1 ) ) );
        BOOST_CHECK( utility::feq( copy.bin_width(), var.bin_width() ) );
      }
    }
    BOOST_CHECK_EQUAL( count, 8 );
    BOOST_CHECK( utility::feq( -2.0, var.minimum() ) );
    BOOST_CHECK( utility::feq( 1.8, var.maximum() ) );
     
  }
  

}
BOOST_AUTO_TEST_CASE( histogram_lifetime )
{
  {
    // Canonical pattern
    BOOST_CHECK( std::is_default_constructible< utility::histogram >::type{} );
    BOOST_CHECK( std::is_copy_constructible< utility::histogram >::type{} );
    BOOST_CHECK( std::is_move_constructible< utility::histogram >::type{} );
    BOOST_CHECK( ( std::is_assignable< utility::histogram, utility::histogram >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< utility::histogram >::type{} );
  }
  
  {
    // Default ctor
    utility::histogram var;
    BOOST_CHECK_EQUAL( var.minimum(), 0.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 0.0 );
    BOOST_CHECK_EQUAL( var.size(), 0ul );
    BOOST_CHECK_EQUAL( var.auto_extendable(), false );
    for( int x = -10; x != 11; ++x )
    {
      BOOST_REQUIRE( not var.in_range( double( x )/5.0 ) );
    }
  }
  std::stringstream store;
  {
    // Min/Max/Count ctor
    utility::histogram var( -1.0, 1.0, 21ul, true );
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.size(), 21ul );
    BOOST_CHECK_EQUAL( var.auto_extendable(), true );
    BOOST_CHECK( utility::feq( var.bin_width(), 0.095238095238095233 ) );
  
    boost::archive::text_oarchive oa(store);
    // write class instances to archive
    oa << var;
  
    {
      // copy
      utility::histogram var2( var );
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK_EQUAL( var2.auto_extendable(), true );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      // BOOST_CHECK( var == var2 );
    }
    {
      // assign
      utility::histogram var2;
      BOOST_CHECK_EQUAL( var2.minimum(), 0.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 0.0 );
      BOOST_CHECK_EQUAL( var2.auto_extendable(), false );
      var2 = var;
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK_EQUAL( var2.auto_extendable(), true );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      // BOOST_CHECK( var == var2 );
    }
    {
      // move
      utility::histogram var1( var );
      BOOST_CHECK_EQUAL( var1.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var1.maximum(), 1.0 );
      utility::histogram var2( std::move( var1 ) );
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK_EQUAL( var2.auto_extendable(), true );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      // BOOST_CHECK( var == var2 );
    }
  }
  {
    // Min/Max/Step
    utility::histogram var( -1.0, 1.0, 0.1 );
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.bin_width(), 0.1 );
    BOOST_CHECK_EQUAL( var.auto_extendable(), false );
    BOOST_CHECK_EQUAL( var.size(), 20ul );
  }
  {
    utility::histogram var;
   
    boost::archive::text_iarchive ia(store);
    // read class instances from archive
    ia >> var;
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.size(), 21ul );
    BOOST_CHECK_EQUAL( var.auto_extendable(), true );
    BOOST_CHECK( utility::feq( var.bin_width(), 0.095238095238095233 ) );
  } 

}
BOOST_AUTO_TEST_CASE( histogram_iterator_lifetime )
{
  {
    // Canonical pattern
    BOOST_CHECK( std::is_default_constructible< utility::histogram::bin_iterator >::type{} );
    BOOST_CHECK( std::is_copy_constructible< utility::histogram::bin_iterator >::type{} );
    BOOST_CHECK( std::is_move_constructible< utility::histogram::bin_iterator >::type{} );
    BOOST_CHECK( ( std::is_assignable< utility::histogram::bin_iterator, utility::histogram::bin_iterator >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< utility::histogram::bin_iterator >::type{} );
  }
  {
    // Default ctor
    utility::histogram::bin_iterator iter1;
    utility::histogram::bin_iterator iter2;
    BOOST_CHECK( iter1 == iter2 );
    BOOST_CHECK( not ( iter1 != iter2 ) );
    BOOST_CHECK( not ( iter1 < iter2 ) );
    BOOST_CHECK( iter1 <= iter2 );
    BOOST_CHECK( not ( iter1 > iter2 ) );
    BOOST_CHECK( iter1 >= iter2 );
    BOOST_CHECK_EQUAL( iter1 - iter2, 0L );
  
    // Min/Max/Count ctor
    utility::histogram var( -1.0, 1.0, 21ul, true );
    BOOST_CHECK_EQUAL( std::distance( var.begin(), var.end() ), 21L );
    {
      // copy
      utility::histogram::bin_iterator iter3( var.begin() );
      BOOST_CHECK( iter3 == var.begin() );
      BOOST_CHECK( iter3 != var.end() );
      BOOST_CHECK( iter3 < var.end() );
      BOOST_CHECK( iter3 <= var.end() );
      BOOST_CHECK( iter3 <= var.begin() );
      BOOST_CHECK( not( iter3 > var.begin() ) );
      BOOST_CHECK( iter3 >= var.begin() );
      ++iter3;
      BOOST_CHECK( iter3 > var.begin() );
      BOOST_CHECK( iter3 >= var.begin() );
      BOOST_CHECK( not( iter3 <= var.begin() ) );
      BOOST_CHECK( iter3 != var.begin() );
      BOOST_CHECK( iter3 != var.end() );
      BOOST_CHECK_EQUAL( *iter3, 0.0 );
      BOOST_CHECK( utility::feq( iter3.width(), 0.095238095238095233 ) );
      BOOST_CHECK_CLOSE( iter3.width(), 0.095238095238095233, 0.000000000001 );
      BOOST_CHECK( utility::feq( iter3.minimum(), -1.0 + 0.095238095238095233 ) );
      BOOST_CHECK_CLOSE( iter3.minimum(), -1.0 + 0.095238095238095233, 0.000000000001 );
      BOOST_CHECK( utility::feq( iter3.maximum(), -1.0 + 2*0.095238095238095233 ) );
      BOOST_CHECK_CLOSE( iter3.maximum(), -1.0 + 2*0.095238095238095233, 0.000000000001 );
      BOOST_CHECK( utility::feq( iter3.midpoint(), -1.0 + 1.5*0.095238095238095233 ) );
      BOOST_CHECK_CLOSE( iter3.midpoint(), -1.0 + 1.5*0.095238095238095233, 0.000000000001 );
      BOOST_CHECK( utility::feq( iter3.mean(), 0.0 ) );
      BOOST_CHECK( utility::feq( iter3.variance(), 0.0 ) );
    }
    {
      // assign
      utility::histogram::bin_iterator iter3;
      BOOST_CHECK( iter3 == iter1 );
      iter3 = var.begin();
      BOOST_CHECK( iter3 == var.begin() );
    }
    {
      // move
      utility::histogram::bin_iterator iter3( var.begin() );
      BOOST_CHECK( iter3 == var.begin() );
      utility::histogram::bin_iterator iter4( std::move( iter3 ) );
      BOOST_CHECK( iter4 == var.begin() );
    }
    for( auto iter = var.begin(); iter != var.end(); ++iter )
    {
      BOOST_CHECK_EQUAL( *iter, 0.0 );
    }
    for( auto val : var )
    {
      BOOST_CHECK_EQUAL( val, 0.0 );
    }
  }

}
BOOST_AUTO_TEST_CASE( histogram_data_collection )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TESTS FOR CORRECTNESS
    utility::histogram var( 3.5, 4.5, 5, true );
  
    // Insure start from zero
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 5ul );
    BOOST_CHECK_EQUAL( var.bin_width(), 0.2 );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.bin_mean( ith ), 0.0 );
      BOOST_CHECK_EQUAL( var.bin_variance( ith ), 0.0 );
    }
  
    // Add first data points (all 4.0)
    var.sample( exemplar.data4.begin(), exemplar.data4.end() );
    BOOST_CHECK_EQUAL( var.count(), 1ul );
    BOOST_CHECK_EQUAL( var.size(), 5ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.bin_mean( ith ), ( ith == 2 ? 100.0 : 0.0 ) );
      BOOST_CHECK_EQUAL( var.bin_variance( ith ), 0.0 );
    }
  
    // Add second data points (all 5.0)
    var.sample( exemplar.data5.begin(), exemplar.data5.end() );
  
    BOOST_CHECK_EQUAL( var.count(), 2ul );
    BOOST_REQUIRE_EQUAL( var.size(), 8ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_CLOSE( var.bin_mean( ith ), ( ith == 2 or ith == 7 ? 50.0 : 0.0 ), 0.000001 );
      BOOST_CHECK_CLOSE( var.bin_variance( ith ), ( ith == 2 or ith == 7 ? 5000.0 : 0.0 ), 0.000001 );
    }
  
    // Add third data points (all 6.0)
    var.sample( exemplar.data6.begin(), exemplar.data6.end() );
  
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    BOOST_REQUIRE_EQUAL( var.size(), 13ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_CLOSE( var.bin_mean( ith ), ( ith == 2 or ith == 7 or ith == 12 ? (100.0/3.0) : 0.0 ), 0.000001 );
      BOOST_CHECK_CLOSE( var.bin_variance( ith ), ( ith == 2 or ith == 7 or ith == 12 ? (10000.0/3.0) : 0.0 ), 0.000001 );
    }
  }

}
BOOST_AUTO_TEST_CASE( basic_histogram_lifetime )
{
  {
    // Canonical pattern
    BOOST_CHECK( std::is_default_constructible< utility::basic_histogram >::type{} );
    BOOST_CHECK( std::is_copy_constructible< utility::basic_histogram >::type{} );
    BOOST_CHECK( std::is_move_constructible< utility::basic_histogram >::type{} );
    BOOST_CHECK( ( std::is_assignable< utility::basic_histogram, utility::basic_histogram >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< utility::basic_histogram >::type{} );
  }
  {
    // Default ctor
    utility::basic_histogram var;
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 0ul );
    BOOST_CHECK_EQUAL( var.is_sampling(), false );
  }
  {
    // Size ctor
    utility::basic_histogram var( 5ul );
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 5ul );
    BOOST_CHECK_EQUAL( var.is_sampling(), false );
  }

}
BOOST_AUTO_TEST_CASE( basic_histogram_data_collection )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TESTS FOR CORRECTNESS
    utility::basic_histogram var( 10 );
  
    // Insure start from zero
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 10ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.bin_mean( ith ), 0.0 );
      BOOST_CHECK_EQUAL( var.bin_variance( ith ), 0.0 );
    }
  
    // Add first data points
    BOOST_CHECK( not var.is_sampling() );
    var.begin_sample();
    BOOST_CHECK( var.is_sampling() );
    {
      // digitizer function
      auto digtr = []( double val )
      {
        return std::size_t( std::floor( val/3.0 ) );
      };
      for( auto val : exemplar.var )
      {
        BOOST_CHECK_NO_THROW( var.increment_bin_safe( digtr( val ) ) );
      }
    }
    BOOST_CHECK( var.is_sampling() );
    var.end_sample();
    BOOST_CHECK( not var.is_sampling() );
  
    BOOST_CHECK_EQUAL( var.count(), 1ul );
    BOOST_CHECK_EQUAL( var.size(), 10ul );
    {
      std::vector< double > means { 17.0, 3.0, 5.0, 1.0, 2.0, 0.0, 1.0, 1.0, 1.0, 0.0 };
      for( std::size_t ith = 0; ith != var.size(); ++ith )
      {
        //std::cout << "[" << ith << "]: " << var.bin_mean( ith ) << "\n";
        BOOST_CHECK_EQUAL( var.bin_mean( ith ), means.at( ith ) );
        BOOST_CHECK_EQUAL( var.bin_variance( ith ), 0.0 );
      }
    }
    // Add second data points
    BOOST_CHECK( not var.is_sampling() );
    var.begin_sample();
    BOOST_CHECK( var.is_sampling() );
    {
      // digitizer function
      auto digtr = []( double val )
      {
        return std::size_t( std::ceil( val/3.0 ) );
      };
      for( auto val : exemplar.var )
      {
        BOOST_CHECK_NO_THROW( var.increment_bin_safe( digtr( val ) ) );
      }
    }
    BOOST_CHECK( var.is_sampling() );
    var.end_sample();
    BOOST_CHECK( not var.is_sampling() );
  
    BOOST_CHECK_EQUAL( var.count(), 2ul );
    BOOST_REQUIRE_EQUAL( var.size(), 10ul );
    {
      std::vector< double > means { 9.0, 9.5, 4.0, 3.0, 1.5, 1.0, 0.5, 1.0, 1.0,  0.5 };
      std::vector< double > vars { 128.0, 84.5, 2.0, 8.0, 0.5, 2.0, 0.5, 0.0, 0.0, 0.5 };
      for( std::size_t ith = 0; ith != var.size(); ++ith )
      {
        //std::cout << "[" << ith << "]: " << var.bin_mean( ith ) << "\n";
        BOOST_CHECK_EQUAL( var.bin_mean( ith ), means.at( ith ) );
        BOOST_CHECK_EQUAL( var.bin_variance( ith ), vars.at( ith ) );
      }
    }
  
    // Add third data points
    BOOST_CHECK( not var.is_sampling() );
    var.begin_sample();
    BOOST_CHECK( var.is_sampling() );
    {
      // digitizer function
      auto digtr = []( double val )
      {
        return std::size_t( std::ceil( std::sqrt( val + 0.01 ) ) );
      };
      for( auto val : exemplar.var )
      {
        BOOST_CHECK_NO_THROW( var.increment_bin_safe( digtr( val ) ) );
      }
    }
    BOOST_CHECK( var.is_sampling() );
    var.end_sample();
    BOOST_CHECK( not var.is_sampling() );
  
    BOOST_CHECK_EQUAL( var.count(), 3ul );
    BOOST_REQUIRE_EQUAL( var.size(), 10ul );
    {
      std::vector< double > means { 6.0, 11.666666666666667, 3.0, 4.66666666666666, 2.0, 1.3333333333333333, 0.6666666666666666, 0.6666666666666666, 0.6666666666666666,  0.3333333333333333 };
      std::vector< double > vars { 91.0, 56.33333333333333, 4.0, 12.33333333333333, 1.0, 1.3333333333333333, 0.3333333333333333, 0.3333333333333333, 0.3333333333333333, 0.3333333333333333 };
      for( std::size_t ith = 0; ith != var.size(); ++ith )
      {
        //std::cout << "[" << ith << "]: " << var.bin_mean( ith ) << "\n";
        BOOST_CHECK_CLOSE( var.bin_mean( ith ), means.at( ith ), 0.0000001 );
        BOOST_CHECK_CLOSE( var.bin_variance( ith ), vars.at( ith ), 0.0000001 );
      }
    }
  }
  
  

}
BOOST_AUTO_TEST_CASE( fixed_1d_histogram_lifetime )
{
  {
    // Canonical pattern
    BOOST_CHECK( std::is_default_constructible< utility::fixed_size_histogram< utility::digitizer > >::type{} );
    BOOST_CHECK( std::is_copy_constructible< utility::fixed_size_histogram< utility::digitizer > >::type{} );
    BOOST_CHECK( std::is_move_constructible< utility::fixed_size_histogram< utility::digitizer > >::type{} );
    BOOST_CHECK( ( std::is_assignable< utility::fixed_size_histogram< utility::digitizer >, utility::fixed_size_histogram< utility::digitizer > >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< utility::fixed_size_histogram< utility::digitizer > >::type{} );
  }
  
  {
    // Default ctor
    utility::fixed_size_histogram< utility::digitizer > var;
    BOOST_CHECK_EQUAL( var.minimum(), 0.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 0.0 );
    BOOST_CHECK_EQUAL( var.size(), 0ul );
    for( int x = -10; x != 11; ++x )
    {
      BOOST_REQUIRE( not var.in_range( double( x )/5.0 ) );
    }
  }
  std::stringstream store;
  {
    // Min/Max/Count ctor
    utility::fixed_size_histogram< utility::digitizer > var( { -1.0, 1.0, 21ul } );
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.size(), 21ul );
    BOOST_CHECK( utility::feq( var.bin_width(), 0.095238095238095233 ) );
  
    boost::archive::text_oarchive oa(store);
    // write class instances to archive
    oa << var;
  
    {
      // copy
      utility::fixed_size_histogram< utility::digitizer > var2( var );
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      // BOOST_CHECK( var == var2 );
    }
    {
      // assign
      utility::fixed_size_histogram< utility::digitizer > var2;
      BOOST_CHECK_EQUAL( var2.minimum(), 0.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 0.0 );
      var2 = var;
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      // BOOST_CHECK( var == var2 );
    }
    {
      // move
      utility::fixed_size_histogram< utility::digitizer > var1( var );
      BOOST_CHECK_EQUAL( var1.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var1.maximum(), 1.0 );
      utility::fixed_size_histogram< utility::digitizer > var2( std::move( var1 ) );
      BOOST_CHECK_EQUAL( var2.minimum(), -1.0 );
      BOOST_CHECK_EQUAL( var2.maximum(), 1.0 );
      BOOST_CHECK_EQUAL( var2.size(), 21ul );
      BOOST_CHECK( utility::feq( var2.bin_width(), 0.095238095238095233 ) );
      // BOOST_CHECK( var == var2 );
    }
  }
  {
    // Min/Max/Step
    utility::fixed_size_histogram< utility::digitizer > var( { -1.0, 1.0, 0.1 } );
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.bin_width(), 0.1 );
    BOOST_CHECK_EQUAL( var.size(), 20ul );
  }
  {
    utility::fixed_size_histogram< utility::digitizer > var;
   
    boost::archive::text_iarchive ia(store);
    // read class instances from archive
    ia >> var;
    BOOST_CHECK_EQUAL( var.minimum(), -1.0 );
    BOOST_CHECK_EQUAL( var.maximum(), 1.0 );
    BOOST_CHECK_EQUAL( var.size(), 21ul );
    BOOST_CHECK( utility::feq( var.bin_width(), 0.095238095238095233 ) );
  } 

}
BOOST_AUTO_TEST_CASE( fixed_1d_histogram_iterator_lifetime )
{
  {
    // Canonical pattern
    BOOST_CHECK( std::is_default_constructible< utility::fixed_size_histogram< utility::digitizer >::bin_iterator >::type{} );
    BOOST_CHECK( std::is_copy_constructible< utility::fixed_size_histogram< utility::digitizer >::bin_iterator >::type{} );
    BOOST_CHECK( std::is_move_constructible< utility::fixed_size_histogram< utility::digitizer >::bin_iterator >::type{} );
    BOOST_CHECK( ( std::is_assignable< utility::fixed_size_histogram< utility::digitizer >::bin_iterator, utility::fixed_size_histogram< utility::digitizer >::bin_iterator >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< utility::fixed_size_histogram< utility::digitizer >::bin_iterator >::type{} );
  }
  {
    // Default ctor
    utility::fixed_size_histogram< utility::digitizer >::bin_iterator iter1;
    utility::fixed_size_histogram< utility::digitizer >::bin_iterator iter2;
    BOOST_CHECK( iter1 == iter2 );
    BOOST_CHECK( not ( iter1 != iter2 ) );
    BOOST_CHECK( not ( iter1 < iter2 ) );
    BOOST_CHECK( iter1 <= iter2 );
    BOOST_CHECK( not ( iter1 > iter2 ) );
    BOOST_CHECK( iter1 >= iter2 );
    BOOST_CHECK_EQUAL( iter1 - iter2, 0L );
  
    // Min/Max/Count ctor
    utility::fixed_size_histogram< utility::digitizer > var( { -1.0, 1.0, 21ul } );
    BOOST_CHECK_EQUAL( std::distance( var.begin(), var.end() ), 21L );
    {
      // copy
      utility::fixed_size_histogram< utility::digitizer >::bin_iterator iter3( var.begin() );
      BOOST_CHECK( iter3 == var.begin() );
      BOOST_CHECK( iter3 != var.end() );
      BOOST_CHECK( iter3 < var.end() );
      BOOST_CHECK( iter3 <= var.end() );
      BOOST_CHECK( iter3 <= var.begin() );
      BOOST_CHECK( not( iter3 > var.begin() ) );
      BOOST_CHECK( iter3 >= var.begin() );
      ++iter3;
      BOOST_CHECK( iter3 > var.begin() );
      BOOST_CHECK( iter3 >= var.begin() );
      BOOST_CHECK( not( iter3 <= var.begin() ) );
      BOOST_CHECK( iter3 != var.begin() );
      BOOST_CHECK( iter3 != var.end() );
      BOOST_CHECK( utility::feq( iter3.width(), 0.095238095238095233 ) );
      BOOST_CHECK_CLOSE( iter3.width(), 0.095238095238095233, 0.000000000001 );
      BOOST_CHECK( utility::feq( iter3.minimum(), -1.0 + 0.095238095238095233 ) );
      BOOST_CHECK_CLOSE( iter3.minimum(), -1.0 + 0.095238095238095233, 0.000000000001 );
      BOOST_CHECK( utility::feq( iter3.maximum(), -1.0 + 2*0.095238095238095233 ) );
      BOOST_CHECK_CLOSE( iter3.maximum(), -1.0 + 2*0.095238095238095233, 0.000000000001 );
      BOOST_CHECK( utility::feq( iter3.midpoint(), -1.0 + 1.5*0.095238095238095233 ) );
      BOOST_CHECK_CLOSE( iter3.midpoint(), -1.0 + 1.5*0.095238095238095233, 0.000000000001 );
      BOOST_CHECK( utility::feq( iter3.mean(), 0.0 ) );
      BOOST_CHECK( utility::feq( iter3.variance(), 0.0 ) );
    }
    {
      // assign
      utility::fixed_size_histogram< utility::digitizer >::bin_iterator iter3;
      BOOST_CHECK( iter3 == iter1 );
      iter3 = var.begin();
      BOOST_CHECK( iter3 == var.begin() );
    }
    {
      // move
      utility::fixed_size_histogram< utility::digitizer >::bin_iterator iter3( var.begin() );
      BOOST_CHECK( iter3 == var.begin() );
      utility::fixed_size_histogram< utility::digitizer >::bin_iterator iter4( std::move( iter3 ) );
      BOOST_CHECK( iter4 == var.begin() );
    }
    for( auto iter = var.begin(); iter != var.end(); ++iter )
    {
      BOOST_CHECK_EQUAL( iter.mean(), 0.0 );
      BOOST_CHECK_EQUAL( (*iter).mean(), 0.0 );
      BOOST_CHECK_EQUAL( iter->mean(), 0.0 );
    }
    for( auto val : var )
    {
      BOOST_CHECK_EQUAL( val.mean(), 0.0 );
    }
  }

}
BOOST_AUTO_TEST_CASE( fixed_1d_histogram_data_collection )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // TESTS FOR CORRECTNESS
    utility::fixed_size_histogram< utility::digitizer > var( { 3.5, 4.5, 5 } );
  
    // Insure start from zero
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.size(), 5ul );
    BOOST_CHECK_EQUAL( var.bin_width(), 0.2 );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.bin_mean( ith ), 0.0 );
      BOOST_CHECK_EQUAL( var.bin_variance( ith ), 0.0 );
    }
  
    // Add first data points (all 4.0)
    var.sample( exemplar.data4.begin(), exemplar.data4.end() );
    BOOST_CHECK_EQUAL( var.count(), 1ul );
    BOOST_CHECK_EQUAL( var.size(), 5ul );
    for( std::size_t ith = 0; ith != var.size(); ++ith )
    {
      BOOST_CHECK_EQUAL( var.bin_mean( ith ), ( ith == 2 ? 100.0 : 0.0 ) );
      BOOST_CHECK_EQUAL( var.bin_variance( ith ), 0.0 );
    }
  }

}
// Test of the pattern_test test methods with basic types that should support
// the given patterns.
BOOST_AUTO_TEST_CASE( pattern_test_test )
{
  // test match to canonical pattern
  pattern_test::canonical< int, true, true >( 1, 2 );
  pattern_test::canonical< double, true, true >( 1.0, 2.0 );
  // test match to comparable pattern
  pattern_test::comparable< int >( 4, 2, 1, 2 );
  pattern_test::comparable< double >( 4.0, 2.0, 1.0, 2.0 );

}

//Series of tests to check estimater lifetime methods
BOOST_AUTO_TEST_CASE( binary_estimate_lifetime )
{
  estimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  {
    // TEST: default ctor
    utility::binary_estimate var1;
    //
    // Test that initial values are well-defined
    BOOST_REQUIRE(var1.count() == 0);
    BOOST_REQUIRE(utility::feq(var1.mean(), 0.0));
    BOOST_REQUIRE(utility::feq(var1.variance(), 0.0));
  }
  {
    utility::binary_estimate var1;
  
    // TEST: copy constructor on empty object
    utility::binary_estimate var2(var1);
  
    // var1 should be unchanged
    BOOST_CHECK_EQUAL(var1.count(), 0);
    BOOST_CHECK(utility::feq(var1.mean(), 0.0));
    BOOST_CHECK(utility::feq(var1.variance(), 0.0));
    // var2 should have the same data as var1
    BOOST_CHECK_EQUAL(var1.mean(), var2.mean());
    BOOST_CHECK_EQUAL(var1.variance(), var2.variance());
    BOOST_CHECK_EQUAL(var1.count(), var2.count());
    BOOST_CHECK( var1.equivalent( var2 ) );
  }
  {
    utility::binary_estimate var1;
    utility::binary_estimate var3;
    // Insert some values (using raw pointers)
    for (std::size_t idx = 0; idx != 15; ++idx)
    {
      if (1.0 > exemplar.var[idx])
      {
        var1.success();
      }
      else
      {
        var1.fail();
      }
      if (1.0 > exemplar.var[idx + 16])
      {
        var3.success();
      }
      else
      {
        var3.fail();
      }
    }
    BOOST_CHECK_NE(var1.mean(), var3.mean());
    BOOST_CHECK_NE(var1.variance(), var3.variance());
    BOOST_CHECK_EQUAL(var1.count(), var3.count());
   
    // Run canonical test
    pattern_test::canonical< utility::binary_estimate, false, true >( var1, var3 );
  
    // TEST: copy constructor on object with data
    utility::binary_estimate var2(var1);
  
    // var1 should be unchanged
    BOOST_CHECK_EQUAL(var1.count(), 15);
    // var2 should have the same data as var1
    BOOST_CHECK_EQUAL(var1.mean(), var2.mean());
    BOOST_CHECK_EQUAL(var1.variance(), var2.variance());
    BOOST_CHECK_EQUAL(var1.count(), var2.count());
    BOOST_CHECK( var1 == var2 );
  }
  {
    utility::binary_estimate var1;
    // Insert some values (using iterators)
    for (auto value : exemplar.var)
    {
      if (1.0 > value)
      {
        var1.success();
      }
      else
      {
        var1.fail();
      }
    } 
    BOOST_CHECK_EQUAL(var1.count(), 31);
    // TEST: assignment
    utility::binary_estimate var2;
    // insert some values
    for (std::size_t idx = 0; idx != 16; ++idx)
    {
      if (1.0 > exemplar.var[idx])
      {
        var2.success();
      }
      else
      {
        var2.fail();
      }
    }
    BOOST_CHECK( not ( var1 == var2 ) );
    BOOST_CHECK_EQUAL(var2.count(), 16);
    // assignment should overwrite existing data
    var2 = var1;
    BOOST_CHECK( var1 == var2 );
  
    // var1 should be unchanged
    BOOST_CHECK_EQUAL(var1.count(), 31);
    BOOST_CHECK_EQUAL(var2.count(), 31);
  }
  

}

BOOST_AUTO_TEST_CASE( basic_mean_lifetime )
{
  
  estimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  {
    // TEST: default ctor
    utility::basic_mean var1;
    //
    // Test that initial values are well-defined
    BOOST_REQUIRE(utility::feq(var1.mean(), 0.0));
    BOOST_REQUIRE(utility::feq(var1.variance(0), 0.0));
    // Test that initial values are well-defined for
    // invalid counts
    BOOST_REQUIRE(utility::feq(var1.mean(), 0.0));
    BOOST_REQUIRE(utility::feq(var1.variance(1), 0.0));
    BOOST_REQUIRE(utility::feq(var1.variance(2), 0.0));
  }
  {
    utility::basic_mean var1;
  
    // TEST: copy constructor on empty object
    utility::basic_mean var2(var1);
  
    // var1 should be unchanged
    BOOST_CHECK(utility::feq(var1.mean(), 0.0));
    BOOST_CHECK(utility::feq(var1.variance(0), 0.0));
    // var2 should have the same data as var1
    BOOST_CHECK_EQUAL(var1.mean(), var2.mean());
    BOOST_CHECK_EQUAL(var1.variance(0), var2.variance(0));
  }
  {
    utility::basic_mean var1;
    // Insert some values
    std::size_t count = 0;
    for (auto val : exemplar.var)
    {
      ++count;
      var1.append(val, count);
    }
  
    // TEST: copy constructor on object with data
    utility::basic_mean var2(var1);
  
    // var1 should be unchanged
    BOOST_CHECK(utility::feq(var1.mean(), exemplar.mean));
    BOOST_CHECK(utility::feq(var1.variance(31), exemplar.svar, 1<<9));
    // var2 should have the same data as var1
    BOOST_CHECK_EQUAL(var1.mean(), var2.mean());
    BOOST_CHECK_EQUAL(var1.variance(31), var2.variance(31));
  }
  {
    utility::basic_mean var1;
    // Insert some values
    for (std::size_t count = 0; count != exemplar.var.size(); ++count)
    {
      var1.append(exemplar.var[count], count + 1);
    }
    // assert match to canonical pattern
    {
      pattern_test::canonical< utility::basic_mean, false, true >( var1, {} );
    }
    // TEST: assignment
    utility::basic_mean var2;
    // insert some values
    const size_t sz (exemplar.var.size()/2);
    for (std::size_t count = 0; count != sz; ++count)
    {
      var2.append(exemplar.var[count], count + 1);
    }
    const double mean2 = var2.mean();
    const double varnc2 = var2.variance(2);
    // assignment should overwrite existing data
    var2 = var1;
  
    // var1 should be unchanged
    BOOST_CHECK_EQUAL(var1.mean(), exemplar.mean);
    BOOST_CHECK(utility::feq(var1.mean(), exemplar.mean));
    BOOST_CHECK(utility::feq(var1.variance(31), exemplar.svar, 1<<9));
    // var2 should have the same data as var1
    BOOST_CHECK_EQUAL(var1.mean(), var2.mean());
    BOOST_CHECK(var2.mean() != mean2);
    BOOST_CHECK_EQUAL(var1.variance(31), var2.variance(31));
    BOOST_CHECK(var2.variance(31) != varnc2);
  }
  {
    std::stringstream store;
    {
      utility::basic_mean var1;
      for (std::size_t count = 0; count != exemplar.var.size(); ++count)
      {
        var1.append(exemplar.var[count], count + 1);
      }
  
      boost::archive::text_oarchive oa(store);
      // write class instance to archive
      oa << var1;
      // var1 should be unchanged
      BOOST_CHECK(utility::feq(var1.mean(), exemplar.mean));
      BOOST_CHECK(utility::feq(var1.variance(31), exemplar.svar, 1<<9));
    }
    {
      utility::basic_mean var2;
      boost::archive::text_iarchive ia(store);
      // read class instance to archive
      ia >> var2;
      // var2 should have the same data as var1 had
      BOOST_CHECK(utility::feq(var2.mean(), exemplar.mean));
      BOOST_CHECK(utility::feq(var2.variance(31), exemplar.svar, 1<<9));
    }
  
  }

}

//Perform series of tests on the data collection routines of the
//estimater class
BOOST_AUTO_TEST_CASE( basic_mean_data_collection )
{
  estimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  {
    utility::basic_mean test_var;
    // TEST: default ctor
    //
    // Test that initial values are well-defined
    BOOST_REQUIRE(utility::feq(test_var.mean(), 0.0));
    BOOST_REQUIRE(utility::feq(test_var.variance(0), 0.0));
  }
  {
    utility::basic_mean test_var;
    std::size_t count = 0;
    // TEST: append method
    for (auto val : exemplar.var)
    {
      ++count;
      test_var.append(val, count);
    }
    BOOST_CHECK_EQUAL(count, 31);
    BOOST_CHECK(utility::feq(test_var.mean(), exemplar.mean));
    BOOST_CHECK(utility::feq(test_var.variance(count), exemplar.svar));
  
    // TEST: reset returns to well-defined values
    test_var.reset ();
  
    BOOST_CHECK(utility::feq(test_var.mean(), 0.0));
    BOOST_CHECK(utility::feq(test_var.variance(0), 0.0));
  
    // Check that reinserting value gives the
    // same results after the reset.
    count = 0;
    for (auto val : exemplar.var)
    {
      ++count;
      test_var.append(val, count);
    }
    BOOST_CHECK_EQUAL(count, 31);
    BOOST_CHECK(utility::feq(test_var.mean(), exemplar.mean));
    BOOST_CHECK(utility::feq(test_var.variance(count), exemplar.svar));
  
  }

}

BOOST_AUTO_TEST_CASE( estimate_2d_lifetime )
{
  estimater_test_fw const& exemplar( estimater_test_fw::exmplr );
  {
    // Default ctor
    utility::estimate_2d var;
  
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.shape()[0], 0ul );
    BOOST_CHECK_EQUAL( var.shape()[1], 0ul );
  
    // Resize array
    var.resize( {{4,25}} );
  
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.shape()[0], 4ul );
    BOOST_CHECK_EQUAL( var.shape()[1], 25ul );
  }
  {
    // Ctor with size array
    utility::estimate_2d var( utility::estimate_2d::index_type( {4,25} ) );
  
    BOOST_CHECK_EQUAL( var.count(), 0ul );
    BOOST_CHECK_EQUAL( var.shape()[0], 4ul);
    BOOST_CHECK_EQUAL( var.shape()[1], 25ul );
  }
  {
    // Test of copy and serialization
    std::stringstream store;
    {
      utility::estimate_2d var( utility::estimate_2d::index_type( {4,25} ) );
  
      var.append( exemplar.data4.begin(), exemplar.data4.end() );
      var.append( exemplar.data5.begin(), exemplar.data5.end() );
      var.append( exemplar.data6.begin(), exemplar.data6.end() );
  
      BOOST_CHECK_EQUAL(var.count(), 3ul);
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          BOOST_CHECK_EQUAL( var.mean( {{ith,jth}} ), 5.0 );
          BOOST_CHECK_EQUAL( var.variance( {{ith,jth}} ), 1.0 );
        }
      }
  
      utility::estimate_2d var_copy (var);
      BOOST_CHECK_EQUAL(var_copy.count(), 3ul);
      for (std::size_t ith = 0; ith != var_copy.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var_copy.shape()[1]; ++jth)
        {
          BOOST_CHECK_EQUAL( var_copy.mean(  {{ith,jth}}  ), 5.0 );
          BOOST_CHECK_EQUAL( var_copy.variance(  {{ith,jth}}  ), 1.0 );
        }
      }
  
      boost::archive::text_oarchive oa(store);
  
      // write class instance to archive
      oa << var;
      // var should be unchanged
      BOOST_CHECK_EQUAL(var.count(), 3ul);
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 5.0);
          BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 1.0);
        }
      }
  
    }
    {
      utility::estimate_2d var;
      BOOST_CHECK_EQUAL(var.count(), 0ul);
      BOOST_CHECK_EQUAL(var.shape()[0], 0ul);
      BOOST_CHECK_EQUAL(var.shape()[1], 0ul);
      boost::archive::text_iarchive ia(store);
      // read class instance to archive
      ia >> var;
      // var should have the same data as var1 had
      BOOST_CHECK_EQUAL(var.shape()[0], 4ul);
      BOOST_CHECK_EQUAL(var.shape()[1], 25ul);
      BOOST_CHECK_EQUAL(var.count(), 3ul);
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 5.0);
          BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 1.0);
        }
      }
    }
  }

}
BOOST_AUTO_TEST_CASE( estimate_2d_data_collection )
{
  estimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  {
    utility::estimate_2d var( utility::estimate_2d::index_type( {10,10} ) );
    BOOST_CHECK_EQUAL(var.size(), 100ul);
  
    // Insure start from zero
    BOOST_CHECK_EQUAL(var.count(), 0ul);
    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
    {
      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
      {
        BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 0.0);
        BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 0.0);
      }
    }
  
    // Add first data points (all 4.0)
    var.append (exemplar.data4.begin(), exemplar.data4.end());
    BOOST_CHECK_EQUAL(var.count(), 1ul);
    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
    {
      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
      {
        BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 4.0);
        BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 0.0);
      }
    }
  
    // Add second data points (all 5.0)
    var.append (exemplar.data5.begin(), exemplar.data5.end());
  
    BOOST_CHECK_EQUAL(var.count(), 2ul);
    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
    {
      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
      {
        BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 4.5);
        BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 0.5);
      }
    }
  
    // Add third data points (all 6.0)
    var.append (exemplar.data6.begin(), exemplar.data6.end());
  
    BOOST_CHECK_EQUAL(var.count(), 3ul);
    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
    {
      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
      {
        BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 5.0);
        BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 1.0);
      }
    }
  
    // Add fourth data points (from 1.0, 2.0, ... 100.0)
    var.append (exemplar.data_index.begin(), exemplar.data_index.end());
  
    BOOST_CHECK_EQUAL(var.count(), 4ul);
    double val = 0.0;
    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
    {
      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
      {
        auto sqr = [](double a)
        {
          return a*a;
        };
        const double mean(3.75+val/4.0);
        const double variance((sqr(4-mean) + sqr(5-mean) + sqr(6-mean) + sqr(val - mean))/3.0);
        BOOST_CHECK(utility::feq(var.mean( {{ith,jth}} ),mean));
        BOOST_CHECK(utility::feq(var.variance( {{ith,jth}} ),variance));
        val += 1.0;
      }
    }
  }
  {
    // DATA / ARRAY SIZE MISMATCH
    {
      // DATA SET TOO LONG
      utility::estimate_2d var( utility::estimate_2d::index_type( {10,9} ) );
      BOOST_CHECK_EQUAL(var.size(), 90ul);
      {
        auto last = var.append (exemplar.data4.begin(), exemplar.data4.end());
        BOOST_CHECK(last != exemplar.data4.end());
        BOOST_CHECK(last == exemplar.data4.begin() + var.size());
      }
      {
        auto last = var.append (exemplar.data5.begin(), exemplar.data5.end());
        BOOST_CHECK(last != exemplar.data5.end());
        BOOST_CHECK(last == exemplar.data5.begin() + var.size());
      }
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 4.5);
          BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 0.5);
        }
      }
    }
    {
      // DATA SET TOO SHORT
      utility::estimate_2d var( utility::estimate_2d::index_type( {10,11} ) );
      BOOST_CHECK_EQUAL(var.size(), 110ul);
      {
        auto last = var.append (exemplar.data4.begin(), exemplar.data4.end());
        BOOST_CHECK(last == exemplar.data4.end());
        BOOST_CHECK(last != exemplar.data4.begin() + var.size());
      }
      {
        auto last = var.append (exemplar.data5.begin(), exemplar.data5.end());
        BOOST_CHECK(last == exemplar.data5.end());
        BOOST_CHECK(last != exemplar.data5.begin() + var.size());
      }
      std::size_t counter = 0;
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth, ++counter)
        {
          if (counter < exemplar.data4.size())
          {
            BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 4.5);
            BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 0.5);
          }
          else
          {
            BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 0.0);
            BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 0.0);
          }
        }
      }
    }
  
  }
  

}
BOOST_AUTO_TEST_CASE( estimate_3d_lifetime )
{
  estimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  {
    // Default ctor
    utility::estimate_3d var;
  
    BOOST_CHECK_EQUAL(var.count(), 0ul);
    BOOST_CHECK_EQUAL(var.shape()[0], 0ul);
    BOOST_CHECK_EQUAL(var.shape()[1], 0ul);
    BOOST_CHECK_EQUAL(var.shape()[2], 0ul);
  
    // Resize array
    var.resize( {{4,5,5}} );
  
    BOOST_CHECK_EQUAL(var.count(), 0ul);
    BOOST_CHECK_EQUAL(var.shape()[0], 4ul);
    BOOST_CHECK_EQUAL(var.shape()[1], 5ul);
    BOOST_CHECK_EQUAL(var.shape()[2], 5ul);
  }
  {
    // Ctor with size array
    utility::estimate_3d var( utility::estimate_3d::index_type( {4,5,5} ) );
  
    BOOST_CHECK_EQUAL(var.count(), 0ul);
    BOOST_CHECK_EQUAL(var.shape()[0], 4ul);
    BOOST_CHECK_EQUAL(var.shape()[1], 5ul);
    BOOST_CHECK_EQUAL(var.shape()[2], 5ul);
  }
  {
    // Test of copy and serialization
    std::stringstream store;
    {
      utility::estimate_3d var( utility::estimate_3d::index_type( {4,5,5} ) );
  
      var.append (exemplar.data4.begin(), exemplar.data4.end());
      var.append (exemplar.data5.begin(), exemplar.data5.end());
      var.append (exemplar.data6.begin(), exemplar.data6.end());
  
      BOOST_CHECK_EQUAL(var.count(), 3ul);
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
          {
            BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 5.0);
            BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 1.0);
          }
        }
      }
  
      utility::estimate_3d var_copy(var);
      BOOST_CHECK_EQUAL(var_copy.count(), 3ul);
      for (std::size_t ith = 0; ith != var_copy.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var_copy.shape()[1]; ++jth)
        {
          for (std::size_t kth = 0; kth != var_copy.shape()[2]; ++kth)
          {
            BOOST_CHECK_EQUAL(var_copy.mean( {{ith,jth,kth}} ), 5.0);
            BOOST_CHECK_EQUAL(var_copy.variance( {{ith,jth,kth}} ), 1.0);
          }
        }
      }
  
      boost::archive::text_oarchive oa(store);
  
      // write class instance to archive
      oa << var;
      // var should be unchanged
      BOOST_CHECK_EQUAL(var.count(), 3ul);
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          for (std::size_t kth = 0; kth != var.shape()[1]; ++kth)
          {
            BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 5.0);
            BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 1.0);
          }
        }
      }
  
    }
    {
      utility::estimate_3d var;
      BOOST_CHECK_EQUAL(var.count(), 0ul);
      BOOST_CHECK_EQUAL(var.shape()[0], 0ul);
      BOOST_CHECK_EQUAL(var.shape()[1], 0ul);
      BOOST_CHECK_EQUAL(var.shape()[2], 0ul);
      boost::archive::text_iarchive ia(store);
      // read class instance to archive
      ia >> var;
      // var should have the same data as var1 had
      BOOST_CHECK_EQUAL(var.shape()[0], 4ul);
      BOOST_CHECK_EQUAL(var.shape()[1], 5ul);
      BOOST_CHECK_EQUAL(var.shape()[2], 5ul);
      BOOST_CHECK_EQUAL(var.count(), 3ul);
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          for (std::size_t kth = 0; kth != var.shape()[1]; ++kth)
          {
            BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 5.0);
            BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 1.0);
          }
        }
      }
    }
  }
  

}
BOOST_AUTO_TEST_CASE( estimate_3d_data_collection )
{
  estimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  utility::estimate_3d var( utility::estimate_3d::index_type( {5,5,4} ) );
  
  // Insure start from zero
  BOOST_CHECK_EQUAL(var.count(), 0ul);
  for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  {
    for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
    {
      for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
      {
        BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 0.0);
        BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 0.0);
      }
    }
  }
  
  // Add first data points (all 4.0)
  var.append (exemplar.data4.begin(), exemplar.data4.end());
  BOOST_CHECK_EQUAL(var.count(), 1ul);
  for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  {
    for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
    {
      for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
      {
        BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 4.0);
        BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 0.0);
      }
    }
  }
  
  // Add second data points (all 5.0)
  var.append (exemplar.data5.begin(), exemplar.data5.end());
  
  BOOST_CHECK_EQUAL(var.count(), 2ul);
  for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  {
    for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
    {
      for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
      {
        BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 4.5);
        BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 0.5);
      }
    }
  }
  
  // Add third data points (all 6.0)
  var.append (exemplar.data6.begin(), exemplar.data6.end());
  
  BOOST_CHECK_EQUAL(var.count(), 3ul);
  for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  {
    for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
    {
      for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
      {
        BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 5.0);
        BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 1.0);
      }
    }
  }
  
  // Add fourth data points (from 1.0, 2.0, ... 100.0)
  var.append (exemplar.data_index.begin(), exemplar.data_index.end());
  
  BOOST_CHECK_EQUAL(var.count(), 4ul);
  double val = 0.0;
  for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  {
    for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
    {
      for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
      {
        auto sqr = [](double a)
        {
          return a*a;
        };
        const double mean(3.75+val/4.0);
        const double variance((sqr(4-mean) + sqr(5-mean) + sqr(6-mean) + sqr(val - mean))/3.0);
        BOOST_CHECK(utility::feq(var.mean( {{ith,jth,kth}} ),mean));
        BOOST_CHECK(utility::feq(var.variance( {{ith,jth,kth}} ),variance));
        val += 1.0;
      }
    }
  }
  {
    // DATA / ARRAY SIZE MISMATCH
    {
      // DATA SET TOO LONG
      utility::estimate_3d var( utility::estimate_3d::index_type( {2,5,9} ) );
      BOOST_CHECK_EQUAL(var.size(), 90ul);
      {
        auto last = var.append (exemplar.data4.begin(), exemplar.data4.end());
        BOOST_CHECK(last != exemplar.data4.end());
        BOOST_CHECK(last == exemplar.data4.begin() + var.size());
      }
      {
        auto last = var.append (exemplar.data5.begin(), exemplar.data5.end());
        BOOST_CHECK(last != exemplar.data5.end());
        BOOST_CHECK(last == exemplar.data5.begin() + var.size());
      }
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
          {
            BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 4.5);
            BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 0.5);
          }
        }
      }
    }
    {
      // DATA SET TOO SHORT
      utility::estimate_3d var( utility::estimate_3d::index_type( {4,5,6} ) );
      BOOST_CHECK_EQUAL(var.size(), 120ul);
      {
        auto last = var.append (exemplar.data4.begin(), exemplar.data4.end());
        BOOST_CHECK(last == exemplar.data4.end());
        BOOST_CHECK(last != exemplar.data4.begin() + var.size());
      }
      {
        auto last = var.append (exemplar.data5.begin(), exemplar.data5.end());
        BOOST_CHECK(last == exemplar.data5.end());
        BOOST_CHECK(last != exemplar.data5.begin() + var.size());
      }
      std::size_t counter = 0;
      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
      {
        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
        {
          for (std::size_t kth = 0; kth != var.shape()[2]; ++kth, ++counter)
          {
            if (counter < exemplar.data4.size())
            {
              BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 4.5);
              BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 0.5);
            }
            else
            {
              BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 0.0);
              BOOST_CHECK_EQUAL(var.variance( {{ith,jth,kth}} ), 0.0);
            }
          }
        }
      }
    }
  }

}
BOOST_AUTO_TEST_CASE( digital_sampler )
{
  //XXestimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  //XX{
  //XX  // Default ctor
  //XX  utility::digital_sampler var;
  //XX
  //XX  BOOST_CHECK_EQUAL(var.size(), 0ul);
  //XX  BOOST_CHECK_EQUAL(var.get_digitizer().size(), 0ul);
  //XX}
  //XX{
  //XX  std::stringstream store;
  //XX  utility::digitizer axis (4.0,100.0,1.0);
  //XX  {
  //XX    // Ctor with digitizer
  //XX    utility::digital_sampler var (axis);
  //XX
  //XX    BOOST_CHECK_EQUAL(var.size(), axis.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer().size(), axis.size());
  //XX    for (std::size_t ith = 0; ith != var.size(); ++ith)
  //XX    {
  //XX      BOOST_CHECK_EQUAL(var[ith], 0ul);
  //XX    }
  //XX
  //XX    // Test of data insertion
  //XX    // ----------------------
  //XX
  //XX    // MANY POINTS WITH THE SAME VALUE
  //XX    var.insert (exemplar.data4.begin(), exemplar.data4.end());
  //XX    var.insert (exemplar.data5.begin(), exemplar.data5.end());
  //XX    var.insert (exemplar.data6.begin(), exemplar.data6.end());
  //XX    // SOME POINTS OUTSIDE RANGE
  //XX    var.insert (exemplar.data_index.begin(), exemplar.data_index.end());
  //XX
  //XX    {
  //XX      std::size_t ith = 0;
  //XX      for (auto smpl: var)
  //XX      {
  //XX        switch(ith)
  //XX        {
  //XX        case 0:
  //XX        case 1:
  //XX        case 2:
  //XX          BOOST_CHECK_EQUAL(smpl, 101ul);
  //XX          break;
  //XX        default:
  //XX          BOOST_CHECK_EQUAL(smpl, 1ul);
  //XX          break;
  //XX        }
  //XX        ++ith;
  //XX      }
  //XX    }
  //XX    {
  //XX      // Test of copy
  //XX      utility::digital_sampler var_copy (var);
  //XX      BOOST_CHECK_EQUAL(var_copy.size(), axis.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.get_digitizer().size(), axis.size());
  //XX      {
  //XX        std::size_t ith = 0;
  //XX        for (auto smpl: var_copy)
  //XX        {
  //XX          switch(ith)
  //XX          {
  //XX          case 0:
  //XX          case 1:
  //XX          case 2:
  //XX            BOOST_CHECK_EQUAL(smpl, 101ul);
  //XX            break;
  //XX          default:
  //XX            BOOST_CHECK_EQUAL(smpl, 1ul);
  //XX            break;
  //XX          }
  //XX          ++ith;
  //XX        }
  //XX      }
  //XX    }
  //XX    // Test of serialization
  //XX    boost::archive::text_oarchive oa(store);
  //XX    // write class instance to archive
  //XX    oa << var;
  //XX    // var should be unchanged
  //XX    BOOST_CHECK_EQUAL(var.size(), axis.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer().size(), axis.size());
  //XX    {
  //XX      std::size_t ith = 0;
  //XX      for (auto smpl: var)
  //XX      {
  //XX        switch(ith)
  //XX        {
  //XX        case 0:
  //XX        case 1:
  //XX        case 2:
  //XX          BOOST_CHECK_EQUAL(smpl, 101ul);
  //XX          break;
  //XX        default:
  //XX          BOOST_CHECK_EQUAL(smpl, 1ul);
  //XX          break;
  //XX        }
  //XX        ++ith;
  //XX      }
  //XX    }
  //XX  }
  //XX  {
  //XX    // Test of deserialization
  //XX    utility::digital_sampler var;
  //XX    boost::archive::text_iarchive ia(store);
  //XX    // read class instance from archive
  //XX    ia >> var;
  //XX    BOOST_CHECK_EQUAL(var.size(), axis.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer().size(), axis.size());
  //XX    for (std::size_t ith = 0; ith != var.size(); ++ith)
  //XX    {
  //XX      switch(ith)
  //XX      {
  //XX      case 0:
  //XX      case 1:
  //XX      case 2:
  //XX        BOOST_CHECK_EQUAL(var[ith], 101ul);
  //XX        break;
  //XX      default:
  //XX        BOOST_CHECK_EQUAL(var[ith], 1ul);
  //XX        break;
  //XX      }
  //XX    }
  //XX
  //XX    // Test of RESET
  //XX    var.reset ();
  //XX    BOOST_CHECK_EQUAL(var.size(), axis.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer().size(), axis.size());
  //XX    for (std::size_t ith = 0; ith != var.size(); ++ith)
  //XX    {
  //XX      BOOST_CHECK_EQUAL(var[ith], 0ul);
  //XX    }
  //XX  }
  //XX}
  //XX

}
BOOST_AUTO_TEST_CASE( digital_2d_sampler )
{
  //XXestimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  //XX{
  //XX  // Default ctor
  //XX  utility::digital_2d_sampler var;
  //XX
  //XX  BOOST_CHECK_EQUAL(var.size(), 0ul);
  //XX  BOOST_CHECK_EQUAL(var.shape()[0], 0ul);
  //XX  BOOST_CHECK_EQUAL(var.shape()[1], 0ul);
  //XX  BOOST_CHECK_EQUAL(var.get_digitizer(0).size(), 0ul);
  //XX  BOOST_CHECK_EQUAL(var.get_digitizer(1).size(), 0ul);
  //XX}
  //XX{
  //XX  std::stringstream store;
  //XX  utility::digitizer axis0 (4.0,10.0,1.0);
  //XX  utility::digitizer axis1 (4.0,11.0,1.0);
  //XX  {
  //XX    // Ctor with digitizer
  //XX    utility::digital_2d_sampler var( std::array< utility::digitizer, 2ul >( {axis0,axis1} ) );
  //XX
  //XX    BOOST_CHECK_EQUAL(var.size(), axis0.size()*axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[0], axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[1], axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(0).size(), axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(1).size(), axis1.size());
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX      {
  //XX        BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 0ul);
  //XX      }
  //XX    // Test of data insertion
  //XX    // ----------------------
  //XX    // MANY POINTS WITH THE SAME VALUE
  //XX    typedef std::array< double, 2 > pt;
  //XX    std::vector< pt > dataset(100);
  //XX    for (std::size_t i = 0; i != dataset.size(); ++i)
  //XX    {
  //XX      dataset[i][0] = exemplar.data4[i];
  //XX      dataset[i][1] = exemplar.data5[i];
  //XX    }
  //XX    var.insert (dataset.begin(), dataset.end());
  //XX
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX      {
  //XX        if (ith == 0 and jth == 1)
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 100ul);
  //XX        else
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 0ul);
  //XX      }
  //XX
  //XX    for (std::size_t i = 0; i != dataset.size(); ++i)
  //XX    {
  //XX      dataset[i][0] = exemplar.data6[i];
  //XX      dataset[i][1] = exemplar.data_index[i];
  //XX    }
  //XX    var.insert (dataset.begin(), dataset.end());
  //XX
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX      {
  //XX        if (ith == 0 and jth == 1)
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 100ul);
  //XX        else if (ith == 2)
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 1ul);
  //XX        else
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 0ul);
  //XX      }
  //XX
  //XX    {
  //XX      // Test of copy
  //XX      utility::digital_2d_sampler var_copy (var);
  //XX      BOOST_CHECK_EQUAL(var_copy.size(), axis0.size()*axis1.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.shape()[0], axis0.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.shape()[1], axis1.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.get_digitizer(0).size(), axis0.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.get_digitizer(1).size(), axis1.size());
  //XX      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX        {
  //XX          if (ith == 0 and jth == 1)
  //XX            BOOST_CHECK_EQUAL((var_copy[ {{ith,jth}} ]), 100ul);
  //XX          else if (ith == 2)
  //XX            BOOST_CHECK_EQUAL((var_copy[ {{ith,jth}} ]), 1ul);
  //XX          else
  //XX            BOOST_CHECK_EQUAL((var_copy[ {{ith,jth}} ]), 0ul);
  //XX        }
  //XX    }
  //XX    // Test of serialization
  //XX    boost::archive::text_oarchive oa(store);
  //XX    // write class instance to archive
  //XX    oa << var;
  //XX  }
  //XX  {
  //XX    // Test of deserialization
  //XX    utility::digital_2d_sampler var;
  //XX    boost::archive::text_iarchive ia(store);
  //XX    // read class instance from archive
  //XX    ia >> var;
  //XX    BOOST_CHECK_EQUAL(var.size(), axis0.size()*axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[0], axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[1], axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(0).size(), axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(1).size(), axis1.size());
  //XX
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX      {
  //XX        if (ith == 0 and jth == 1)
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 100ul);
  //XX        else if (ith == 2)
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 1ul);
  //XX        else
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 0ul);
  //XX      }
  //XX
  //XX    // Test of RESET
  //XX    var.reset ();
  //XX    BOOST_CHECK_EQUAL(var.size(), axis0.size()*axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[0], axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[1], axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(0).size(), axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(1).size(), axis1.size());
  //XX
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX      {
  //XX        BOOST_CHECK_EQUAL((var[ {{ith,jth}} ]), 0ul);
  //XX      }
  //XX  }
  //XX}
  //XX

}
BOOST_AUTO_TEST_CASE( digital_3d_sampler )
{
  //XXestimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  //XX{
  //XX  // Default ctor
  //XX  utility::digital_3d_sampler var;
  //XX
  //XX  BOOST_CHECK_EQUAL(var.size(), 0ul);
  //XX  BOOST_CHECK_EQUAL(var.shape()[0], 0ul);
  //XX  BOOST_CHECK_EQUAL(var.shape()[1], 0ul);
  //XX  BOOST_CHECK_EQUAL(var.shape()[2], 0ul);
  //XX  BOOST_CHECK_EQUAL(var.get_digitizer(0).size(), 0ul);
  //XX  BOOST_CHECK_EQUAL(var.get_digitizer(1).size(), 0ul);
  //XX  BOOST_CHECK_EQUAL(var.get_digitizer(2).size(), 0ul);
  //XX}
  //XX{
  //XX  std::stringstream store;
  //XX  utility::digitizer axis0 (4.0,10.0,1.0);
  //XX  utility::digitizer axis1 (4.0,11.0,1.0);
  //XX  utility::digitizer axis2 (4.0,12.0,1.0);
  //XX  {
  //XX    // Ctor with digitizer
  //XX    utility::digital_3d_sampler var( std::array< utility::digitizer, 3ul >( {axis0,axis1,axis2} ) );
  //XX
  //XX    BOOST_CHECK_EQUAL(var.size(), axis0.size()*axis1.size()*axis2.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[0], axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[1], axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[2], axis2.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(0).size(), axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(1).size(), axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(2).size(), axis2.size());
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX        for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
  //XX        {
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 0ul);
  //XX        }
  //XX    // Test of data insertion
  //XX    // ----------------------
  //XX    // MANY POINTS WITH THE SAME VALUE
  //XX    typedef std::array< double, 3 > pt;
  //XX    std::vector< pt > dataset(100);
  //XX    for (std::size_t i = 0; i != dataset.size(); ++i)
  //XX    {
  //XX      dataset[i][0] = exemplar.data4[i];
  //XX      dataset[i][1] = exemplar.data5[i];
  //XX      dataset[i][2] = exemplar.data6[i];
  //XX    }
  //XX    var.insert (dataset.begin(), dataset.end());
  //XX
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX        for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
  //XX        {
  //XX          if (ith == 0 and jth == 1 and kth == 2)
  //XX            BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 100ul);
  //XX          else
  //XX            BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 0ul);
  //XX        }
  //XX
  //XX    for (std::size_t i = 0; i != dataset.size(); ++i)
  //XX    {
  //XX      dataset[i][0] = exemplar.data5[i];
  //XX      dataset[i][1] = exemplar.data6[i];
  //XX      dataset[i][2] = exemplar.data_index[i];
  //XX    }
  //XX    var.insert (dataset.begin(), dataset.end());
  //XX
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX        for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
  //XX        {
  //XX          if (ith == 0 and jth == 1 and kth == 2)
  //XX            BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 100ul);
  //XX          else if (ith == 1 and jth == 2)
  //XX            BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 1ul);
  //XX          else
  //XX            BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 0ul);
  //XX        }
  //XX
  //XX    {
  //XX      // Test of copy
  //XX      utility::digital_3d_sampler var_copy (var);
  //XX      BOOST_CHECK_EQUAL(var_copy.size(), axis0.size()*axis1.size()*axis2.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.shape()[0], axis0.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.shape()[1], axis1.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.shape()[2], axis2.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.get_digitizer(0).size(), axis0.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.get_digitizer(1).size(), axis1.size());
  //XX      BOOST_CHECK_EQUAL(var_copy.get_digitizer(2).size(), axis2.size());
  //XX      for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX        for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX          for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
  //XX          {
  //XX            if (ith == 0 and jth == 1 and kth == 2)
  //XX              BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 100ul);
  //XX            else if (ith == 1 and jth == 2)
  //XX              BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 1ul);
  //XX            else
  //XX              BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 0ul);
  //XX          }
  //XX    }
  //XX    // Test of serialization
  //XX    boost::archive::text_oarchive oa(store);
  //XX    // write class instance to archive
  //XX    oa << var;
  //XX  }
  //XX  {
  //XX    // Test of deserialization
  //XX    utility::digital_3d_sampler var;
  //XX    boost::archive::text_iarchive ia(store);
  //XX    // read class instance from archive
  //XX    ia >> var;
  //XX    BOOST_CHECK_EQUAL(var.size(), axis0.size()*axis1.size()*axis2.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[0], axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[1], axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[2], axis2.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(0).size(), axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(1).size(), axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(2).size(), axis2.size());
  //XX
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX        for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
  //XX        {
  //XX          if (ith == 0 and jth == 1 and kth == 2)
  //XX            BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 100ul);
  //XX          else if (ith == 1 and jth == 2)
  //XX            BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 1ul);
  //XX          else
  //XX            BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 0ul);
  //XX        }
  //XX
  //XX    // Test of RESET
  //XX    var.reset ();
  //XX    BOOST_CHECK_EQUAL(var.size(), axis0.size()*axis1.size()*axis2.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[0], axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[1], axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.shape()[2], axis2.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(0).size(), axis0.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(1).size(), axis1.size());
  //XX    BOOST_CHECK_EQUAL(var.get_digitizer(2).size(), axis2.size());
  //XX
  //XX    for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX      for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX        for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
  //XX        {
  //XX          BOOST_CHECK_EQUAL((var[ {{ith,jth,kth}} ]), 0ul);
  //XX        }
  //XX  }
  //XX}
  //XX
  //XX

}
BOOST_AUTO_TEST_CASE( sample_2d_collection )
{
  //XXestimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  //XX{
  //XX  std::stringstream store;
  //XX  utility::digitizer axis0 (0.0,10.0,1.0);
  //XX  utility::digitizer axis1 (1.0,11.0,1.0);
  //XX  utility::estimate_2d var( utility::estimate_2d::index_type( {axis0.size(), axis1.size()} ) );
  //XX
  //XX  // Insure start from zero
  //XX  BOOST_CHECK_EQUAL(var.size(), axis0.size()*axis1.size());
  //XX  BOOST_CHECK_EQUAL(var.count(), 0ul);
  //XX  for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX  {
  //XX    for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX    {
  //XX      BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 0.0);
  //XX      BOOST_CHECK_EQUAL(var.variance( {{ith,jth}} ), 0.0);
  //XX    }
  //XX  }
  //XX  utility::digital_2d_sampler smpl( std::array< utility::digitizer, 2ul >( {axis0,axis1} ) );
  //XX
  //XX  // ----------------------
  //XX  // MANY POINTS WITH THE SAME VALUE
  //XX  typedef std::array< double, 2 > pt;
  //XX  std::vector< pt > dataset(100);
  //XX  for (std::size_t i = 0; i != dataset.size(); ++i)
  //XX  {
  //XX    dataset[i][0] = exemplar.data4[i];
  //XX    dataset[i][1] = exemplar.data5[i];
  //XX  }
  //XX  smpl.insert (dataset.begin(), dataset.end());
  //XX
  //XX  var.append (smpl.begin(), smpl.end());
  //XX
  //XX  smpl.reset ();
  //XX  for (std::size_t i = 0; i != dataset.size(); ++i)
  //XX  {
  //XX    dataset[i][0] = exemplar.data6[i];
  //XX    dataset[i][1] = exemplar.data_index[i];
  //XX  }
  //XX  smpl.insert (dataset.begin(), dataset.end());
  //XX
  //XX  var.append (smpl.begin(), smpl.end());
  //XX  for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX    for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX    {
  //XX      if (ith == 4 and jth == 4)
  //XX        BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 50.0);
  //XX      else if (ith == 6)
  //XX        BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 0.5);
  //XX      else
  //XX        BOOST_CHECK_EQUAL(var.mean( {{ith,jth}} ), 0.0);
  //XX    }
  //XX}
  //XX

}
BOOST_AUTO_TEST_CASE( sample_3d_collection )
{
  //XXestimater_test_fw const& exemplar(estimater_test_fw::exmplr);
  //XX{
  //XX  utility::digitizer axis0 (4.0,10.0,1.0);
  //XX  utility::digitizer axis1 (4.0,11.0,1.0);
  //XX  utility::digitizer axis2 (4.0,12.0,1.0);
  //XX  // Ctor with digitizer
  //XX  utility::digital_3d_sampler smpl( std::array< utility::digitizer, 3ul >( {axis0,axis1,axis2} ) );
  //XX  utility::estimate_3d var( utility::estimate_3d::index_type( {axis0.size(),axis1.size(),axis2.size()} ) );
  //XX
  //XX
  //XX  // Test of data insertion
  //XX  // ----------------------
  //XX  // MANY POINTS WITH THE SAME VALUE
  //XX  typedef std::array< double, 3 > pt;
  //XX  std::vector< pt > dataset(100);
  //XX  for (std::size_t i = 0; i != dataset.size(); ++i)
  //XX  {
  //XX    dataset[i][0] = exemplar.data4[i];
  //XX    dataset[i][1] = exemplar.data5[i];
  //XX    dataset[i][2] = exemplar.data6[i];
  //XX  }
  //XX  smpl.insert (dataset.begin(), dataset.end());
  //XX
  //XX  var.append (smpl.begin(), smpl.end ());
  //XX
  //XX  smpl.reset ();
  //XX  for (std::size_t i = 0; i != dataset.size(); ++i)
  //XX  {
  //XX    dataset[i][0] = exemplar.data5[i];
  //XX    dataset[i][1] = exemplar.data6[i];
  //XX    dataset[i][2] = exemplar.data_index[i];
  //XX  }
  //XX  smpl.insert (dataset.begin(), dataset.end());
  //XX  var.append (smpl.begin(), smpl.end ());
  //XX
  //XX  for (std::size_t ith = 0; ith != var.shape()[0]; ++ith)
  //XX    for (std::size_t jth = 0; jth != var.shape()[1]; ++jth)
  //XX      for (std::size_t kth = 0; kth != var.shape()[2]; ++kth)
  //XX      {
  //XX        if (ith == 0 and jth == 1 and kth == 2)
  //XX          BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 50.0);
  //XX        else if (ith == 1 and jth == 2)
  //XX          BOOST_CHECK_EQUAL(var.mean( {{ith,jth,kth}} ), 0.5);
  //XX        else
  //XX          BOOST_CHECK_EQUAL(var.mean ( {{ith,jth,kth}} ), 0.0);
  //XX      }
  //XX}
  //XX
  //XX

}
