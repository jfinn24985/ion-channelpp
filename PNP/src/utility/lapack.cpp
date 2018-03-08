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

#ifdef USE_MKL
#include <mkl_lapack.h>
#include <mkl_service.h>
#endif

#ifdef USE_OPENBLAS
#include <f77blas.h>
#include <openblas_config.h>
#endif

// Get version information for the ATLAS library
#ifdef USE_ATLAS
extern "C"
{
#include <clapack.h>
#include <atlas/atlas_buildinfo.h>
}
#include <boost/format.hpp>
#endif

#ifdef USE_GSL
#include <boost/format.hpp>
#include <clapack.h>
#include <gsl_version.h>
#endif

#include "utility/lapack.hpp"

namespace utility {

// Computes an LU factorization of a general M-by-N matrix A
// using partial pivoting with row interchanges.
//
// The factorization has the form
//   A = P * L * U
// Where P is a permutation matrix, L is lower triangular with
// unit diagonal elements (lower trapezoidal if m>n). U is 
// upper triangular (upper trapezoidal if m<n).
//
// \return is info about the result of diagonalization.
//   = 0 : success
//   < 0 : (-result)th argument had an illegal value
//   > 0 : U(result,result) is exactly zero. Factorization completed
//           but division by zero will occur if it is used to solve a 
//           system of equations.
lapack::int_type lapack::dgetrf(bool & row_major, lapack::int_type m, lapack::int_type n, double * amx, lapack::int_type lda, lapack::int_type * ipiv)
{
  lapack::int_type info( 1 );
  // Intel MKL library
#ifdef USE_MKL
  if( not row_major )
  {
    row_major = true;
    square_transpose( amx, m );
  }
  ::dgetrf(&m, &n, amx, &lda, ipiv, &info);
#endif
  
  // openBLAS library
#ifdef USE_OPENBLAS
  if( row_major )
  {
    row_major = false;
    square_transpose( amx, m );
  }
  ::dgetrf_(&m, &n, amx, &lda, ipiv, &info);
#endif
  
  // ATLAS library
#ifdef USE_ATLAS
  if( not row_major )
  {
    row_major = true;
    square_transpose( amx, m );
  }
  info = ::clapack_dgetrf(CblasRowMajor, m, n, amx, lda, ipiv);
#endif
  
  // GNU Scientific library
#ifdef USE_GSL
  if( not row_major )
  {
    row_major = true;
    square_transpose( amx, m );
  }
  info = ::dgetrf(CblasRowMajor, m, n, amx, lda, ipiv);
#endif
  return info;

}

// Computers an LU factorization of a general M-by-N matrix A
// using partial pivoting with row interchanges.
//
// The factorization has the form
//   A = P * L * U
// Where P is a permutation matrix, L is lower triangular with
// unit diagonal elements (lower trapezoidal if m>n). U is 
// upper triangular (upper trapezoidal if m<n).
//
// \return is info about the result of diagonalization.
//   = 0 : success
//   < 0 : (-result)th argument had an illegal value
//   > 0 : U(result,result) is exactly zero. Factorization completed
//           but division by zero will occur if it is used to solve a 
//           system of equations.
lapack::int_type lapack::dgetrf(bool & row_major, boost::multi_array< double, 2 > & amx, std::vector< int_type  > & ipiv)
{
  return lapack::dgetrf( row_major, amx.shape()[ 0 ], amx.shape()[ 1 ], amx.origin(), ipiv.size(), ipiv.data() );
}

//Solves a system of linear equations
//  A * C = B
//with a general N-by-N matrix A using the LU
//factorization computed by dgetrf. (We only 
//use the no-transpose version so the transpose or
//conjugate transpose forms are not accessible
//through this interface.)

lapack::int_type lapack::dgetrs(bool row_major, lapack::int_type n, lapack::int_type nrhs, double * amx, lapack::int_type lda, lapack::int_type * ipiv, double * b, lapack::int_type ldb)
{
  lapack::int_type info( 1 );
  // Intel Maths Kernel Library
#ifdef USE_MKL
  char trans{ row_major ? 'N' : 'T' };
  ::dgetrs(&trans, &n, &nrhs, amx, &lda, ipiv, b, &ldb, &info);
#endif
  
  // openBLAS Library
#ifdef USE_OPENBLAS
  char trans{ row_major ? 'T' : 'N' };
  ::dgetrs_(&trans, &n, &nrhs, amx, &lda, ipiv, b, &ldb, &info);
#endif
  
  
  // The ATLAS Library
#ifdef USE_ATLAS
  char trans{ row_major ? CblasRowMajor : CblasColMajor };
  info = ::clapack_dgetrs(CblasRowMajor, CblasNoTrans, n, nrhs, amx, lda, ipiv, b, ldb);
#endif
  
  // GNU Scientific Library
#ifdef USE_GSL
  char trans{ row_major ? CblasRowMajor : CblasColMajor };
  info = ::dgetrs(CblasRowMajor, CblasNoTrans, n, nrhs, amx, lda, ipiv, b, ldb);
#endif
  return info;
  

}

//Solves a system of linear equations
//  A * C = B
//with a general N-by-N matrix A using the LU
//factorization computed by dgetrf. (We only 
//use the no-transpose version so the transpose or
//conjugate transpose forms are not accessible
//through this interface.)

lapack::int_type lapack::dgetrs(bool row_major, const boost::multi_array< double, 2 > & amx, const std::vector< int_type > & ipiv, std::vector< double > & b)
{
  return lapack::dgetrs( row_major, amx.shape()[ 1 ], 1, const_cast< double* >( amx.origin() ),  ipiv.size(), const_cast< lapack::int_type* >( ipiv.data() ), b.data(), b.size() );
}

//Solves a system of linear equations
//  A * C = B
//with a general N-by-N matrix A using the LU
//factorization computed by dgetrf. (We only 
//use the no-transpose version so the transpose or
//conjugate transpose forms are not accessible
//through this interface.)

lapack::int_type lapack::dgetrs(bool row_major, const boost::multi_array< double, 2 > & amx, const std::vector< int_type > & ipiv, std::valarray< double > & b)
{
  return lapack::dgetrs( row_major, amx.shape()[ 1 ], 1, const_cast< double* >( amx.origin() ),  ipiv.size(), const_cast< lapack::int_type* >( ipiv.data() ), &b[0], b.size() );
}

//Get the inuse lapack library version
std::string lapack::version()
{
  static bool once (false);
  static std::string version;
  
  if (not once)
  {
  // Get version information for the MKL library
#ifdef USE_MKL
    char buf_[256];
    ::mkl_get_version_string (buf_, 255);
    buf_[255] = '\0';
    version = std::string (buf_);
#endif
  
  // Get version information for the MKL library
#ifdef USE_OPENBLAS
    version = std::string{ OPENBLAS_VERSION };
#endif
  
  // Get version information for the ATLAS library
#ifdef USE_ATLAS
    boost::format ver("ATLAS math library version %1%: arch %2%");
    ver % ATL_VERS % ATL_ARCH;
    version = ver.str();
#endif
  
  // Get version information for the GSL library
#ifdef USE_GSL
    boost::format ver("Gnu Scientific library %1%");
    ver % gsl_version;
    version = ver.str ();
#endif
    once = true;
  }
  return version;
  

}

// Transpose a square matrix
void lapack::square_transpose(boost::multi_array< double, 2 > & mx)
{
  // UTILITY_REQUIRE( mx.shape()[ 1 ] == mx.shape()[ 0 ], "Matrix is not square" );
  std::array< std::size_t, 2 > idx, jdx;
  for(std::size_t ii = 0; ii != mx.shape()[ 0 ]; ++ii)
  {
    idx[0] = ii;
    jdx[1] = ii;
    for (std::size_t jj = ii + 1; jj < mx.shape()[ 1 ] ; ++jj)
    {
      idx[1] = jj;
      jdx[0] = jj;
      std::swap( mx( idx ), mx( jdx ) );
    }
  }

}

// Transpose a square matrix
void lapack::square_transpose(double * mat, lapack::int_type sz)
{
  for( int_type i = 0; i < sz; ++i )
  for( int_type j = i + 1; j < sz; ++j )
  {
#ifdef FAST_INDEX
    std::swap (mat[i * sz + j], mat[j * sz + i]);
#else
    std::swap (*(mat + i * sz + j), *(mat + j * sz + i));
#endif
  }

}

// Transpose a square matrix
void lapack::square_transpose2(double * mat, lapack::int_type sz)
{
  struct local_index_type
  {
    int_type i1, j1, i2, j2;
  };
  std::vector< local_index_type > stack;
  stack.push_back( { 0, 0, sz, sz } );
  while( not stack.empty() )
  {
    local_index_type cur = stack.back();
    stack.pop_back();
    if( cur.i1 == cur.j1 ) // diagonal
    {
      const int_type da{ cur.i2 - cur.i1 };
      if( da > 16 )
      {
        const int_type hda{ da / 2 };
        stack.push_back( { cur.i1, cur.j1, cur.i1 + hda, cur.j1 + hda } );
        stack.push_back( { cur.i1 + hda, cur.j1, cur.i2, cur.j1 + hda } );
        stack.push_back( { cur.i1 + hda, cur.j1 + hda, cur.i2, cur.j2 } );
      }
      else
      {
        for( ; cur.i1 < cur.i2; ++cur.i1 )
        {
          for( cur.j1 = cur.i1 + 1; cur.j1 < cur.j2; ++cur.j1 )
          {
#ifdef FAST_INDEX
            std::swap( mat[cur.i1 * sz + cur.j1], mat[cur.j1 * sz + cur.i1] );
#else
            std::swap( *(mat + cur.i1 * sz + cur.j1), *(mat + cur.j1 * sz + cur.i1) );
#endif
          }
        }
      }
    }
    else // non-diagonal
    {
      const int_type da{ cur.i2 - cur.i1 };
      if( da > 16 )
      {
        const int_type hda{ da / 2 };
        stack.push_back( { cur.i1, cur.j1, cur.i1 + hda, cur.j1 + hda } );
        stack.push_back( { cur.i1 + hda, cur.j1, cur.i2, cur.j1 + hda } );
        stack.push_back( { cur.i1, cur.j1 + hda, cur.i1 + hda, cur.j2 } );
        stack.push_back( { cur.i1 + hda, cur.j1 + hda, cur.i2, cur.j2 } );
      }
      else
      {
        for( ; cur.i1 < cur.i2; ++cur.i1 )
        {
          for( int_type jj = cur.j1; jj < cur.j2; ++jj )
          {
#ifdef FAST_INDEX
            std::swap( mat[cur.i1 * sz + jj], mat[jj * sz + cur.i1] );
#else
            std::swap( *(mat + cur.i1 * sz + jj), *(mat + jj * sz + cur.i1) );
#endif
          }
        }
      }
    }
  }

}


} // namespace utility
