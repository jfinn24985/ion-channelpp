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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "platform/simulation.hpp"
#include "utility/archive.hpp"

#include "observable/report_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "particle/particle_manager.hpp"
#include "trial/choice_manager.hpp"
#include "platform/simulation_manager.hpp"
#include "platform/storage_manager.hpp"
#include "geometry/base_region.hpp"
#include "core/input_delegater.hpp"
#include "core/input_base_reader.hpp"
#include "core/input_document.hpp"

// manual includes
#include "core/input_help.hpp"
#include "core/strngs.hpp"
#include "evaluator/hard_sphere_overlap.hpp"
#include "geometry/periodic_cube_region.hpp"
#include "observable/base_observable.hpp" // for serialization
#include "observable/base_sink.hpp"       // for serialization
#include "particle/change_set.hpp"
#include "particle/ensemble.hpp"
#include "platform/imc_simulation.hpp"
#include "platform/serial_storage.hpp"
#include "platform/simulator_meta.hpp"
#include "platform/standard_simulation.hpp"
#include "trial/base_chooser.hpp"         // for serialization
#include "trial/choice.hpp"               // for serialization
#include "utility/fuzzy_equals.hpp"
#include "utility/machine.hpp"
// -
#include <fstream>
//-
namespace platform {

// The maximum understood input file version.
std::size_t simulation::max_input_version_ = {1};

simulation::simulation(boost::shared_ptr< geometry::base_region > cell_region, boost::shared_ptr< storage_manager > fstype)
: rman_( new observable::report_manager )
, eman_( new evaluator::evaluator_manager )
, gman_( new geometry::geometry_manager( cell_region ) )
, pman_( new particle::particle_manager )
, cman_( new trial::choice_manager )
, simtype_()
, fstype_( fstype )
, inner_loop_size_()
, energy_()
, outer_loop_index_( 0ul )
, rgen_( new boost::mt19937 )
, ranf_( rgen_ )
, report_interval_()
, run_title_()
{}

simulation::simulation(bool test)
: simulation( boost::shared_ptr< geometry::base_region >( new geometry::periodic_cube_region( "cell", 10.0 ) ), boost::shared_ptr< platform::storage_manager >( new platform::serial_storage ) )
{}

simulation::~simulation()
{}

// Access simulation subtype object.
const simulation_manager& simulation::manager() const
{
  UTILITY_REQUIRE( this->has_manager(), "Can not get simulation subtype before it is set." );
  return *(this->simtype_);
}

// Reset the random number generate with the given seed. It
// should generally only be done once.
void simulation::set_random_seed(uint32_t seedval) 
{
  this->rgen_->seed( seedval );

}

// Set the intended average number of particles in the simulation.
void simulation::set_target_count(std::size_t val)
{
  this->pman_->set_target_count( val );
}

//Set the temperature (in Kelvin)
void simulation::set_temperature(double val)
{
  this->eman_->temperature( val );
}

// Set the the relative permittivity of the solvent medium (relative 
// to permittivity in a vacuum)
void simulation::set_solvent_permittivity(double val)
{
  this->eman_->permittivity( val );
}

// Adds factory methods and input parsers for all the types that can be instantiated from 
// the input file.
void simulation::build_input_delegater(core::input_delegater & delegate)
{
  //////////////////
  // Evaluator types
  evaluator::evaluator_manager::build_input_delegater( this->eman_, delegate );
  //////////
  // Species
  particle::particle_manager::build_input_delegater( this->pman_, delegate );
  ///////////////
  // Region types
  geometry::geometry_manager::build_input_delegater( this->gman_, delegate );
  
  ///////////////
  // Choice types
  trial::choice_manager::build_input_delegater( this->cman_, delegate );
  
  ///////////////////
  // Observable types
  observable::report_manager::build_input_delegater( this->rman_, delegate );
  
  ////////////////
  // Storage types
  platform::storage_manager::build_input_delegater( this->fstype_, delegate );
  
  ///////////////////
  // Simulation types
  {
    boost::shared_ptr< platform::simulator_meta > var( new platform::simulator_meta( *this ) );
    platform::standard_simulation::add_definition( *var );
    platform::imc_simulation::add_definition( *var );
  
    delegate.add_input_delegate( var );
  }

}

// Get the log (from the storage manager)
std::ostream & simulation::get_log() const
{
  return this->fstype_->get_log();
}

// Write the name of the program and the license. If instantiated,
// get extra license information from simulation manager.
void simulation::license(std::ostream & os) const
{
  if( this->has_manager() )
  {
    this->simtype_->license( os );
  }
  
  os << core::strngs::horizontal_bar () << "\n";
  os << "This software is free software: you can redistribute it and/or modify\n";
  os << "it under the terms of the GNU General Public License as published by\n";
  os << "the Free Software Foundation, either version 3 of the License, or\n";
  os << "(at your option) any later version.\n";
  os << "\n";
  os << "This program is distributed in the hope that it will be useful,\n";
  os << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
  os << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
  os << "GNU General Public License for more details.\n";
  os << "\n";
  os << "You should have received a copy of the GNU General Public License\n";
  os << "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";
  os << core::strngs::horizontal_bar () << "\n";

}

// Write details about the current program and execution
// environment to os.
void simulation::program_description(std::ostream & os) const 
{
  // host and environment information
  os << core::strngs::horizontal_bar() << "\n";
  utility::machine_env::create()->description( os );
  
  os << "\n External Libraries\n";
  os << " ------------------\n";
  // LIBRARY INFORMATION
  // os << "BLAS/LAPACK LIBRARY" << util::lapack::version () "\n";
  
  // RANDOM NUMBER GENERATOR USED:
  os << " random number generator : " << utility::random_distribution::version() << "\n";
  os << "\n";
  
  // Print compile time constants
  core::constants::description( os );
  os << core::strngs::horizontal_bar() << "\n";

}

// Read in the given input source.
//
// \post has_manager
// \post report.has_sink
void simulation::read_input(core::input_base_reader & reader)
{
  // Random seed value
  core::input_delegater decoder( this->get_max_input_version() );
  
  this->build_input_delegater( decoder );
  
  decoder.read_input( reader );
  
  // Ensure that overlap evaluator is present.
  {
    bool add_overlap_eval = true;
    const std::string label = evaluator::hard_sphere_overlap::type_label_();
    for( auto const& evltr : this->eman_->get_evaluators() )
    {
      if( label == evltr.type_label() )
      {
        add_overlap_eval = false;
      }
    }
    if( add_overlap_eval )
    {
      std::vector< core::input_parameter_memo > dummy( 1 );
      dummy[ 0 ].set_name( core::strngs::fsend() );
      this->eman_->add_evaluator( evaluator::hard_sphere_overlap::make_evaluator( dummy ) );
    }
  }
  
  // After processing input we can now set up the output
  // location.
  if( not this->rman_->has_sink() )
  {
    this->rman_->set_sink( this->fstype_->open_output() );
  }

}

// Generate an initial particle configuration to be able to
// run a simulation.
//
// In general the "main" method handles creating
// the from an input file using "read_input" then
// "generate_simulation". This should prepare the particle
// system for performing a simulaton. This method is
// publically available for manually constructing a
// simulation, for example during testing.

void simulation::generate_simulation()
{
  this->pman_->generate_simulation( *(this->gman_), this->get_random(), this->fstype_->get_log() );
  this->fstype_->get_log() << core::strngs::horizontal_bar() << "\n";
}

// Finalize the volume or particle count based on
// ionic-strength and the volume or particle count
// set in the input file.

void simulation::resolve_system_size()
{
  this->pman_->generate_simulation( *(this->gman_), this->get_random(), this->fstype_->get_log() );
}

// Write contents of simulation as an input document
//
// \pre has_manager
void simulation::write_document(core::input_document & wr) 
{
  UTILITY_REQUIRE( this->has_manager(), "Can not write input document before setting simulation manager." );
  // Add simulator section
  std::size_t ix = wr.add_section( core::strngs::simulator_label() );
  auto &sec = wr[ ix ];
  sec.add_entry( core::strngs::fsname(), "\"" + this->run_title() + "\"" );
  sec.add_entry( core::strngs::inner_label(), this->inner_loop_size() );
  sec.add_entry( core::strngs::fsisav(), this->report_interval() );
  sec.add_entry( core::strngs::fsntrg(), this->pman_->target_count() );
  sec.add_entry( core::strngs::fstsi(), this->eman_->temperature() );
  sec.add_entry( core::strngs::fsepsw(), this->eman_->permittivity() );
  this->simtype_->write_part_document( wr, ix );
  
  this->fstype_->write_document( wr );
  
  this->rman_->write_document( wr );
  this->eman_->write_document( wr );
  this->gman_->write_document( wr );
  this->pman_->write_document( wr );
  this->cman_->write_document( wr );

}

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

std::size_t simulation::compute_degrees_of_freedom()
{
  std::size_t dof{ 0 };
  std::size_t number_of_particles{ this->pman_->get_ensemble().count() };
  //  The number of degrees of freedom is calculated from
  //  the following parameters.
  // 
  //  * Each energy evaluator contributes 1 DoF.
  
  dof = this->eman_->size();
  
  //  * If the system is Grand Canonical then:
  // 
  if( this->cman_->is_grand_canonical() )
  {
    const std::size_t target_count{ this->pman_->target_count() };
    //  ** The number of particles for calculating DoF 
    //  is target_count + sqrt(target_count). This comes
    //  from our expectation that the particle number will
    //  form a (semi-)uniform distribution around the 
    //  target count with a standard deviation of 
    //  sqrt(target_count).
    std::size_t number_of_solute{ target_count };
    if( target_count != 0ul )
    {
      number_of_solute += static_cast< std::size_t >( std::sqrt( target_count ) );
      number_of_particles = ( number_of_particles > target_count ? number_of_particles - target_count : 0ul );
    }
    //  ** Each non-solute particle contributes
    //  3 DoF.
    //  ** Each solute particle contributes 3+1 DoF
    //  from being able to have different specie type.
    dof += number_of_particles * 3 + number_of_solute * 4;
    //  ** Each solute specie type contributes 1 DoF. 
    dof += this->pman_->specie_count();
  }
  else
  {
    //  * The position of the particles contribute
    //  3 DoF / particle.
    dof += number_of_particles * 3;
  }
  // Return degrees of freedom
  return dof;

}

// Details about the current simulation to be written to the
// log at the start of the simulation.
//
// \pre has_manager
void simulation::description(std::ostream & os) const 
{
  UTILITY_REQUIRE( this->has_manager(), "Can not get description before setting simulation manager." );
  // File/directory names etc.
  this->fstype_->description( os );
  // The particulars of this simulation object.
  this->simtype_->description( os );
  this->do_description( os );
  
  os << core::strngs::horizontal_bar() << "\n";
  
  this->pman_->description( os ); // reports target count
  // NO description : this->gman_->do_description( os );
  this->eman_->description( os ); // report temperature and epsw
  this->cman_->description( os ); // choice rate table
  this->rman_->description( os ); // UUID and observers
  os << core::strngs::horizontal_bar() << "\n";

}

// Details about the current simulation object.
//
// \pre has_manager
void simulation::do_description(std::ostream & os) const 
{
  os << core::strngs::horizontal_bar() << "\n";
  os << " Standard intervals\n";
  os << "-------------------\n";
  os << "   trials per \"step\" : " << this->inner_loop_size_ << "\n";
  os << " steps between reports : " << this->report_interval_ << "\n";
  os << "    current step index : " << this->outer_loop_index_ << "\n";

}

// Report simulator statistics and issue the 'report' signal.
//
// * monitor that running energy and computed energy remain close
void simulation::do_report() 
{
  // CALLED AT EACH CHECKPOINT
  std::ostream &os = this->fstype_->get_log();
  os << "\n" << core::strngs::horizontal_bar() << "\n";
  os << " SIMULATION STEP: " << this->outer_loop_index_ << "\n";
  // recalculate total energy
  const double ent(this->energy());
  this->total_energy();
  os << "  total energy of simulation    : " << this->energy() << "\n";
  if (std::abs(ent - this->energy()) > 1.E-6)
  {
    os << "  ######### problems energy ################\n";
    os << "        running energy          : " << ent << "\n";
  }
  os << "        difference to running   : " << this->energy() - ent << "\n";
  
  this->rman_->on_report( os );
  os << core::strngs::horizontal_bar() << "\n";

}

// Set up evaluators, observers and trial choices in preparation for
// running a set of MC trials. These objects should reset any
// internal state and configure themselves for the simulation
// particle configuration and cell geometry.

void simulation::prepare()
{
  // Get reference to FP environment
  utility::fp_env &env(utility::fp_env::env_);
  // Check for FP error before doing anything
  if (not env.no_except())
  {
    if (0 != ((~env.Inexact) & env.except()))
    {
      UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except()),  " Floating point exception : "+ env.error_message ());
    }
    env.reset();
  }
  
  // Rebuild choices
  this->cman_->prepare( this->pman_->get_species(), *(this->gman_), this->ranf_ );
  
  // Reset observables
  this->rman_->prepare( *(this->pman_), *(this->gman_), *(this->eman_) );
  
  // Reset the evaluators
  this->eman_->prepare( *(this->pman_), *(this->gman_) );
  
  if ( not env.no_except() )
  {
    if (0 != ((~env.Inexact) & env.except()))
    {
      this->fstype_->get_log() << "Floating point exception during simulation preparation \"" << env.error_message () << "\"\n";
      UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                      , " Floating point exception : "+ env.error_message ());
    }
    env.reset();
  }
  // Calculate/reset the system's total energy (can only be done after preparing
  // the evaluators.)
  this->total_energy();
  if ( not env.no_except() )
  {
    if (0 != ((~env.Inexact) & env.except()))
    {
      this->fstype_->get_log() << "Floating point exception during calculation of initial energy\n \"" << env.error_message () << "\"\n";
      UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                      , " Floating point exception : "+ env.error_message ());
    }
    env.reset();
  }

}

