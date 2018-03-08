#ifndef IONCH_PLATFORM_SIMULATION_HPP
#define IONCH_PLATFORM_SIMULATION_HPP

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

#include <boost/serialization/shared_ptr.hpp>
#include <cstddef>
#include <boost/random.hpp>

#include "utility/random.hpp"
#include <string>
#include <iostream>

namespace observable { class report_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace geometry { class geometry_manager; } 
namespace particle { class particle_manager; } 
namespace trial { class choice_manager; } 
namespace platform { class simulation_manager; } 
namespace platform { class storage_manager; } 
namespace geometry { class base_region; } 
namespace core { class input_delegater; } 
namespace core { class input_base_reader; } 
namespace core { class input_document; } 

namespace platform {

//Class of objects that manage a simulation.
//
//* Inheritance: The simulation state is separated into MC
//content, MC control and system interaction components.
//The MC control and system interaction can vary while the
//MC content remains unchanged. The simulation object has an
//object derived from the simulation_manager class to provide
//MC control and an object derived from the storage_manager
//class for system interaction.
//
//* Parallelism: A simulation object expects to be run
//serially. If a program runs multiple simulations they
//interact via the storage_manager derived class.
//
//
//
//Input file for simulation related objects
//
//fileversion X
//
//simulation
//inner-loop XXX
//report-loop XXX
//name "..."
//type standard
//equilibrium-loop XXX
//production-loop XXX
//end
//
//run
//type standard
//checkpoint-name "..."
//output-name "..."
//run-number X
//output-format "..."
//end

class simulation
 {
   private:
    boost::shared_ptr< observable::report_manager > rman_;

    boost::shared_ptr< evaluator::evaluator_manager > eman_;

    boost::shared_ptr< geometry::geometry_manager > gman_;

    boost::shared_ptr< particle::particle_manager > pman_;

    boost::shared_ptr< trial::choice_manager > cman_;

    boost::shared_ptr< simulation_manager > simtype_;

    boost::shared_ptr< storage_manager > fstype_;

    //The number of trials performed in a step cycle.
    std::size_t inner_loop_size_;

    //The running energy estimate
    double energy_;

    //The current number of outer loop cycles (0 based) that have been performed.
    //
    //(Allow for restarts from last checkpoint position)
    std::size_t outer_loop_index_;

    //The randomness source. 
    //
    //The default seed value is taken from the host 
    //system's random device and is not printed. If you
    //want to record the seed value, generate the value
    //externally, print the value and use 'set_random_seed'.
    boost::shared_ptr<boost::mt19937> rgen_;

    //The random number distribution generator. This uses
    //the rgen_ attribute as the randomness source.
    utility::random_distribution ranf_;

    //The number of step cycles between each report.
    std::size_t report_interval_;

    // User input title for this simulation run
    std::string run_title_;

    // The maximum understood input file version.
    static std::size_t max_input_version_;


   public:
    simulation(boost::shared_ptr< geometry::base_region > cell_region, boost::shared_ptr< storage_manager > fstype);


   private:
    // Deserialisation only
    simulation() = default;


   protected:
    simulation(bool test);


   public:
    ~simulation();


   private:
    simulation(simulation && source) = delete;

    simulation(const simulation & source) = delete;

    simulation & operator=(const simulation & source) = delete;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version);

   public:
    // The current value of the running energy
    double energy() const
    {
      return this->energy_;
    }

    // The number of trials in an inner loop
    std::size_t inner_loop_size() const
    {
      return this->inner_loop_size_;
    }

    // Get the current outer loop index. This number should be continuously increasing
    // and should not be reset.
    std::size_t get_outer_loop_index() const
    {
      return this->outer_loop_index_;
    }

    //  The maximum input file version to be understood by this program
    static std::size_t get_max_input_version()
    {
      return simulation::max_input_version_;
    }

    // Access source of randomness
    utility::random_distribution & get_random()
    {
      return this->ranf_;
    }

    // The number of outer loops between saving of the simulation state.
    std::size_t report_interval() const
    {
      return this->report_interval_;
    }

    // Get the simulation's set of trial selection choices.
    const trial::choice_manager& choices() const
    {
      return *(this->cman_);
    }

    // Get the simulation's set of particles.
    const evaluator::evaluator_manager& evaluators() const
    {
      return *(this->eman_);
    }

    // Test if simulation subtype object is set.
    bool has_manager() const
    {
      return ( this->simtype_ and true );
    }

    // Access simulation subtype object.
    const simulation_manager& manager() const;

    // Get the simulations set of particles.
    const particle::particle_manager& particles() const
    {
      return *(this->pman_);
    }

    // Get the simulations set of particles (non-constant).
    particle::particle_manager& particles()
    {
      return *(this->pman_);
    }

    // Get the simulation's set of regions.
    const geometry::geometry_manager& regions() const
    {
      return *(this->gman_);
    }

