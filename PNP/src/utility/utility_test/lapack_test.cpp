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

#define BOOST_TEST_MODULE lapack_test
#include <boost/test/unit_test.hpp>

#include "utility/utility_test/lapack_test.hpp"
#include "utility/random.hpp"
#include "utility/fuzzy_equals.hpp"
#include "utility/lapack.hpp"

// manual includes
#include <boost/timer/timer.hpp>
// -
//Test the lapack routines.
BOOST_AUTO_TEST_CASE( lapack_version )
{
  // utility::fp_env &env(utility::fp_env::env_);
  // utility::fp_env::fp_nonstop_scope scoper;
  // BOOST_CHECK_EQUAL(0, env.except());
  
  BOOST_CHECK_EQUAL( utility::lapack::version(), " OpenBLAS 0.2.19 " );

}

//Test the lapack routines using C style arrays
BOOST_AUTO_TEST_CASE( dgetrf_dgetrs_c_test )
{
  // WE HAVE THREE DGETRF FUNCTIONS TO TEST
  double var[ 3 ][ 3 ];
  utility::lapack::int_type n{ 3 }, m{ 3 }, lda{ 3 };
  utility::lapack::int_type ipiv[ 3 ]{ 0, 1, 2 };
  utility::lapack::int_type info{ 1 };
  bool row_major{ true };
  
  var[0][0] = 1.0;
  var[0][1] = 2.0;
  var[0][2] = 3.0;
  
  var[1][0] = 4.0;
  var[1][1] = 5.0;
  var[1][2] = 6.0;
  
  var[2][0] = 7.0;
  var[2][1] = 8.0;
  var[2][2] = 0.0;
  //
  //  Factor the matrix.
  //
  //  for(std::size_t ii = 0; ii != 3ul; ++ii)
  //  {
  //    for(std::size_t jj = 0; jj != 3ul; ++jj)
  //    {
  //      std::cout << "  A["<< ii << "," << jj << "] = " << var[ii][jj];
  //    }
  //    std::cout << "\n";
  //  }
  //  std::cout << ipiv[0] << " " << ipiv[1] << " " << ipiv[2] << "\n";
  info = utility::lapack::dgetrf( row_major, m, n, &var[0][0], lda, &ipiv[0] );
  BOOST_CHECK_EQUAL( info, 0 );
  //  for(std::size_t ii = 0; ii != 3ul; ++ii)
  //  {
  //    for(std::size_t jj = 0; jj != 3ul; ++jj)
  //    {
  //      std::cout << "  A["<< ii << "," << jj << "] = " << var[ii][jj];
  //  
  //    }
  //    std::cout << "\n";
  //  }
  //  std::cout << ipiv[0] << " " << ipiv[1] << " " << ipiv[2] << "\n";
  //  
  double b[ 3 ] { 14.0, 32.0, 23.0 };
  double result[ 3 ] { 1.0, 2.0, 3.0 };
  
  //  std::cout << b[0] << " " << b[1] << " " << b[2] << "\n";
  info = utility::lapack::dgetrs( row_major, n, 1, &var[0][0], lda, &ipiv[0], &b[0], m );
  BOOST_CHECK_EQUAL( info, 0 );
  //  std::cout << b[0] << " " << b[1] << " " << b[2] << "\n";
  for( std::size_t ii{ 0 }; ii != 3ul; ++ii )
  {
    BOOST_CHECK( utility::feq( b[ii], result[ii] ) );
  }

}

//Test the lapack routines using C++ multi_array and vector pivot
BOOST_AUTO_TEST_CASE( dgetrf_matrix_vector_test )
{
  // WE HAVE THREE DGETRF FUNCTIONS TO TEST
  typedef std::array< std::size_t, 2 > index_type;
  boost::multi_array< double, 2 > var( index_type{ 3, 3 } );
  std::vector< utility::lapack::int_type > ipiv{ { 0, 1, 2 } };
  utility::lapack::int_type info{ 1 };
  bool row_major{ true };
  
  var( index_type{0,0} ) = 1.0;
  var( index_type{0,1} ) = 2.0;
  var( index_type{0,2} ) = 3.0;
  
  var( index_type{1,0} ) = 4.0;
  var( index_type{1,1} ) = 5.0;
  var( index_type{1,2} ) = 6.0;
  
  var( index_type{2,0} ) = 7.0;
  var( index_type{2,1} ) = 8.0;
  var( index_type{2,2} ) = 0.0;
  //
  //  Factor the matrix.
  //
  //  for(std::size_t ii = 0; ii != 3ul; ++ii)
  //  {
  //    for(std::size_t jj = 0; jj != 3ul; ++jj)
  //    {
  //      std::cout << "  A["<< ii << "," << jj << "] = " << var( index_type{ii,jj} );
  //    }
  //    std::cout << "\n";
  //  }
  //  std::cout << ipiv[0] << " " << ipiv[1] << " " << ipiv[2] << "\n";
  info = utility::lapack::dgetrf( row_major, var, ipiv );
  BOOST_CHECK_EQUAL( info, 0 );
  //  for(std::size_t ii = 0; ii != 3ul; ++ii)
  //  {
  //    for(std::size_t jj = 0; jj != 3ul; ++jj)
  //    {
  //      std::cout << "  A["<< ii << "," << jj << "] = " << var( index_type{ii,jj} );
  //  
  //    }
  //    std::cout << "\n";
  //  }
  //  std::cout << ipiv[0] << " " << ipiv[1] << " " << ipiv[2] << "\n";
  //  
  std::vector< double > b{ { 14.0, 32.0, 23.0 } };
  std::vector< double > result{ { 1.0, 2.0, 3.0 } };
  
  //  std::cout << b[0] << " " << b[1] << " " << b[2] << "\n";
  info = utility::lapack::dgetrs( row_major, var, ipiv, b );
  BOOST_CHECK_EQUAL( info, 0 );
  //  std::cout << b[0] << " " << b[1] << " " << b[2] << "\n";
  for( std::size_t ii{ 0 }; ii != 3ul; ++ii )
  {
    BOOST_CHECK( utility::feq( b[ii], result[ii] ) );
  }

}

