

#ifndef DEBUG
#define DEBUG 0
#endif

#include "grs/evaluator/evaluators.hpp"

#include <boost/python/class.hpp>
#include <boost/python.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/call_method.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <cstdint>
#include <sstream>
#include <coulomb_evaluator.cpp>

namespace
{

static const bool LOTS_OF_NOISE (false);

namespace boopy = boost::python;
namespace numpy = boost::python::numeric;

// Reference to the overlap_exception's python class object
static PyObject *OverlapExceptionType;
} // end namespace

BOOST_PYTHON_MODULE(evaluator_ext)
{
  boost::python::numeric::array::set_module_and_type("numpy", "ndarray");

  // initialize the Numpy C API
  import_array();

  boost::python::class_<lennard_jones_evaluator>("LennardJonesEvaluator", boost::python::init< boost::python::object >())
  .def("__call__", &lennard_jones_evaluator::call_method)
  ;

  boost::python::class_<particle_overlap>("ParticleOverlapEvaluator", boost::python::init< boost::python::object >())
  .def("__call__", &particle_overlap::call_method)
  ;

  boost::python::class_<coulomb_evaluator>("CoulombEvaluator", boost::python::init<>())
  .def("call_homogeneous", &coulomb_evaluator::call_method< true >)
  .def("call_heterogeneous", &coulomb_evaluator::call_method< false >)
  ;

  boost::python::class_<overlap_exception> overlap_exception_type("OverlapError", boost::python::init< std::string >());
  overlap_exception_type.def("__str__",&overlap_exception::what)
  ;
  OverlapExceptionType=overlap_exception_type.ptr();

  boost::python::register_exception_translator< overlap_exception >(&overlap_exception::translator);
   
}




