#ifndef IONCH_EVALUATOR_ICC_MATRIX_HPP
#define IONCH_EVALUATOR_ICC_MATRIX_HPP


#include "utility/lapack.hpp"
#include <boost/multi_array.hpp>

#include <boost/serialization/vector.hpp>
#include <cstddef>
#include <iostream>
#include <boost/serialization/valarray.hpp>

namespace evaluator { class icc_surface_grid; } 

namespace evaluator {

//  Matrix of coefficients of simultaneous equations used
//  for calculating induced surface charges from external
//  electric field.
class icc_matrix
 {
   public:
    typedef boost::multi_array<double, 2> array_type;


   private:
    // The A matrix
    array_type amx_;

    //The index vector from the LU decomposition
    std::vector<utility::lapack::int_type> indx_;

    // Storage order of matrix. Defaults to true but may be changed by some methods to optimise performance.
    bool row_major_;


   public:
    icc_matrix();


   private:
    icc_matrix(const icc_matrix & source) = delete;

    icc_matrix(icc_matrix && source) = delete;


   public:
    ~icc_matrix()
    {}


   private:
    icc_matrix & operator=(const icc_matrix & source) = delete;



  friend class boost::serialization::access;    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
       ar & amx_;
       ar & indx_;
       ar & row_major_;
    }


   public:
    // Get value from A matrix
    //
    // \pre i1 < size and i2 < size (not checked)
    double a(std::size_t i1, std::size_t i2) const;

    bool empty() const
    {
       return this->indx_.empty();
    }

    bool is_row_major() const
    {
      return this->row_major_;
    }

    // Get value from H vector
    //
    // \pre idx < size
    double pivot(std::size_t i1) const
    {
      return this->indx_.at( i1 );
    }
    

    std::size_t size() const
    {
       return this->indx_.size();
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
    void write_a_matrix(std::ostream & output) const;

    void back_substitute(std::vector< double > & ch) const;

    void back_substitute(std::valarray< double > & ch) const;

    // Compute the A matrix by integrating the the grid.
    //
    // \pre empty
    // \pre not grid.empty
    
    void compute_amx(icc_surface_grid & grid);

    // After the A matrix has been generated, perform LU decomposition.
    
    void lu_decompose_amx();


};

} // namespace evaluator
#endif
