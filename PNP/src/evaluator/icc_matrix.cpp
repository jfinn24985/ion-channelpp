

#ifndef DEBUG
#define DEBUG 0
#endif

#include "evaluator/icc_matrix.hpp"
#include "utility/archive.hpp"

#include "evaluator/icc_surface_grid.hpp"

// -
#include "core/strngs.hpp"
#include "utility/utility.hpp"
// -
#include <array>
// -
namespace evaluator {

icc_matrix::icc_matrix()
: amx_()
, indx_()
, row_major_( true )
{}


// Get value from A matrix
//
// \pre i1 < size and i2 < size (not checked)
double icc_matrix::a(std::size_t i1, std::size_t i2) const
{
  std::array< std::size_t, 2 > idx;
  idx[ 0 ] = i1;
  idx[ 1 ] = i2;
  return this->amx_( idx );
}

//----------------------------------------------------------------------
// save amx and indx
//
// This is the counterpoint method to 'readam'.  It saves a digest
// of of the input parameters critical to defining the matrix.
// These are the protein geometry parameters, the patch integration
// grid parameters and the permittivity constants. Then saves the
// 'amx' matrix itself.
//
// \pre not empty
void icc_matrix::write_a_matrix(std::ostream & output) const
{
  UTILITY_REQUIRE( not this->empty(), "No patch information to write." );
  UTILITY_REQUIRE( output.good (), "Invalid output stream." );
  {
    output << core::strngs::comment_begin() << " SIZE : " << this->size() << " " << core::strngs::comment_end() << "\n";
    output << core::strngs::comment_begin() << " ROW_MAJOR : " << ( this->row_major_ ? "true" : "false" ) << " " << core::strngs::comment_end() << "\n";
    // The indx and A matrix
    std::array< std::size_t, 2 > idx;
    for (std::size_t ipch = 0; ipch != this->size(); ++ipch)
    {
      idx[ 0 ] = ipch;
      output << this->indx_[ ipch ] << "\n";
      for (std::size_t jpch = 0; jpch != this->size(); ++jpch)
      {
        idx[ 1 ] = jpch;
        output << ipch << " " << jpch << " " << this->amx_( idx ) << "\n";
      }
      output << "\n";
    }
  }

}

void icc_matrix::back_substitute(std::vector< double > & ch) const
{
  {
     const utility::lapack::int_type info_dgetrs = utility::lapack::dgetrs( this->row_major_, this->amx_, this->indx_, ch );
     UTILITY_ALWAYS( info_dgetrs == 0, "DGETRS routine returned error number " + std::to_string( info_dgetrs ) + ". 1 might mean no math library was configured." );
  }
  

}

void icc_matrix::back_substitute(std::valarray< double > & ch) const
{
  {
     const utility::lapack::int_type info_dgetrs = utility::lapack::dgetrs( this->row_major_, this->amx_, this->indx_, ch );
     UTILITY_ALWAYS( info_dgetrs == 0, "DGETRS routine returned error number " + std::to_string( info_dgetrs ) + ". 1 might mean no math library was configured." );
  }
  

}

// Compute the A matrix by integrating the the grid.
//
// \pre empty
// \pre not grid.empty

void icc_matrix::compute_amx(icc_surface_grid & grid)
{
UTILITY_CHECK( this->empty(), "Cannot compute A matrix twice");
UTILITY_CHECK( not grid.empty(), "Cannot compute A matrix without any patches defined");
//
// get grid size
this->amx_.resize( boost::extents[ grid.size() ][ grid.size() ] );
this->indx_.resize( grid.size(), 0 );

// generate the A matrix
grid.generate_matrix( this->amx_ );


}

// After the A matrix has been generated, perform LU decomposition.

void icc_matrix::lu_decompose_amx()
{
UTILITY_CHECK( this->indx_[ 0 ] == this->indx_[ 1 ] , "Cannot LU decompose A matrix twice.");

// -----------------------------------------------------
// Do LU decomposition of the A matrix
{
   const utility::lapack::int_type info_dgetrf = utility::lapack::dgetrf( this->row_major_, this->amx_, this->indx_ );

   UTILITY_ALWAYS( info_dgetrf == 0, "Matrix inversion failed, DGETRF routine returned error number " + std::to_string( info_dgetrf ) + ". 1 might mean no math library was configured." );
}


}


} // namespace evaluator
