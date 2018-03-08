#ifndef IONCH__PYTHON_WRAPPERS_HPP
#define IONCH__PYTHON_WRAPPERS_HPP


#include "evaluator/base_evaluator.hpp"
#include "platform/simulator.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include "observable/base_observable.hpp"

//-
#include <evaluator/base_evaluator.hpp>
#include <trial/change_set.hpp>
#include <boost/python.hpp>
//-
namespace particle { class change_set; } 
namespace particle { class ensemble; } 
namespace geometry { class coordinate; } 
namespace utility { class random_distribution; } 

using namespace boost::python;
using namespace evaluator;
//Wrapper to allow derived classes in python
struct evaluator_wrapper : public evaluator::base_evaluator, wrapper<evaluator::base_evaluator> 
{
    void compute_potential(const platform::simulator & sys, particle::change_set & changes, std::size_t start_index = 0) const override
    {
      this->get_override("compute_potential")(sys, changes, start_index);
    }
    //Log message descibing the observable and its parameters
    void description(std::ostream & os) const override
    {
      this->get_override("description")(os);
    }

    //Prepare the evaluator for use with the given simulator and
    //stepper.
    virtual void prepare(const platform::simulator & sim) override
    {
      this->get_override("prepare")(sim);
    }

    std::string type_label() const override
    {
      return this->get_override("type_label")();
    }


};
struct sampler_wrapper : public observable::base_observable, wrapper<observable::sampled_observable> 
{
    //Log message descibing the observable and its parameters
    virtual void description(std::ostream & out) const override
    {
      this->get_override("description")(out);
    }

    //Prepare the evaluator for use with the given simulator and
    //stepper. Check for connection to signals of interest and 
    //connect if necessary
    virtual void prepare(platform::simulator & sim) override
    {
      this->get_override("prepare")(sim);
    }

    //Register any signals output by the object with 
    //the simulator and optionally connect to any 
    //simulator specific signals. The object is responsible
    //for testing if a signal with the same label has been
    //registered.
    //
    //The base class method does nothing.
    virtual void enrol(platform::simulator & sim) override
    {
      if (override f = this->get_override("enrol")) f(sim);
    }

    //Register any signals output by the object with 
    //the simulator and optionally connect to any 
    //simulator specific signals. The object is responsible
    //for testing if a signal with the same label has been
    //registered.
    //
    //The base class method does nothing.
    virtual void default_enrol(platform::simulator & sim)
    {
      this->base_observable::enrol(sim);
    }


};
struct region_wrapper, wrapper<platform::region>  
{
    //Compute the distances between any new position and existing positions.
    //
    //NOTE: after call rij.size == ens.size and rij[0:start_index] == 0. That is the
    //rij vector will include entries for all elements of the ensemble but only 
    //entries from start_index will contain computed distances.
    virtual void compute_distances(const particle::ensemble & ens, const geometry::coordinate & position, platform::simulator::array_type & rij, std::size_t startindex) const override
    {
      this->get_override("compute_distances")(ens, position, rij, startindex);
    }

    //Details about the current geometry to be written to the
    //log at the start of the simulation.
    virtual void description(std::ostream & os) const override
    {
      this->get_override("description")(os);
    }

    //Calculate the square of the distance between two points.
    virtual double distance_sqr(const geometry::coordinate & lhs, const geometry::coordinate & rhs) const override
    {
      return this->get_override("distance_sqr")(lhs, rhs);
    }

    //Is a particle at the given position and radius in a valid position
    //in the system?
    virtual bool is_valid_position(geometry::coordinate & coord, std::size_t ispec) const override
    {
      return this->get_override("is_valid_position")(coord, ispec);
    }

    //Generate a new random position in the given region.
    virtual void new_position(geometry::coordinate & pos, std::size_t ispec, utility::random_distribution & ranf) const override
    {
      this->get_override("new_position")(pos, ispec, ranf);
    }

    //Create a position grid object that generates a
    //set of grid positions within the region with
    //a minimum of npart positions.
    //virtual std::unique_ptr< geometry::grid_generator > make_grid_generator(std::size_t npart, utility::random_distribution & rgen) const override
    //{
    //  return this->get_override("make_grid_generator")(npart, rgen);
    //}

    //The simulator's type name
    virtual std::string type_label() const override
    {
      return this->get_override("type_label")();
    }


};
#endif
