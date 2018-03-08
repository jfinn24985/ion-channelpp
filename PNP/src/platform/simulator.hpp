#ifndef IONCH_PLATFORM_SIMULATOR_HPP
#define IONCH_PLATFORM_SIMULATOR_HPP

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

#include <boost/noncopyable.hpp>

#include "utility/archive.hpp"

#include "trial/choice.hpp"
#include "trial/base_chooser.hpp"
#include "evaluator/base_evaluator.hpp"
#include "observable/base_sink.hpp"
#include <boost/signals2/signal.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/function.hpp>

#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "particle/specie.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include "observable/report_manager.hpp"
#include <cstddef>
#include <boost/random.hpp>

#include "utility/random.hpp"
#include <string>
#include <iostream>
#include "utility/unique_ptr.hpp"
#include "platform/range_t.hpp"

namespace particle { class change_set; } 
namespace core { class constants; } 
namespace utility { class fp_env; } 
namespace utility { class machine_env; } 
namespace core { class strngs; } 
namespace particle { class ensemble; } 
namespace core { class input_delegater; } 
namespace geometry { class grid_generator; } 
namespace core { class input_base_reader; } 
namespace geometry { class coordinate; } 
namespace geometry { class coordinate_set; } 
namespace core { class input_document; } 

namespace platform {

// Carry out a MC simulation.
//
// This class (and its descendents) is the most fundamental
// class for simulation.  It orchestrates the running of the
// simulation. Its major roles are to manage building the
// simulation, running the simulation and sampling the simulation.
//
// To handle different simulation cells you derive from
// this class. The derived class will then specialise the
// initialisation functions that create choices, observables etc
// that are cell specific.
//
// Base class of different types of MC simulation.
//
// Each simulator contains:
//
//* transformer_ : a 'stepper' object that runs the main loop
//of the simulation.
//
//* simualtion_ : a 'system' object that contains the information
//about the ensemble, particles etc that are being simulated.
//
//Each simulator refers to:
//
//* evaluators_ : a set of evaluator objects for calculating
//the energy.
//
//* observables_ : a set of observable objects that collect data
//from the simulations.
//
//. RESPONSIBILITIES .
//
//* Coordinate run order with other simulators.
//
//* Simulation ownership: Coordinate objects taking part
//in a single simulation. No objects are shared between
//simulators. These objects include energy evaluators
//(evaluators), data collectors (observables), simulation component
//descriptions (species and ensemble), global simulation
//parameters (temperature, output directory, etc.).
//
//* Initiation of the simulation: Configure the stepper object
//that will perform the trials of the simulation. Coordinate
//connection of the energy evaluators and data collectors to
//the stepper object.
//
//* Simulation maintenance: Provide controlled write access to
//global simulation information.
//
//* Simulator subtype methods: Methods that relate to the geometry
//of the simulation cell.
//
//  This class contains the loop sizes, observables and evaluators.
//  It has two simulation run functions; 'run' for standard
//  simulations and 'run_imc' for running iterative simulations
//
//  Signals available and their intents:
//
//  - 'prepare' - call to indicate that the system has created the
//  initial simulation state and is about to start a phase of the
//  simulation.  Arguments are this object (sim). This method will
//  be called at the beginning of a simulation and when switching
//  from the thermalization to production phase. **Objects in
//  the observables list do not need to connect to this signal as
//  they will be called directly for a method of this name.
//
//  - 'report' - call to write observed data to a log stream and
//  to data files. Arguments are this object and a stream object
//  (sim, out).
//
//  - 'inner-loop' - call at completion of every MC
//  trial. Arguments are the change object (change)
//
//  - 'outer-loop' - call after a certain number of MC trials. The
//  number of trials should be proportional to the decorrelation
//  time of the simulation allowing reasonably independent
//  samples to be made. Argument will be this object (sim).

class simulator : public boost::noncopyable
 {
  friend class simulator_meta;

   public:
    //Type of the outer-loop signals
    typedef boost::signals2::signal<void (simulator const&)> sampler_signal_type;

    //The type of a mathematical array for use in calculating inter-particle
    //distances.
    typedef std::vector< double > array_type;

    typedef boost::function1<void, const simulator*> checkpoint_fn_type;


   private:
    // Trial generators
    boost::ptr_vector< trial::base_choice > choices_;