// Method for setting up and running a set of MC trials.
//
// The 'do_run' method's jobs is to determine the sequence of
// trials that should be performed. This method's job is to perform
// the trials. It is not anticipated that this method would need to
// be altered.
void simulation::run_loop(std::size_t loopcount)
{
  // Get reference to FP environment
  utility::fp_env &env(utility::fp_env::env_);
  
  // Set report interval to some meaningful default if not set
  if (this->report_interval_ == 0)
  {
    this->report_interval_ = std::max(1ul, loopcount / 5);
  }
  const std::size_t inner_loop_size{ std::max( this->inner_loop_size_, this->compute_degrees_of_freedom() ) };
  if( inner_loop_size != this->inner_loop_size_ )
  {
    this->fstype_->get_log() << core::strngs::horizontal_bar() << "\n\n";
    this->fstype_->get_log() << "    NOTE\n\n";
    this->fstype_->get_log() << "    The degrees of freedom of the simulation (" << inner_loop_size << ") is larger than\n    the requested inner loop size (" << this->inner_loop_size_ << "). Using the degrees of\n    freedom for the inner loop.\n\n";
    this->fstype_->get_log() << core::strngs::horizontal_bar() << "\n\n";
  }
  // Run the simulation
  auto const& ens = this->pman_->get_ensemble();
  for (size_t d9m8h2 = 0; d9m8h2 != loopcount; ++d9m8h2)
  {
    this->outer_loop_index_ += 1;
    for ( size_t t1b6b8 = 0; t1b6b8 != inner_loop_size; ++t1b6b8 )
    {
      // select a move
      double sofar = this->ranf_.uniform( 0.0, 1.0 );
      std::unique_ptr< particle::change_set > trial;
      trial = this->cman_->generate( *(this->pman_), *(this->gman_), this->ranf_, sofar );
      if ( not trial->fail() )
      {
        // Calculate distance vectors.
        for ( auto & atom : *trial )
        {
          if (atom.do_old)
          {
            this->gman_->calculate_distances_sq( atom.old_position, ens.get_coordinates(), atom.old_rij2, 0, ens.size() );
          }
          if (atom.do_new)
          {
            this->gman_->calculate_distances_sq( atom.new_position, ens.get_coordinates(), atom.new_rij2, 0, ens.size() );
          }
        }
        // Calculate energy
        this->eman_->compute_potential( *(this->pman_), *(this->gman_), *trial );
        if ( not env.no_except() )
        {
          if (0 != ((~env.Inexact) & env.except()))
          {
            this->fstype_->get_log() << "Floating point exception in computation of trial\n \""
               << env.error_message () << "\"\n"
               << "Loop [" << d9m8h2 << "][" << t1b6b8 << "]\n";
            UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                            , " Floating point exception : "+ env.error_message ());
          }
          env.reset();
        }
  
        if ( not trial->fail() )
        {
          trial->set_accept( this->ranf_.uniform(0.0,1.0) <= trial->metropolis_factor() );
          if (trial->accept())
          {
            this->energy_ += trial->energy();
            this->pman_->commit( *trial );
          }
        }
      }
      this->eman_->on_conclude_trial( *trial );
      this->rman_->on_trial_end( *trial );
      if (not env.no_except())
      {
        if (0 != ((~env.Inexact) & env.except()))
        {
          this->fstype_->get_log() << "Floating point exception in inner sampling\n \""
             << env.error_message () << "\"\n"
             << "Loop [" << d9m8h2 << "][END]\n";
          UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                          , " Floating point exception : "+ env.error_message ());
        }
        env.reset();
      }
    }
    // Note this->outer_loop_index_ is a 1-based count
    this->fstype_->get_log() << " STEP : " << (d9m8h2 + 1) << "(" << this->outer_loop_index_ << ")\n";
    this->rman_->on_sample( *(this->pman_), *(this->gman_), *(this->eman_) );
    if (((d9m8h2 + 1) % this->report_interval_) == 0)
    {
      this->do_report();
    }
    if (not env.no_except())
    {
      if (0 != ((~env.Inexact) & env.except()))
      {
        this->fstype_->get_log() << "Floating point exception in outer sampling\n \""
             << env.error_message () << "\"\n"
             << "Loop [" << d9m8h2 << "][END]\n";
        UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                        , " Floating point exception : "+ env.error_message ());
      }
      env.reset();
    }
    if (((d9m8h2 + 1) % this->report_interval_) == 0)
    {
      auto fsptr = this->fstype_->open_checkpoint();
      boost::archive::text_oarchive oa( *fsptr );
      oa << *this;
    }
  }
  

}

