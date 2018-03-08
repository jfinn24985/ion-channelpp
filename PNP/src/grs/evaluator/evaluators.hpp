#ifndef IONCH_GRS_EVALUATORS_HPP
#define IONCH_GRS_EVALUATORS_HPP


#include <stdexcept>

#include <string>
#include <cstddef>
#include <boost/python/numeric.hpp>

#include <boost/python/object.hpp>

#include "utility/multi_array.hpp"

#include <numpy/arrayobject.h>
namespace grs {


template < class X >
struct numpy_type_traits
{
  enum { TYPEID=-1 };
};

template <>
struct numpy_type_traits< std::int64_t >
{
  enum { TYPEID=NPY_INT64 };
};

template <>
struct numpy_type_traits< std::int32_t >
{
  enum { TYPEID=NPY_INT32 };
};

template <>
struct numpy_type_traits< double >
{
  enum { TYPEID=NPY_DOUBLE };
};

template <>
struct numpy_type_traits< bool >
{
  enum { TYPEID=NPY_BOOL };
};


//Indicate and Overlap occured.
class overlap_exception : public std::runtime_error
 {
   public:
    overlap_exception(std::string s): std::runtime_error(s) {}

    //Translate the C++ exception into Python
    static void translator(const overlap_exception & err);


};
//Provide a simplified, typesafe interface to use and create 1D numpy arrays
template<class Type>
class vector_ref
 {
   private:
    //The data pointer
    Type * _data;

    //The number of a array points
    std::size_t _size;

    //The array we reference
    boost::python::numeric::array _arr;


   public:
    Type* begin()
    {
      return _data;
    }

    Type const* begin() const
    {
      return _data;
    }

    Type* end()
    {
      return _data + _size;
    }

    Type* end() const
    {
      return _data + _size;
    }

    bool empty() const
    {
      return 0 == _size;
    }

    Type* size() const
    {
      return _size;
    }

    Type operator[](std::size_t i) const
    {
      return *(_data + i);
    }

    Type& operator[](std::size_t i)
    {
      return *(_data + i);
    }

    //Get a reference to the Numpy array
    boost::python::numeric::array to_python()
    {
      return _arr;
    }

    //Get a reference to the python array
     operator boost::python::numeric::array()
    {
      return _arr;
    }


   private:
    //Static member to generate a Numpy array of the given size
    static boost::python::numeric::array _build_arr(std::size_t n)
    {
        npy_intp dims[] = { n };
        return boost::python::numeric::array (boost::python::handle<>(PyArray_SimpleNew(1, &dims[0], numpy_type_traits< Type >::TYPEID)));
    }


   public:
    vector_ref(std::size_t n)
    : _data()
    , _size(n)
    , _arr (_build_arr(n))
    {
      _data = static_cast< Type* >(PyArray_DATA(_arr.ptr()));
    }

    vector_ref(std::size_t n, Type * data)
    : _data()
    , _size(n)
    , _arr (_build_arr(n))
    {
      _data = static_cast< Type* >(PyArray_DATA(_arr.ptr()));
      std::copy(data, data + n, _data);
    }

    template< class FwdIter > vector_ref(FwdIter begin, FwdIter end)
    : _data()
    , _size(std::distance(begin, end))
    , _arr (_build_arr(_size))
    {
      _data = static_cast< Type* >(PyArray_DATA(_arr.ptr()));
      std::copy(begin, end, _data);
    }

    vector_ref(boost::python::object arr)
    : _data()
    , _size()
    , _arr (arr)
    {
      if (!arr.ptr())
      {
        throw std::runtime_error("Array is a nul object.");
      }
      PyObject *_dataobj = PyArray_FROM_OTF(arr.ptr(), numpy_type_traits< Type >::TYPEID,  NPY_IN_ARRAY);
      if (!_dataobj)
      {
        throw std::runtime_error("Array is not of the correct type.");
      }
      // check that m is a vector
      const int k = PyArray_NDIM(_dataobj);
      if (k != 1)
      {
        throw std::runtime_error("Array is not of the correct shape.");
      }
      _size = PyArray_SIZE(_dataobj); // number of elements in array
      _data = static_cast< Type* >(PyArray_DATA(_dataobj));
    }


};
//Provide a simplified, typesafe interface to use and create 2D numpy arrays. 
template<class Type, std::size_t N>
class matrix_ref
 {
   public:
    typedef Type TypeN[N];


   private:
    //The data pointer
    TypeN * _data;

    //The number of a array points
    std::size_t _size;

    //The array we reference
    boost::python::numeric::array _arr;


   public:
    Type* begin()
    {
      return _data;
    }

    Type const* begin() const
    {
      return _data;
    }

    Type* end()
    {
      return _data + _size * N;
    }

    Type* end() const
    {
      return _data + _size * N;
    }

    bool empty() const
    {
      return 0 == _size;
    }

    Type* size() const
    {
      return _size;
    }

    Type operator()(std::size_t i, std::size_t j) const
    {
      return _data[i][j];
    }

    Type& operator()(std::size_t i, std::size_t j)
    {
      return _data[i][j];
    }

    //Get a reference to the Numpy array
    boost::python::numeric::array to_python()
    {
      return _arr;
    }

    //Get a reference to the python array
     operator boost::python::numeric::array()
    {
      return _arr;
    }