    //  Choice generators.
    boost::ptr_vector< trial::base_chooser > choosers_;

    //The set of in-use evaluators
    boost::ptr_vector< evaluator::base_evaluator > evaluators_;

    //Set of species (python only)
    std::vector< particle::specie > species_;

    //Manage data collectors.
    boost::shared_ptr< observable::report_manager > reporter_;

    //The simulation particle set
    //
    //(CREATED IN CTOR --> ALWAYS EXISTS)
    boost::shared_ptr< particle::ensemble > ensemble_;

    //Function to checkpoint the system.
    checkpoint_fn_type checkpoint_function_;

    //The running energy estimate
    double energy_;

    //The number of step cycles to perform in the equilibration phase of the simulation.
    std::size_t equilibration_interval_;

    //The number of trials performed in a step cycle.
    std::size_t inner_loop_size_;

    //The current number of outer loop cycles (0 based) that have been performed.
    //
    //(Allow for restarts from last checkpoint position)
    std::size_t outer_loop_index_;

    //The number of step cycles to perform in the production phase of the simulation.
    std::size_t production_interval_;

    //The number of step cycles between each report.
    std::size_t report_interval_;

    //The target number of particles
    std::size_t target_particles_;

    //Simulation temperature
    double temperature_kelvin_;

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

    //Signal source for 'report' signal [void (simulator const&)]
    sampler_signal_type prepare_sig_;

    //Boost Format template defining the directory for output files.
    //Directory name is generated using
    //
    //  boost::format(output_dir_fmt_) % run_index % run_uuid
    //
    //The default is a format that gives '006/dat' where '006' is
    //the three digit run number string and the UUID is not used.
    std::string output_dir_fmt_;

    //The input file name template
    std::string filename_base_;

    // User input title for this simulation run
    std::string run_title_;

    // The unique identifier for a simulation
    std::string run_uuid_;

    //The index of this run. 
    //
    //Defaults to 1.
    std::size_t run_index_;

    // The permittivity of the system
    double permittivity_;

// LIFETIME METHODS
   public:
    //Main ctor
    simulator();

    virtual ~simulator();


   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version);
// SIMULATION INITIATION
//
// Coordinate the setting up of the simulation
// trial loop.
// 

   public:
    // Transfer ownership of chooser 'choice' into our choices list.
    //
    
    void add_chooser(trial::base_chooser* choice);

    // Transfer ownership of an energy evaluators into the evaluators list
    
    void add_evaluator(evaluator::base_evaluator* evltr);

    // Add (copy) specie to set and return the index of the specie
    //
    // (non C++)
    std::size_t add_specie(const particle::specie & spc);


   private:
    // Register the input file meta classes for this simulation.
    virtual void build_reader(core::input_delegater & decoder) = 0;


   public:
    // Details about the current simulation to be written to the
    // log at the start of the simulation.
    void description(std::ostream & os) const;


   protected:
    // Details about the current simulation object.
    virtual void do_description(std::ostream & os) const;


   public:
    //Write the name of the program and the license.
    void license(std::ostream & os) const;


   private:
    //Implement in derived classes to write the name of 
    //the program and any citation information.
    virtual void do_license(std::ostream & os) const = 0;


   public:
    // Generate a series of possible filenames and return the
    // first filename that is found. Throw an error if no filenames
    // are found. The template string is processed using 'printf'
    // like function.
    //
    // \post exists(return val)
    
    std::string find_input_filename() const;

    // Generate a series of possible filenames and return the
    // first filename that is found. Throw an error if no filenames
    // are found.  The template is processed using boost::format.
    //
    // \post exists(return val)
    
    std::string find_input_filename(int d) const;

    //Generate an initial ensemble to run a simulation.
    void generate_simulation(std::ostream & oslog);


   private:
    // Pass ensemble to derived classes so they can generate
    // the initial set of particles.
    virtual std::unique_ptr< geometry::grid_generator > do_generate_simulation(std::ostream & oslog) = 0;


   public:
    //  The maximum input file version to be understood by this program
    virtual std::size_t get_max_input_version() const = 0;


   private:
    //Generate a UUID string
    static std::string make_uuid_string();


   public:
    // Generate a random value to seed the random number generator.
    //
    // This uses an operating system provided random source to generate
    // a single random unsigned int suitable to seed the psuedo-random
    // number generator. 
    
