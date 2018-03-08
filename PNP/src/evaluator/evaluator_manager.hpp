#ifndef IONCH_EVALUATOR_EVALUATOR_MANAGER_HPP
#define IONCH_EVALUATOR_EVALUATOR_MANAGER_HPP

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

#include <boost/range/iterator_range.hpp>

#include "evaluator/base_evaluator.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "utility/unique_ptr.hpp"
#include <iostream>
#include <cstddef>
#include <boost/serialization/shared_ptr.hpp>

namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace particle { class change_set; } 
namespace core { class input_document; } 
namespace core { class input_delegater; } 

namespace evaluator {

typedef boost::iterator_range<boost::ptr_vector< evaluator::base_evaluator >::const_iterator> const_range_t;
class evaluator_manager
 {
   private:
    //Set of evaluators for a simulation.
    boost::ptr_vector< base_evaluator > evaluators_;

    // The relative permittivity of the media
    double permittivity_;

    // The simulation temperature
    double temperature_;


   public:
    evaluator_manager();

    ~evaluator_manager();


   private:
    evaluator_manager(evaluator_manager & source) = delete;

    evaluator_manager(const evaluator_manager & source) = delete;

    evaluator_manager & operator=(evaluator_manager source) = delete;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version);

   public:
    // Transfer ownership of an energy evaluators into the evaluators list
    
    void add_evaluator(std::unique_ptr< base_evaluator >& evltr);

    // Transfer ownership of an energy evaluators into the evaluators list
    
    void add_evaluator(std::unique_ptr< base_evaluator >&& evltr);

    //Compute the change in potential energy.
    void compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const;

    //Calculate the total potential energy.
    double compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const;

    // Details about the current simulation to be written to the
    // log at the start of the simulation.
    void description(std::ostream & os) const;

    bool empty() const
    {
      return this->evaluators_.empty();
    }

    // Access the list of evaluators
    const_range_t get_evaluators() const;

    //  Called after the trial is complete. (default do nothing)
    virtual void on_conclude_trial(const particle::change_set & changes);

    double permittivity() const
    {
      return this->permittivity_;
    }

    void permittivity(double val)
    {
      this->permittivity_ = val;
    }
    

    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman);

    // The number of evaluators
    std::size_t size() const
    {
      return this->evaluators_.size();
    }

    // Standard relative permittivity of water (80.0)
    static double standard_aqueous_permittivity() { return 80.0; }

    // Standard room temperature (24°C) in Kelvin
    static double standard_room_temperature() { return 298.15; }

    double temperature() const
    {
      return this->temperature_;
    }

    void temperature(double val)
    {
      this->temperature_ = val;
    }
    

    // Write contents of simulation as an input document
    void write_document(core::input_document & wr);

    // Adds factory methods and input parsers for all the types that can be instantiated from 
    // the input file.
    static void build_input_delegater(boost::shared_ptr< evaluator_manager > eman, core::input_delegater & delegate);


};
template<class Archive> inline void evaluator_manager::serialize(Archive & ar, const unsigned int version) 
{
  ar & evaluators_;
  ar & permittivity_;
  ar & temperature_;

}


} // namespace evaluator
#endif