namespace grs {

//Translate the C++ exception into Python
void overlap_exception::translator(const overlap_exception & err)

{
  PyErr_SetObject(OverlapExceptionType, boost::python::object(err).ptr());

}

// Calculate particle-particle_set LJ interactions contribution. 
double lennard_jones_evaluator::call_method(boost::python::object change_set, boost::python::object item_list) 
{
  // NOTES:
  //
  // * create _minima_distance, _minima_energy and _cutoff_sq cache variables if necessary
  // * compute inter-particle distances (using compute_distances)
  // * compute new position - existing position energies (add)
  // * compute old position - existing position energies (subtract)
  // * compute new position - new position energies (add)
  // * compute old position - old position energies (add as over deleted above)
  
  namespace boopy = boost::python;
  namespace numpy = boost::python::numeric;
  
  double energy = 0.0;
  
  // Ignore any entries in item_list if item_list.species[idx] equals this
  const std::int32_t invalid_specie = boopy::extract< std::int32_t >(item_list.attr("INVALID_SPECIE_INDEX"));
  
  // The maximum in-use particle position
  const std::size_t end_index = boopy::extract< long int >(item_list.attr("end_index"));
  
  // The specie index array
  vector_ref< std::int32_t > species (boopy::extract< numpy::array >(item_list.attr("species")));
  
  // The precomputed distance vectors
  boopy::list changelists = boopy::extract< boopy::list >(change_set.attr("particle_particle_distances"));
  
  if (this->_minima_distance.empty ())
    {
      // The set of specie flyweight objects
      boopy::list specie_set (item_list.attr("specie_set"));
      const std::int32_t l = (boopy::len(specie_set));
      boost::array< std::int32_t, 2 > idx = {{ l, l }};
      this->_minima_distance.resize(idx);
      this->_minima_energy.resize(idx);
      this->_cutoff_sq.resize(idx);
      // Get dist and energy and cutoffs from specie_set
      for (std::int32_t ispec = 0; ispec != l; ++ispec)
        {
  	idx[0] = ispec;
  	boopy::object specie0 = specie_set[ispec];
  	boopy::list mindist (specie0.attr("minima_distance"));
  	boopy::list minenergy (specie0.attr("minima_energy"));
  	boopy::list cutoffs (specie0.attr("cutoffs"));
  	for (std::int32_t jspec = 0; jspec != l; ++jspec)
  	  {
  	    idx[1] = jspec;
  	    this->_minima_distance(idx) = boopy::extract< double >(mindist[jspec]);
  	    this->_minima_energy(idx) = boopy::extract< double >(minenergy[jspec]);
  	    this->_cutoff_sq(idx) = std::pow(boopy::extract< double >(cutoffs[jspec]), 2);
  	  }
        }
    }
  // The list of change atoms
  boopy::list changes = boopy::extract< boopy::list >(change_set.attr("changes"));
  const std::size_t number_of_changes (boopy::len(changes));
  
  for (std::size_t icount = 0; icount != number_of_changes; ++icount)
    {
      // the current ChangeAtom
      boopy::object change (changes[icount]);
      const std::size_t index = (boopy::extract< bool >(change.attr("is_add")) ? -1 : boopy::extract< std::int32_t >(change.attr("index")));
      // Specie indices
      boost::array< std::int32_t ,2 > idx;
      idx [0] = boopy::call_method< std::int32_t >(change.ptr(), "specie_index");
  
      // Cached information about the current change
      const bool _is_add (not distances[icount].new_old.empty() and distances[icount].old_old.empty());
      const bool _is_move (not distances[icount].new_old.empty() and not distances[icount].old_old.empty());
      const bool _is_remove (distances[icount].new_old.empty() and not distances[icount].old_old.empty());
  
      // Gained potential energy from a particle in a new position
      if (not distances[icount].new_old.empty())
        {
  	vector_ref< double > distarr (boopy::extract< numpy::array >(changelists[icount][0]));
  
  	for (std::size_t jndex = 0; jndex != end_index; ++jndex)
  	  {
  	    idx [1] = species[jndex];
  	    if (jndex != index and idx [1] != invalid_specie)
  	      {
  		energy += lennard_jones_energy (distances[icount].new_old[jndex], this->_cutoff_sq(idx)
  							 , this->_minima_distance(idx)
  							 , this->_minima_energy(idx));
  	      }
  	  }
  	// Gained potential energy from particles in new positions
  	if (icount + 1 < number_of_changes and distances[icount].old_old.empty())
  	  {
  	    vector_ref< double > distarr (boopy::extract< numpy::array >(changelists[icount][2]));
  	    for (std::size_t jcount = icount + 1; jcount != number_of_changes; ++jcount)
  	      {
  		if (not distances[jcount].new_old.empty() and distances[jcount].old_old.empty())
  		  {
  		    boopy::object change1 = changes[jcount];
  		    idx [1] = boopy::call_method< std::int32_t >(change1.ptr(), "specie_index");
  		    // With add two or more particles we must account for the new interactions
  		    // between them
  		    energy += lennard_jones_energy (distances[icount].new_new[jcount], this->_cutoff_sq(idx)
  							     , this->_minima_distance(idx)
  							     , this->_minima_energy(idx));
  		  }
  	      }
  	  }
        }
  
      // Lost potential energy from a particle in an old position
      if (not distances[icount].old_old.empty())
        {
  	// Removing a particle, calculate lost energy. We need to check indices in
  	// inner loop as distance between the particle to remove and itself is zero
  	for (std::size_t jndex = 0; jndex != end_index; ++jndex)
  	  {
  	    if (jndex != index)
  	      {
  		idx [1] = species[jndex];
  		if (idx [1] != invalid_specie)
  		  {
  		    energy -= lennard_jones_energy (distances[icount].old_old[jndex], this->_cutoff_sq(idx)
  							     , this->_minima_distance(idx)
  							     , this->_minima_energy(idx));
  		  }
  	      }
  	  }
        }
  
  
  
      // Over-deleted potential energy from particles in old positions
      if (_is_remove)
        {
  	vector_ref< double > distarr (boopy::extract< numpy::array >(changelists[icount][3]));
  	for (std::size_t jcount = icount + 1; jcount != number_of_changes; ++jcount)
  	  {
  	    boopy::object change1 = changes[jcount];
  	    if (boopy::extract< bool >(change1.attr("is_remove")))
  	      {
  		idx [1] = boopy::call_method< std::int32_t >(change1.ptr(), "specie_index");
  		const std::int32_t jndex (boopy::extract< std::int32_t >(change1.attr("index")));
  		// with removing two or more particles we must add back in the extra
  		// interactions between the disappearing particles we should not have
  		// deleted in the loop above.
  		energy += lennard_jones_energy (distances[icount].old_old[jndex], this->_cutoff_sq(idx)
  							 , this->_minima_distance(idx)
  							 , this->_minima_energy(idx));
  
  	      }
  	  }
        }
    }
  return energy;
  

}

//As change particle : change particle Coulomb interactions are calculated
//in the same way as change particle : config particle, the __init__ method
//makes _do_change_contribution an alias for _do_contribution. The
//nspec argument is the number of interacting species.
lennard_jones_evaluator::lennard_jones_evaluator(boost::python::object geom)
  : _cutoff_sq()
  , _minima_distance()
  , _minima_energy ()
  , _geometry(geom)
  {}



} // namespace grs