    static uint32_t random_seed_value();

    //Read in the given input file
    void read_input(core::input_base_reader & reader, std::ostream & oslog);

    //Get the run UUID string
    std::string run_uuid() const
    {
      return this->run_uuid_;
    }

    //Get the run title
    std::string run_title() const
    {
      return this->run_title_;
    }

    //Get the index (or 0) of the current application run
    std::size_t run_index() const
    {
      return this->run_index_;
    }

    void set_checkpoint_function(checkpoint_fn_type fn)
    {
      this->checkpoint_function_ = fn;
    }

    void set_report_interval(std::size_t val)
    {
      this->report_interval_ = val;
    }

    void set_equilibration_interval(std::size_t val)
    {
      this->equilibration_interval_ = val;
    }

    // Set the template/regular expression for deriving input filenames.
    void set_filename_base(std::string fmt)
    {
     this->filename_base_ = fmt;
    }

    void set_inner_loop_size(std::size_t val)
    {
      this->inner_loop_size_ = val;
    }

    //Set template string used to create directory path for 
    //output files.
    //
    //Formatting is based on Boost Format so the string can
    //be in Boost Format, Posix printf or printf style.
    void set_output_dir_fmt(std::string fmt)
    {
      this->output_dir_fmt_ = fmt;
    }

    void set_production_interval(std::size_t val)
    {
      this->production_interval_ = val;
    }

    // Reset the random number generate with the given seed. It
    // should generally only be done once.
    void set_random_seed(uint32_t seedval);

    //Set the run number. This must be called before the input file is
    //read.
    //
    //\pre run_uuid not set.
    void set_run_number(int num)
    {
      this->run_index_ = num;
    }

    //Set a textual title for this simulation
    void set_run_title(std::string val)
    {
      this->run_title_ = val;
    }
    // Set the intended average number of particles in the simulation.
    void set_target_count(std::size_t val)
    {
      this->target_particles_ = val;
    }

    //Set the temperature (in Kelvin)
    void set_temperature(double val)
    {
      this->temperature_kelvin_ = val;
    }
    // Set the the relative permittivity of the solvent medium (relative 
    // to permittivity in a vacuum)
    void set_solvent_permittivity(double val)
    {
      this->permittivity_ = val;
    }
// SIMULATION OWNERSHIP
//
// Manage the set of objects required to run a
// simulation.
//
    //Compute the distances between given position and existing positions.
    //
    //\pre rij.size <= ens.size
    //\post rij[0:startindex] === 0
    //\post rij[ens.size:] undefined
    void compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, array_type & rij, std::size_t endindex, std::size_t startindex = 0) const;


   private:
    //Compute the distances between any new position and existing positions.
    //
    //\post rij.size == ens.size
    //\post rij[0:start_index] == 0.0
    
    virtual void do_compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, array_type & rij, std::size_t endindex, std::size_t start_index = 0) const = 0;


   public:
    // Is a particle at the given position and radius in a valid position
    // in the system?
    virtual bool is_valid_position(geometry::coordinate & coord, std::size_t ispec) const = 0;

    //Perform the simulation
    void run(std::ostream & oslog);


   private:
    // Derived class implementation of run method.
    virtual void do_run(std::ostream & oslog);


   protected:
    // Method for setting up and running a set of MC trials.
    //
    // The 'do_run' method's jobs is to determine the sequence of
    // trials that should be performed. This method's job is to perform
    // the trials. It is not anticipated that this method would need to
    // be altered.
    void run_loop(std::size_t loopcount, std::ostream & out);

