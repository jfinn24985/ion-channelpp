#ifndef IONCH_UTILITY_LAPACK_HPP
#define IONCH_UTILITY_LAPACK_HPP

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

#include "utility/multi_array.hpp"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/valarray.hpp>
#include <string>

namespace utility {

//  BLAS/LAPACK compatibility routines, should call through to external library
//
//  Consult the documentation of the Blas/Lapack library for documentation about the 
//  routines listed.
class lapack
 {
   public:
    // integer type used by the lapack library
    typedef int int_type;

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
    static int_type dgetrf(bool & row_major, int_type m, int_type n, double * amx, int_type lda, int_type * ipiv);

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
    static int_type dgetrf(bool & row_major, boost::multi_array< double, 2 > & amx, std::vector< int_type  > & ipiv);

    //Solves a system of linear equations
    //  A * C = B
    //with a general N-by-N matrix A using the LU
    //factorization computed by dgetrf. (We only 
    //use the no-transpose version so the transpose or
    //conjugate transpose forms are not accessible
    //through this interface.)
    
    static int_type dgetrs(bool row_major, int_type n, int_type nrhs, double * amx, int_type lda, int_type * ipiv, double * b, int_type ldb);

    //Solves a system of linear equations
    //  A * C = B
    //with a general N-by-N matrix A using the LU
    //factorization computed by dgetrf. (We only 
    //use the no-transpose version so the transpose or
    //conjugate transpose forms are not accessible
    //through this interface.)
    
    static int_type dgetrs(bool row_major, const boost::multi_array< double, 2 > & amx, const std::vector< int_type > & ipiv, std::vector< double > & b);

    //Solves a system of linear equations
    //  A * C = B
    //with a general N-by-N matrix A using the LU
    //factorization computed by dgetrf. (We only 
    //use the no-transpose version so the transpose or
    //conjugate transpose forms are not accessible
    //through this interface.)
    
    static int_type dgetrs(bool row_major, const boost::multi_array< double, 2 > & amx, const std::vector< int_type > & ipiv, std::valarray< double > & b);

    //Get the inuse lapack library version
    static std::string version();

    // Transpose a square matrix
    static void square_transpose(boost::multi_array< double, 2 > & mx);

    // Transpose a square matrix
    static void square_transpose(double * mat, int_type sz);

    // Transpose a square matrix
    static void square_transpose2(double * mat, int_type sz);


};

} // namespace utility
#endif
