#ifndef IONCH_PLATFORM_SIMULATION_MANAGER_HPP
#define IONCH_PLATFORM_SIMULATION_MANAGER_HPP

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

#include "utility/archive.hpp"

#include <cstddef>
#include <iostream>
#include <string>

namespace core { class input_document; } 
namespace platform { class simulation; } 

namespace platform {

// The style of simulation that is running.  This manages the
// super-loop variants and can be selected from the input file
// at runtime.
//
//
// The simulation manager is responsible for:
//
// * Controlling the main simulation loop(s): ''run''
//
// * Instantiating itself (and any subobjects) from information
// in the input file.
//
//
// The following methods must be overwritten:
//
// : run : Prepare and run the simulation. A standard simulation
// would have the sequence: sim.prepare, sim.run_loop(thermal),
// sim.prepare, sim.run_loop(production), [sim.report].
//
// : do_description, license : These methods produce documentation
// for the simulation.
//
// : do_write_part_document, type_label, set_parameters : These
// methods are used when reading and writing an input file.
//

class simulation_manager
 {
   private:
    //The number of step cycles to perform in the equilibration phase of the simulation.
    std::size_t equilibration_interval_;

    //The number of step cycles to perform in the production phase of the simulation.
    std::size_t production_interval_;


   public:
    simulation_manager()
    : equilibration_interval_()
    , production_interval_()
    {}

    virtual ~simulation_manager();


   private:
    // No move
    simulation_manager(simulation_manager && source);

    // No copy
    simulation_manager(const simulation_manager & source);

    // No assignment
    simulation_manager & operator=(const simulation_manager & source);


  friend class boost::serialization::access;    template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {
      ar & equilibration_interval_;
      ar & production_interval_;
    }

   public:
    // Details about the current simulation to be written to the
    // log at the start of the simulation.
    void description(std::ostream & os) const;


   private:
    // Details about the current storage object.
    virtual void do_description(std::ostream & os) const = 0;


   public:
    // The number of outer loops that will be done before data collection commences.
    // * checkpoints will be written (every isave)
    // * data may be collected but will be reset at end of thermalisation
    std::size_t equilibration_interval() const
    {
      return this->equilibration_interval_;
    }

    // The licence or citation information for this 
    // simulation subtype. This supplements information
    // writen by the simulation class.
    virtual void license(std::ostream & os) const = 0;

    // The number of outer loops to perform during the simulation
    std::size_t production_interval() const
    {
      return this->production_interval_;
    }

    // Label that identifies this simulation manager subtype in
    // the input file (virtual access).
    virtual std::string type_label() const = 0;

    // Write data of simulation manager object into the input
    // document object at the given section.
    void write_part_document(core::input_document & wr, std::size_t ix) const;


   private:
    // Write extra data of derived simulator object into the input
    // document object at the given section.
    virtual void do_write_part_document(core::input_document & wr, std::size_t ix) const = 0;


   public:
    void set_equilibration_interval(std::size_t val)
    {
      this->equilibration_interval_ = val;
    }

    void set_production_interval(std::size_t val)
    {
      this->production_interval_ = val;
    }

    // Perform the simulation. Base class just does
    // sim.run_loop(equil_int) + sim.run_loop(prod_int).
    
    virtual bool run(simulation & sim, std::ostream & oslog) = 0;


};

} // namespace platform
#endif