// SIMULATION INITIATION
//
// Coordinate the setting up of the simulation
// trial loop.
// 

   private:
    // Report simulator statistics and issue the 'report' signal.
    //
    // * monitor that running energy and computed energy remain close
    void do_report(std::ostream & os);


   public:
    // Computer the output directory for the current simulator object
    // from the output_dir_fmt template.
    //
    // SIDE_EFFECT: Create directory if necessary
    std::string compute_output_dir() const
    {
      return this->compute_dir(this->output_dir_fmt_);
    }


   private:
    // Implementation of computing a directory from a given format text and
    // creating the directory if necessary.
    std::string compute_dir(std::string fmt) const;


   public:
    // The current value of the running energy
    double energy() const
    {
      return this->energy_;
    }

    // The number of outer loops that will be done before data collection commences.
    // * checkpoints will be written (every isave)
    // * data may be collected but will be reset at end of thermalisation
    std::size_t equilibration_interval() const
    {
      return this->equilibration_interval_;
    }

    // Set the template/regular expression for deriving input filenames.
    const std::string& filename_base() const
    {
       return this->filename_base_;
    }

    // Get the current particle set
    const particle::ensemble& get_ensemble() const
    {
      return *(this->ensemble_.get ());
    }

    // Access the list of evaluators
    //
    // (non python)
    const_range_t get_evaluators() const;

    // Get the current outer loop index
    std::size_t get_outer_loop_index() const
    {
      return this->outer_loop_index_;
    }

    // Get the permittivity of the solvent
    double get_permittivity() const
    {
      return this->permittivity_;
    }

    // Access source of randomness
    utility::random_distribution & get_random()
    {
      return this->ranf_;
    }

    //Access report manager
    observable::report_manager const& get_reporter() const
    {
      return *(this->reporter_);
    }
    //Access report manager (non-const)
    observable::report_manager& get_reporter()
    {
      return *(this->reporter_);
    }
    // Get the definition of a specie by index
    particle::specie const& get_specie(std::size_t key) const
    {
      return this->species_[key];
    }
    // Get the definition of a specie by index
    particle::specie& get_specie(std::size_t key)
    {
      return this->species_[key];
    }

    // Get the key of a specie from a label. 
    //
    // \pre has_specie(label)
    // (C++ throws out_of_range if not found)
    // (Python raises KeyError if not found)
    
    std::size_t get_specie_key(std::string label) const;

    // Get the complete specie set
    std::vector< particle::specie > const& get_species() const
    {
      return this->species_;
    }

    // Get the temperature (in Kelvin)
    double get_temperature() const
    {
      return this->temperature_kelvin_;
    }

    // Do we have a specie with this label?
    bool has_specie(std::string name) const;

    // The number of trials in an inner loop
    std::size_t inner_loop_size() const
    {
      return this->inner_loop_size_;
    }

    //Calculate the current ionic strength (N/volume)
    double ionic_strength() const;

    // The output directory template.
    std::string output_dir_fmt() const
    {
      return this->output_dir_fmt_;
    }

    // Calculate the permittivity at the given location.
    //
    // Default implementation returns value of get_permittivity
    virtual double permittivity_at(const geometry::coordinate & pos) const;

    // The number of outer loops to perform during the simulation
    std::size_t production_interval() const
    {
      return this->production_interval_;
    }

    // The number of outer loops between saving of the simulation state.
    std::size_t report_interval() const
    {
      return this->report_interval_;
    }

    // The number of different species
    //
    // ( equivalent to get_species().size() (
    std::size_t specie_count() const
    {
      return this->species_.size();
    }

    // How many particles we want in the simulation
    //
    // The intended average (solute) particle number in the
    // simulation.
    std::size_t target_count() const
    {
      return this->target_particles_;
    }


   private:
    // Calculate the total energy
    void total_energy();


   public:
    // Return the accessible volume of the given specie.
    virtual double volume(std::size_t ispec) const = 0;

    // Write contents of simulation as an input document
    void write_document(core::input_document & wr);


   private:
    // Write extra data of derived simulator object into the input
    // document object at the given section.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const = 0;


   public:
    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_x() const = 0;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_y() const = 0;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_z() const = 0;


};
template<class Archive> inline void simulator::serialize(Archive & ar, const unsigned int version) 
{
  ar & choices_;
  ar & choosers_;
  ar & evaluators_;
  ar & species_;
  ar & reporter_;
  ar & ensemble_;
  ar & report_interval_;
  ar & energy_;
  ar & equilibration_interval_;
  ar & inner_loop_size_;
  ar & outer_loop_index_;
  ar & production_interval_;
  ar & target_particles_;
  ar & temperature_kelvin_;
  ar & rgen_;
  ar & ranf_;
  // Signals are not archived
  ar & output_dir_fmt_;
  ar & filename_base_;
  ar & run_title_;
  ar & run_uuid_;
  ar & run_index_;
  ar & permittivity_;

}


} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY(platform::simulator);
#endif