    // Get editable access to the simulation's sampler manager.
    observable::report_manager& get_report_manager()
    {
      return *(this->rman_);
    }

    // Get the simulation's samplers.
    const observable::report_manager& report() const
    {
      return *(this->rman_);
    }

    //Get the run title
    std::string run_title() const
    {
      return this->run_title_;
    }

    // Get the simulation's storage manager
    const storage_manager& storage() const
    {
      return *(this->fstype_);
    }

    void set_inner_loop_size(std::size_t val)
    {
      this->inner_loop_size_ = val;
    }

    void set_manager(boost::shared_ptr< simulation_manager > simtype)
    {
      std::swap( this->simtype_, simtype );
    }

    // Reset the random number generate with the given seed. It
    // should generally only be done once.
    void set_random_seed(uint32_t seedval);

    void set_report_interval(std::size_t val)
    {
      this->report_interval_ = val;
    }

    //Set a textual title for this simulation
    void set_run_title(std::string val)
    {
      this->run_title_ = val;
    }
    // Set the intended average number of particles in the simulation.
    void set_target_count(std::size_t val);

    //Set the temperature (in Kelvin)
    void set_temperature(double val);

    // Set the the relative permittivity of the solvent medium (relative 
    // to permittivity in a vacuum)
    void set_solvent_permittivity(double val);

    // Adds factory methods and input parsers for all the types that can be instantiated from 
    // the input file.
    void build_input_delegater(core::input_delegater & delegate);

    // Get the log (from the storage manager)
    std::ostream & get_log() const;

    // Write the name of the program and the license. If instantiated,
    // get extra license information from simulation manager.
    void license(std::ostream & os) const;

    // Write details about the current program and execution
    // environment to os.
    void program_description(std::ostream & os) const;

    // Read in the given input source.
    //
    // \post has_manager
    // \post report.has_sink
    void read_input(core::input_base_reader & reader);

    // Generate an initial particle configuration to be able to
    // run a simulation.
    //
    // In general the "main" method handles creating
    // the from an input file using "read_input" then
    // "generate_simulation". This should prepare the particle
    // system for performing a simulaton. This method is
    // publically available for manually constructing a
    // simulation, for example during testing.
    
    void generate_simulation();

    // Finalize the volume or particle count based on
    // ionic-strength and the volume or particle count
    // set in the input file.
    
    void resolve_system_size();

    // Write contents of simulation as an input document
    //
    // \pre has_manager
    void write_document(core::input_document & wr);

    // Calculate the degrees of freedom of the simulation.  This
    // gives meaningful results only after the simulation has been generated
    // and prepared.
    //
    // The number of degrees of freedom is calculated from the
    // following parameters.
    //
    // * Each energy evaluator contributes 1 DoF.
    //
    // * The position of the particles contribute 3 DoF / particle.
    //
    // * If the system is Grand Canonical then:
    //
    // ** The number of particles for calculating DoF is target_count
    // + sqrt(target_count). This comes from our expectation that
    // the particle number will form a (semi-)uniform distribution
    // around the target count with a standard deviation of
    // sqrt(target_count).
    //
    // ** Each solute particle contributes 1 more DoF from being
    // able to different species.
    //
    // ** Each solute specie type contributes 1 DoF.
    
    std::size_t compute_degrees_of_freedom();

    // Details about the current simulation to be written to the
    // log at the start of the simulation.
    //
    // \pre has_manager
    void description(std::ostream & os) const;


   private:
    // Details about the current simulation object.
    //
    // \pre has_manager
    virtual void do_description(std::ostream & os) const;

    // Report simulator statistics and issue the 'report' signal.
    //
    // * monitor that running energy and computed energy remain close
    void do_report();


   public:
    // Set up evaluators, observers and trial choices in preparation for
    // running a set of MC trials. These objects should reset any
    // internal state and configure themselves for the simulation
    // particle configuration and cell geometry.
    
    void prepare();

    // Method for setting up and running a set of MC trials.
    //
    // The 'do_run' method's jobs is to determine the sequence of
    // trials that should be performed. This method's job is to perform
    // the trials. It is not anticipated that this method would need to
    // be altered.
    void run_loop(std::size_t loopcount);


   private:
    // Calculate the total energy
    void total_energy();


   public:
    bool main();

    bool restart();

    void generate_help(std::string section, std::string param, std::ostream & log);


};
template<class Archive> inline void simulation::serialize(Archive & ar, const unsigned int version) 
{
  ar & rman_;
  ar & eman_;
  ar & gman_;
  ar & pman_;
  ar & cman_;
  ar & simtype_;
  ar & fstype_;
  ar & inner_loop_size_;
  ar & energy_;
  ar & outer_loop_index_;
  ar & ranf_;
  ar & report_interval_;
  ar & rgen_;
  ar & run_title_;

}


} // namespace platform
#endif