   private:
    //Static member to generate a Numpy array of the given size
    static boost::python::numeric::array _build_arr(std::size_t n)
    {
        npy_intp dims[] = { n, N };
        return boost::python::numeric::array (boost::python::handle<>(PyArray_SimpleNew(2, &dims[0], numpy_type_traits< Type >::TYPEID)));
    }


   public:
    matrix_ref(std::size_t n)
    : _data()
    , _size(n)
    , _arr (_build_arr(n))
    {
      _data = static_cast< TypeN* >(PyArray_DATA(_arr.ptr()));
    }

    matrix_ref(std::size_t n, Type * data)
    : _data()
    , _size(n)
    , _arr (_build_arr(n))
    {
      _data = static_cast< TypeN* >(PyArray_DATA(_arr.ptr()));
      std::copy(data, data + n*N, _data);
    }

    template< class FwdIter > matrix_ref(FwdIter begin, FwdIter end)
    : _data()
    , _size(std::distance(begin, end)/N)
    , _arr (_build_arr(_size))
    {
      _data = static_cast< TypeN* >(PyArray_DATA(_arr.ptr()));
      std::copy(begin, end, _data);
    }

    matrix_ref(boost::python::object arr)
    : _data()
    , _size()
    , _arr (arr)
    {
      if (!arr.ptr())
      {
        throw std::runtime_error("Array is a nul object.");
      }
      PyObject *_dataobj = PyArray_FROM_OTF(arr.ptr(), numpy_type_traits< Type >::TYPEID,  NPY_IN_ARRAY);
      if (!_dataobj)
      {
        throw std::runtime_error("Array is not of the correct type.");
      }
      // check that m is a vector
      const int k = PyArray_NDIM(_dataobj);
      if (k != 2)
      {
        throw std::runtime_error("Array is not of the correct shape.");
      }
      const int n = PyArray_DIMS(_dataobj)[1];
      if (n != N)
      {
        throw std::runtime_error("Array is not of the correct shape.");
      }
      _size = PyArray_SIZE(_dataobj); // number of elements in array
      _data = static_cast< TypeN* >(PyArray_DATA(_dataobj));
    }


};
//[cite: en.wikipedia.org/wiki/Lennard-Jones_potential]
//
//The Lennard-Jones potential (also referred to as the L-J potential,
//6-12 potential, or 12-6 potential) is a mathematically simple model
//that approximates the interaction between a pair of neutral atoms
//or molecules.  A form of the potential was first proposed in 1924
//by John Lennard-Jones.[1] The most common expressions of the L-J
//potential are
//
//    \begin{alignat}{3} V_{LJ}& = 4 \varepsilon &\left[ \left( \frac
//    {\sigma} {r} \right)^{12} - \left( \frac {\sigma} {r} \right)^6
//    \right]\\ & = \varepsilon &\left[\left( \frac {r_m} {r}
//    \right)^{12} - 2 \left( \frac {r_m} {r} \right)^6 \right]
//    \end{alignat}
//
//where 
//  \varepsilon is the depth of the potential well, 
//  \sigma is the finite distance at which the inter-particle potential is zero,
//  r is the distance between the particles, and
//  r_m is the distance at which the potential reaches its minimum.  
//
//At r_m, the potential function has the value -\varepsilon.  The
//distances are related as r_m = 2^{1/6}\sigma.  These parameters can
//be fitted to reproduce experimental data or accurate quantum chemistry
//calculations.  Due to its computational simplicity, the Lennard-Jones
//potential is used extensively in computer simulations even though
//more accurate potentials exist.
//
//Pairwise parameters are determined using the Lorentz-Berthelot mixing 
//rules, namely:
//­\varepsilon_{ij} = \sqrt \varepsilon_{ii} \varepsilon_{jj}
//­\sigma_{ij} = frac{ \sigma_{ii} + \sigma_{jj} }{ 2 }
//
//This means only self-self interaction terms need to be specified
//in the input file. Although it may be possible to directly
//set particular pairwise terms.
//
//M. P. Allen and D. J. Tildesley, Computer Simulation of Liquids (Clarendon, Oxford, 1987)].

class lennard_jones_evaluator
 {
   private:
    //The cut-off distance (squared) used to short-cut the energy evaluation for large distances.
    boost::multi_array<double, 2> _cutoff_sq;

    //The distance between two particles of type specie0 and specie1 where the
    //minima in enegy is located (squared).
    boost::multi_array<double, 2> _minima_distance;

    // The depth of the energy well between two species. This
    // is the \varepsilon value in the L-J potential.
    boost::multi_array<double, 2> _minima_energy;


   public:
    // Calculate particle-particle_set LJ interactions contribution. 
    double call_method(boost::python::object change_set, boost::python::object item_list);

    //As change particle : change particle Coulomb interactions are calculated
    //in the same way as change particle : config particle, the __init__ method
    //makes _do_change_contribution an alias for _do_contribution. The
    //nspec argument is the number of interacting species.
    lennard_jones_evaluator(boost::python::object geom);


   private:
    static double lennard_jones_energy(double r_sq, double r_cutoff_sq, double r_min, double e_min) const
    {
      //
      // U_LJ = e_min [ (r_min/r)^12 - 2(r_min/r)^6]
      //
      if (r_sq < r_cutoff_sq)
      {
        const double f = r_min * r_min / r_sq;
        const double g = std::pow(f, 3);
        return e_min * g * (g - 2.0);
      }
      return 0.0;
    }


};

} // namespace grs
#endif