// Calculate the total energy
void simulation::total_energy() 
{
  UTILITY_REQUIRE(not this->eman_->empty (), "Can not calculate energy without evaluators.");
  this->eman_->prepare( *(this->pman_), *(this->gman_) );
  this->energy_ = this->eman_->compute_total_potential( *(this->pman_), *(this->gman_) );

}

bool simulation::main()
{
  // Output settings
  this->description( this->fstype_->get_log() );
  
  // Output interpreted copy of the input file
  {
    this->fstype_->get_log() << core::strngs::horizontal_bar() << "\n";
    core::input_document wr( this->get_max_input_version() );
    this->write_document( wr );
    wr.write( this->fstype_->get_log() );
    this->fstype_->get_log() << core::strngs::horizontal_bar() << "\n";
  }
  
  // Run the simulation
  return this->simtype_->run( *this, this->fstype_->get_log() );

}

bool simulation::restart()
{
  const std::string chpt_path{ this->fstype_->checkpoint_path() };
  if( not boost::filesystem::exists( chpt_path ) )
  {
    this->fstype_->get_log() << "Error: Checkpoint filename \"" << chpt_path
        << "\" does not exist.\n";
    return false;
  }
  std::ifstream chpt{ chpt_path };
  if( not chpt )
  {
    this->fstype_->get_log() << "Error: Checkpoint filename \"" << chpt_path
        << "\" is not readable.\n";
    return false;
  }
  boost::archive::text_iarchive ia( chpt );
  ia >> *this;
  return true;

}

void simulation::generate_help(std::string section, std::string param, std::ostream & log)
{
  core::input_delegater dg{ this->get_max_input_version() };
  core::input_help helper{};
  this->build_input_delegater( dg );
  dg.get_documentation( helper );
  if( not param.empty() )
  {
    helper.write( section, param, log );
  }
  else
  {
    if( section.empty() or not helper.has_section( section ) )
    {
      // top level parameter or all.
      helper.write( log );
    }
    else
    {
      // Write just one section.
      helper.write( section, log );
    }
  }

}


} // namespace platform