//Test the lapack routines using C++ multi_array and vector pivot
BOOST_AUTO_TEST_CASE( dgetrf_matrix_valarray_test )
{
  // WE HAVE THREE DGETRF FUNCTIONS TO TEST
  typedef std::array< std::size_t, 2 > index_type;
  boost::multi_array< double, 2 > var( index_type{ 3, 3 } );
  std::vector< utility::lapack::int_type > ipiv{ { 0, 1, 2 } };
  utility::lapack::int_type info{ 1 };
  bool row_major{ true };
  
  var( index_type{0,0} ) = 1.0;
  var( index_type{0,1} ) = 2.0;
  var( index_type{0,2} ) = 3.0;
  
  var( index_type{1,0} ) = 4.0;
  var( index_type{1,1} ) = 5.0;
  var( index_type{1,2} ) = 6.0;
  
  var( index_type{2,0} ) = 7.0;
  var( index_type{2,1} ) = 8.0;
  var( index_type{2,2} ) = 0.0;
  //
  //  Factor the matrix.
  //
  //  for(std::size_t ii = 0; ii != 3ul; ++ii)
  //  {
  //    for(std::size_t jj = 0; jj != 3ul; ++jj)
  //    {
  //      std::cout << "  A["<< ii << "," << jj << "] = " << var( index_type{ii,jj} );
  //    }
  //    std::cout << "\n";
  //  }
  //  std::cout << ipiv[0] << " " << ipiv[1] << " " << ipiv[2] << "\n";
  info = utility::lapack::dgetrf( row_major, var, ipiv );
  BOOST_CHECK_EQUAL( info, 0 );
  //  for(std::size_t ii = 0; ii != 3ul; ++ii)
  //  {
  //    for(std::size_t jj = 0; jj != 3ul; ++jj)
  //    {
  //      std::cout << "  A["<< ii << "," << jj << "] = " << var( index_type{ii,jj} );
  //  
  //    }
  //    std::cout << "\n";
  //  }
  //  std::cout << ipiv[0] << " " << ipiv[1] << " " << ipiv[2] << "\n";
  //  
  std::valarray< double > b{ { 14.0, 32.0, 23.0 } };
  std::valarray< double > result{ { 1.0, 2.0, 3.0 } };
  
  //  std::cout << b[0] << " " << b[1] << " " << b[2] << "\n";
  info = utility::lapack::dgetrs( row_major, var, ipiv, b );
  BOOST_CHECK_EQUAL( info, 0 );
  //  std::cout << b[0] << " " << b[1] << " " << b[2] << "\n";
  for( std::size_t ii{ 0 }; ii != 3ul; ++ii )
  {
    BOOST_CHECK( utility::feq( b[ii], result[ii] ) );
  }

}

//Test the transpose of C++ multi_array
BOOST_AUTO_TEST_CASE( transpose_test )
{
  typedef std::array< std::size_t, 2 > index_type;
  boost::multi_array< double, 2 > var( index_type{ 3300, 3300 } );
  boost::shared_ptr< boost::mt19937 > gentor(new boost::mt19937);
  utility::random_distribution a(gentor);
  for(std::size_t ii = 0; ii != 3300; ++ii )
  {
    index_type idx;
    idx[0] = ii;
    for(std::size_t jj = 0; jj != 3300; ++jj )
    {
      idx[1] = jj;
      var( idx ) = a.uniform(1.0);
    }
  }
  {
    boost::timer::auto_cpu_timer t;
    for( std::size_t idx{ 0 }; idx != 10; ++idx )
    {
      utility::lapack::square_transpose( var.origin(), var.shape()[0] );
    }
  }
  {
    boost::timer::auto_cpu_timer t;
    for( std::size_t idx{ 0 }; idx != 100; ++idx )
    {
      utility::lapack::square_transpose( var.origin(), var.shape()[0] );
    }
  }
  {
    boost::timer::auto_cpu_timer t;
    for( std::size_t idx{ 0 }; idx != 100; ++idx )
    {
      utility::lapack::square_transpose2( var.origin(), var.shape()[0] );
    }
  }
  boost::multi_array< double, 2 > var2( var );
  BOOST_CHECK( var == var2 );
  utility::lapack::square_transpose( var.origin(), var.shape()[0] );
  utility::lapack::square_transpose2( var2.origin(), var2.shape()[0] );
  BOOST_CHECK( var == var2 );

}

