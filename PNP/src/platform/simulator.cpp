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

#include "platform/simulator.hpp"
#include <boost/bind.hpp>

#include "particle/change_set.hpp"
#include "core/constants.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>

#include "utility/machine.hpp"
#include <uuid/uuid.h>

#include "particle/ensemble.hpp"
#include "core/input_delegater.hpp"
#include "geometry/grid.hpp"
#include "core/input_base_reader.hpp"
#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"
#include "core/input_document.hpp"

// manual includes
#include "core/strngs.hpp"
#include "platform/application.hpp"
#include "trial/change_set.hpp"
#include <boost/random/random_device.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

// For random seed value
#ifndef HAVE_NO_RANDOM_DEVICE
#include <boost/random/random_device.hpp>
#endif

namespace platform {

//Main ctor
simulator::simulator() 
: choices_()
, choosers_()
, evaluators_()
, species_()
, reporter_( new observable::report_manager )
, ensemble_( new particle::ensemble )
, checkpoint_function_()
, energy_( 0.0 )
, equilibration_interval_( 0 )
, inner_loop_size_( 0 )
, outer_loop_index_( 0 )
, production_interval_( 0 )
, report_interval_( 0 )
, target_particles_( 0 )
, temperature_kelvin_( 300.0 )
, rgen_( new boost::mt19937 )
, ranf_( rgen_ )
, prepare_sig_()
, output_dir_fmt_( "%1$03d" )
, filename_base_( "input.%1$03d.inp" )
, run_title_()
, run_uuid_( make_uuid_string() )
, run_index_( 1 )
, permittivity_( 80.0 )
{
  // Add simulator signals to signal set
  //CHECK this->reporter_->set_signal( "prepare", &this->prepare_sig_ );
}


simulator::~simulator() = default;

// Transfer ownership of chooser 'choice' into our choices list.
//

void simulator::add_chooser(trial::base_chooser* choice) 
{
   std::unique_ptr< trial::base_chooser > captured_ptr( choice );
   this->choosers_.push_back ( captured_ptr.get() );
   captured_ptr.release();
}

// Transfer ownership of an energy evaluators into the evaluators list

void simulator::add_evaluator(evaluator::base_evaluator* evltr) 
{
   std::unique_ptr< evaluator::base_evaluator > captured_ptr( evltr );
   this->evaluators_.push_back ( captured_ptr.get() );
   captured_ptr.release();
}

// Add (copy) specie to set and return the index of the specie
//
// (non C++)
std::size_t simulator::add_specie(const particle::specie & spc) 
{
  this->species_.push_back(spc);
  return this->species_.size() - 1;
}

// Details about the current simulation to be written to the
// log at the start of the simulation.
void simulator::description(std::ostream & os) const 
{
  this->license( os );
  
  // host and environment information
  utility::machine_env::create()->description ( os );
  
  os << "-- LIBRARIES --\n";
  // LIBRARY INFORMATION
  // os << "BLAS/LAPACK LIBRARY" << util::lapack::version () "\n";
  
  // RANDOM NUMBER GENERATOR USED:
  os << "RANDOM NUMBER GENERATOR > " << utility::random_distribution::version () << "\n";
  
  // Print compile time constants
  core::constants::description( os );
  os << core::strngs::horizontal_bar () << "\n";
  
  // The particulars of this simulation object.
  this->do_description( os );
  
  os << core::strngs::horizontal_bar() << "\n";
  for(auto const& spc : this->species_)
  {
    spc.description( os );
  }
  if ( not this->choosers_.empty() )
  {
     os << core::strngs::horizontal_bar () << "\n";
     os << "[choices] trial types and rates\n";
     static const boost::format choice_header(" %6s %7s");
     os << boost::format(choice_header) % "type" % "rate(%)" << "\n";
     for (auto const& chsr : this->choosers_ )
     {
        chsr.description( os );
     }
     if ( not this->choices_.empty() )
     {
        static const boost::format choice_header(" %6s %4s %7s");
        os << boost::format(choice_header) % "type" % "spc." % "rate(%)" << "\n";
        static const boost::format choice_row(" %6s %4s %7.2f");
        for (auto const& choice : this->choices_)
        {
           os << boost::format(choice_row) % choice.label() % this->species_[choice.key().key].label() % (choice.probability()*100.0) << "\n";
        }
     }
  }
  this->reporter_->description(os);
  if ( not this->evaluators_.empty() )
  {
     os << core::strngs::horizontal_bar () << "\n";
     for (auto const& evltr : this->evaluators_)
     {
        evltr.description(os);
     }
  }
  if (this->ensemble_)
  {
     os << core::strngs::horizontal_bar () << "\n";
     this->ensemble_->description( os );
  }

}

// Details about the current simulation object.
void simulator::do_description(std::ostream & os) const 
{
  os << "[simulation]\n- base settings\n";
  os << " beta (1/k_bT) : " << (1.0/(core::constants::boltzmann_constant()*this->temperature_kelvin_)) << " J{-1}\n";
  os << "   temperature : " << this->get_temperature() << " K\n";
  os << "  specie count : " << this->specie_count() << "\n";
  os << "- loop sizes\n";
  os << "       thermal : " << this->equilibration_interval_ << "\n";
  os << "    production : " << this->production_interval_ << "\n";
  os << "         inner : " << this->inner_loop_size_ << "\n";
  os << "        report : " << this->report_interval_ << "\n";
  os << " current index : " << this->outer_loop_index_ << "\n";
  os << "- sample loop sizes\n";
  os << "- other parameters\n";
  os << " starting particle count : " << this->target_count() << "\n"; 
  os << " ionic strngth (Molar)    : " << this->ionic_strength() << "\n"; 
  os << "- Simulation Runtime Settings #\n";
  os << " run UUID                 : " << this->run_uuid_ << "\n";
  os << " run index                : " << this->run_index_ << "\n";
  os << " output directory template: " << this->output_dir_fmt_ << "\n";
  os << " output directory         : " << this->compute_output_dir() << "\n";

}

//Write the name of the program and the license.
void simulator::license(std::ostream & os) const
{
  this->do_license( os );
  
  os << core::strngs::horizontal_bar () << "\n";
  os << "This source file is free software: you can redistribute it and/or modify\n";
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

// Generate a series of possible filenames and return the
// first filename that is found. Throw an error if no filenames
// are found. The template string is processed using 'printf'
// like function.
//
// \post exists(return val)

std::string simulator::find_input_filename() const
{
  // Filenames to generate
  //   input.%03d.inp % run_index
  //   input.inp
  std::string Result;
  std::string errmsg ("Unable to find files named:");
  
  if (not this->filename_base_.empty())
  {
    // Check for '%' to see if filename base is a format string
    if (std::string::npos != this->filename_base_.find('%'))
    {
      // TRY ONE : filename base is format string
      // -------
      std::vector< char > fn (this->filename_base_.size() + 8, '\0');
      std::size_t count = 0ul;
      do
      {
        if (count > fn.size())
        {
          fn.resize(count + 1, '\0');
        }
        count = std::snprintf(fn.data(), fn.size() - 1, this->filename_base_.c_str(), this->run_index_);
        UTILITY_ALWAYS (count >= 0, "Encoding error found making a filename from format text [" + this->filename_base_ + "]");
      }
      while (count > fn.size());
      // Remove terminating nul
      fn.resize(count);
      Result.assign(fn.begin(),fn.end());
      // Check if file exists
      if (boost::filesystem::exists(Result))
      {
        return Result;
      }
      else
      {
        // Filename not found
        errmsg.append(" ");
        errmsg.append(Result);
      }
      // TRY TWO : remove format code from filename base
      // -------
      Result.assign(this->filename_base_);
      // Remove everything from the '%' to the next '.'
      std::size_t pos1 = Result.find('%'); // We know this is not npos!
      std::size_t pos2 = Result.find('.', pos1);
      if (std::string::npos != pos2)
      {
        Result.erase (pos1, pos2 - pos1 + 1);
      }
      else
      {
        Result.erase (pos1);
      }
      // Check if file exists
      if (boost::filesystem::exists(Result))
      {
        return Result;
      }
      else
      {
        // Filename not found
        errmsg.append(" ");
        errmsg.append(Result);
      }
      }
    else
    {
      // TRY THREE : filename base is simple string
      // ---------
      Result.assign(this->filename_base_);
      // Check if file exists
      if (boost::filesystem::exists(Result))
      {
        return Result;
      }
      else
      {
        // Filename not found
        errmsg.append(" ");
        errmsg.append(Result);
      }
    }
  }
  // If we got here no files where found.
  UTILITY_INPUT(false, errmsg, core::strngs::fschnl(), nullptr);
  Result.clear(); // To stop compiler complaints
  return Result;

}

// Generate a series of possible filenames and return the
// first filename that is found. Throw an error if no filenames
// are found.  The template is processed using boost::format.
//
// \post exists(return val)

std::string simulator::find_input_filename(int d) const
{
  UTILITY_INPUT(not this->filename_base_.empty()
                 , "No input file name or format text for generating the input file name"
                 , core::strngs::simulator_label(), nullptr);
  // Example filenames to generate
  //   input.%03d.inp % run_index
  //   input.inp
  std::string errmsg ("Unable to find files named:");
  // Assume filename_base is a format string.
  boost::format res(this->filename_base_);
  // Ignore error of too many arguments.
  res.exceptions( boost::io::all_error_bits ^ boost::io::too_many_args_bit  );
  res % this->run_index_;
  std::string Result(res.str());
  // Check if file exists
  if (boost::filesystem::exists(Result))
  {
    return Result;
  }
  else
  {
    // Filename not found
    errmsg.append(" ");
    errmsg.append(Result);
  }
  // TRY TWO : remove format code from filename base
  // -------
  Result.assign(this->filename_base_);
  // Remove everything from the first '%' to the next '.'
  std::size_t pos1 = Result.find('%');
  while (std::string::npos != pos1)
  {
    std::size_t pos2 = Result.find('.', pos1);
    if (std::string::npos != pos2)
    {
      Result.erase (pos1, pos2 - pos1 + 1);
    }
    else
    {
      Result.erase (pos1);
    }
    // Check if file exists
    if (boost::filesystem::exists(Result))
    {
      return Result;
    }
    else
    {
      // Filename not found
      errmsg.append(" ");
      errmsg.append(Result);
    }
    pos1 = Result.find('%');
  }
  // If we got here no files where found.
  UTILITY_INPUT(false, errmsg, core::strngs::simulator_label(), nullptr);
  // To stop compiler complaints return an empty string
  Result.clear();
  return Result;

}

//Generate an initial ensemble to run a simulation.
void simulator::generate_simulation(std::ostream & oslog)
{
  // Call derived method to prepare itself and create a gridder
  // object for generating the ensemble.
  std::unique_ptr< trial::grid_generator > gridder = this->do_generate_simulation( oslog );
  
  // If the system is not Grand Canonical then we must have the
  // right number and ratio of particles in the simulation from
  // the beginning. In this version we simply test for this
  // condition and raise an error if it fails.
  const bool is_grand_canonical( true ); // TODO
  
  // Number of particles to add
  const std::size_t npart { this->target_count() };
  
  // Calculated target ionic strength.
  const double sumconc { this->ionic_strength() };
  
  // Local reference to the ensemble.
  particle::ensemble &ens( *this->ensemble_ );
  
  // Number of non-solute particles
  std::size_t nonsolute_count { 0 };
  
  // First add any predifined particles to the ensemble
  for (std::size_t ispec { 0 }; ispec != this->species_.size(); ++ispec)
  {
     auto const& spc = this->species_[ ispec ];
     const double charge { spc.valency() };
     if ( spc.count() != 0 )
     {
        for (std::size_t idx { 0 }; idx != spc.count(); ++idx)
        {
           ens.append_position( ispec, charge, spc.get_position( idx ), this->permittivity_ );
        }
        if ( not spc.is_solute() )
        {
           nonsolute_count += spc.count();
        }
     }
  }
  
  // Print predefined particles
  if (ens.size() != 0)
  {
     oslog << core::strngs::horizontal_bar() << "\n";
     oslog << " Predefined particles\n";
     oslog << core::strngs::horizontal_bar() << "\n";
     boost::format line( " %|3|  %|3|   %|8|  %|8|  %|8| " );
     oslog << boost::format(line) % "IDX" % "KEY" % "X" % "Y" % "Z" << "\n";
     for ( std::size_t idx = 0; idx != ens.size(); ++idx )
     {
        oslog << boost::format(line) % (idx + 1) % ens.key( idx )
              % ens.x( idx ) % ens.y( idx ) % ens.z( idx ) << "\n";
  
     }
     oslog << core::strngs::horizontal_bar() << "\n";
  
     // Check for overlap in predefined particles.
     bool is_overlap_in_predefined_particles { false };
     for (std::size_t idx = 0; idx != ens.size() - 1; ++idx)
     {
        std::vector< double > rij;
        this->compute_distances( ens.position( idx ), ens.get_coordinates(), rij, ens.size(), idx + 1 );
        const double iradius { this->get_specie( ens.key( idx ) ).radius() };
        for (std::size_t jdx = idx + 1; jdx != rij.size(); ++jdx)
        {
           // overlap in initial system.
           const double min_distance { iradius + this->get_specie( ens.key( jdx ) ).radius() };
           if( min_distance > rij[ jdx ] )
           {
              is_overlap_in_predefined_particles = true;
              oslog << "OVERLAP: Distance |" << (idx + 1) << ", " << (jdx + 1) << "| (= "
                    << rij[ jdx ] << " ) needs to be greater than " << min_distance << ".\n";
           }
        }
     }
     UTILITY_INPUT( not is_overlap_in_predefined_particles, "Overlap between particles in predifined particle set.", core::strngs::fsspec(), nullptr );
  }
  
  // Randomised list of keys
  std::vector< std::size_t > keys;
  
  if ( ens.size() != 0 )
  {
     // Have predefined particles.
     // ==========================
  
     // Expand ensemble if necessary
     if ( ens.size() < nonsolute_count + npart )
     {
        ens.resize( nonsolute_count + npart );
        keys.resize( nonsolute_count + npart, particle::specie_key::nkey );
     }
  
     // The beginning index of non-predefined particles
     const std::size_t base_position { ens.size() };
  
     // How many particles we need to add after removing predefined particles
     std::size_t adjusted_npart( 0ul );
  
     // Extra check for non-GC sims.
     bool incorrect_particle_count_for_non_grand_canonical( false );
  
     for (std::size_t ispec { 0 }; ispec != this->specie_count(); ++ispec)
     {
        std::size_t nspec { std::size_t( npart * this->get_specie( ispec ).concentration() / sumconc ) };
        if ( adjusted_npart + nspec > npart )
        {
           nspec = npart - adjusted_npart;
        }
        // If specie has ANY predefined particles then it will not contribute
        // to randomly generated data set.
        if ( this->get_specie( ispec ).count() != 0 )
        {
           if( not is_grand_canonical and nspec != this->get_specie( ispec ).count())
           {
              incorrect_particle_count_for_non_grand_canonical = true;
              oslog << "Non-grand canonical simulation requires " << nspec << " or zero predefined particles for specie \"" << this->get_specie( ispec ).label() << "\"\n";
           }
        }
        else
        {
           // Fill key array
           std::fill( keys.begin() + base_position + adjusted_npart
                      , keys.begin() + base_position + adjusted_npart + nspec
                      , ispec);
           adjusted_npart += nspec;
        }
     }
     UTILITY_INPUT( not incorrect_particle_count_for_non_grand_canonical, "Non-grand canonical simulations require an exact number of particles for each specie.", core::strngs::fsspec(), nullptr );
  
     // If no particles to be added then we are done
     if ( adjusted_npart == 0 )
     {
        return;
     }
  
     // Randomly order the specie keys
     this->get_random().shuffle( keys.begin() + base_position, keys.begin() + base_position + adjusted_npart );
  
     // Count of particle to add
     std::size_t idx { 0ul };
  
     // Inter-particle distances
     std::vector< double > rij;
  
     std::vector< std::size_t > added( this->specie_count() );
     std::fill( added.begin(), added.end(), 0 );
  
     UTILITY_REQUIRE( gridder, "Could not allocate position generator when needed." );
     UTILITY_REQUIRE( ens.size() != 0 or gridder->size() > npart, "Gridder object does not have enough grid points for the requested number of particles." );
  
     // ii effectively only 'index' in the gridder.
     for (size_t ii = 0; idx != adjusted_npart and ii != npart; ++ii)
     {
        const std::size_t ispec { keys[ base_position + idx ] };
        geometry::coordinate pos;
        UTILITY_CHECK( gridder->next(pos), "Grid exhausted before expected");
  
        // Check for overlap
        //
        this->compute_distances( pos, ens.get_coordinates(), rij, ens.size(), 0 );
        const double iradius { this->get_specie( ispec ).radius() };
        bool overlap { false };
        // Just iterate to the last predefined particle.
        for (std::size_t jdx = 0; jdx != base_position; ++jdx)
        {
           // Skip grid position if overlap.
           if ( iradius + this->get_specie( ens.key( jdx ) ).radius() > rij[ jdx ] )
           {
              overlap = true;
              break;
           }
        }
        // if overlap skip this grid point
        if ( overlap ) continue;
  
        ens.append_position( ispec, this->get_specie( ispec ).valency(), pos, this->permittivity_ );
        ++added[ispec];
        ++idx;
     }
     // Check that we could add most of the wanted number of particles to within 90% of target
     UTILITY_INPUT( double(idx)/double(adjusted_npart) > 0.9, "Added less than 90\% of target particles to initial system due to too many overlaps with predefined particles.", core::strngs::fsspec(), nullptr );
  
     // If not grand canonical then we must have added exactly the right number
     UTILITY_INPUT( is_grand_canonical or idx == adjusted_npart, "Unable to add required number of particles in a non grand canonical simulation due to too many overlaps with predefined particles.", core::strngs::fsspec(), nullptr );
  
     // set specie counts
     for (size_t ispec = 0; ispec != this->specie_count(); ++ispec)
     {
        if ( added[ispec] != 0 )
        {
           this->get_specie( ispec ).set_count( added[ ispec ] + this->get_specie( ispec ).count() );
        }
     }
  }
  else
  {
     // No predefined particles means we do not worry about overlap here!
     ens.resize( npart );
     keys.resize( npart, particle::specie_key::nkey );
  
     // Current particle index
     std::size_t cursor { 0 };
  
     // Build specie key list
     for (std::size_t ispec = 0; ispec != this->specie_count(); ++ispec)
     {
        std::size_t nspec { std::size_t( npart * this->get_specie( ispec ).concentration() / sumconc ) };
        if ( cursor + nspec > npart )
        {
           nspec = npart - cursor;
        }
        // Fill key array
        std::fill( keys.begin() + cursor, keys.begin() + cursor + nspec, ispec);
        cursor += nspec;
        this->get_specie( ispec ).set_count( nspec );
     }
  
     // Randomly order the specie keys
     this->get_random().shuffle( keys.begin(), keys.begin() + cursor );
  
     UTILITY_REQUIRE( gridder, "Could not allocate position generator when needed." );
     UTILITY_REQUIRE( ens.size() != 0 or gridder->size() > npart, "Gridder object does not have enough grid points for the requested number of particles." );
  
  
     // Add particles to ensemble
     for (size_t ii = 0; ii != cursor; ++ii)
     {
        geometry::coordinate pos;
        UTILITY_CHECK( gridder->next(pos), "Grid exhausted before expected");
        const std::size_t ispec { keys[ ii ] };
        ens.append_position( ispec, this->get_specie( ispec ).valency(), pos, this->permittivity_ );
     }
  }
  // Check ensemble invariants
  UTILITY_ENSURE( ens.check_invariants( this->species_ ), "Ensemble state is invalid" );

}

//Generate a UUID string
std::string simulator::make_uuid_string()
{
  // Generate the UUID string
  uuid_t val;
  uuid_generate(val);
  char p[3];
  std::string uuid_string(32, ' ');
  for (int i = 0; i != 16; ++i)
  {
    std::sprintf(&p[0], "%02X", val[i]);
    uuid_string[2*i] = p[0];
    uuid_string[2*i + 1] = p[1];
  }
  return uuid_string;

}

// Generate a random value to seed the random number generator.
//
// This uses an operating system provided random source to generate
// a single random unsigned int suitable to seed the psuedo-random
// number generator. 

uint32_t simulator::random_seed_value()
{
  // Get random seed value from system's random device:
  // requires #include <boost/random/random_device.hpp>
  boost::random::random_device seedgenr;
  return seedgenr() - seedgenr.min();

}

//Read in the given input file
void simulator::read_input(core::input_base_reader & reader, std::ostream & oslog)
{
  // Random seed value
  core::input_delegater decoder( this->get_max_input_version() );
  
  this->build_reader( decoder );
  
  decoder.read_input( *this, reader );
  oslog << core::strngs::horizontal_bar() << "\n";

}

// Reset the random number generate with the given seed. It
// should generally only be done once.
void simulator::set_random_seed(uint32_t seedval) 
{
  this->rgen_->seed (seedval);

}

//Compute the distances between given position and existing positions.
//
//\pre rij.size <= ens.size
//\post rij[0:startindex] === 0
//\post rij[ens.size:] undefined
void simulator::compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, simulator::array_type & rij, std::size_t endindex, std::size_t startindex) const
{
  endindex = std::min( endindex, coords.size() );
  switch( endindex )
  {
  case 0ul:
     rij.clear();
     return;
  case 1ul:
     rij.resize( 1 );
     rij[ 0 ] = 0.0;
     return;
  default:
     if ( rij.size() != endindex )
     {
        rij.resize ( endindex );
     }
     std::fill( rij.begin(), rij.end(), 0.0 );
     this->do_compute_distances( position, coords, rij, endindex, startindex );
  }

}

//Perform the simulation
void simulator::run(std::ostream & oslog) 
{
  UTILITY_REQUIRE( not this->species_.empty(), "Can not run simulation with no species" );
  UTILITY_REQUIRE( this->ensemble_, "Can not run simulation with particles" );
  UTILITY_REQUIRE( not this->choosers_.empty(), "Can not run simulation with no trials" );
  UTILITY_REQUIRE( not this->evaluators_.empty(), "Can not run simulation with no energy evaluators" );
  UTILITY_REQUIRE(not this->reporter_->empty(), "Can not run simulation with no observables" );
  
  this->do_run( oslog );

}

// Derived class implementation of run method.
void simulator::do_run(std::ostream & oslog)
{
  // do thermalization
  this->run_loop(this->equilibration_interval_, oslog);
  oslog << "\nEND OF THERMALISATION\n\n";
  // do main simulation
  this->run_loop(this->production_interval_, oslog);
  oslog << "\nEND OF PRODUCTION\n\n";

}

// Method for setting up and running a set of MC trials.
//
// The 'do_run' method's jobs is to determine the sequence of
// trials that should be performed. This method's job is to perform
// the trials. It is not anticipated that this method would need to
// be altered.
void simulator::run_loop(std::size_t loopcount, std::ostream & out) 
{
  // Get reference to FP environment
  utility::fp_env &env(utility::fp_env::env_);
  // Check for FP error before reporting
  if (not env.no_except())
  {
    if (0 != ((~env.Inexact) & env.except()))
    {
      UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except()),  " Floating point exception : "+ env.error_message ());
    }
    env.reset();
  }
  // Send signal to prepare other objects
  this->outer_loop_index_ = 0;
  // Build choices
  this->choices_.clear();
  for (auto &chsr : this->choosers_)
  {
    chsr.generate_choices( *this, this->choices_ );
  }
  UTILITY_REQUIRE(not this->choices_.empty (), "Can not run simulation with no trials");
  // Ensure sum of probabilities is 1.0
  double sum_choice_rates { 0.0 };
  // reset choice objects
  for (auto &choice : this->choices_)
  {
    sum_choice_rates += choice.probability();
  }
  if (not utility::feq(sum_choice_rates, 1.0))
  {
    for (auto &choice : this->choices_)
    {
      choice.set_probability( choice.probability() / sum_choice_rates );
    }
  }
  // Randomize choice order
  this->ranf_.shuffle(this->choices_);
  // reset observables
  this->reporter_->prepare( *this );
  // reset the evaluators
  for (auto &evltr : this->evaluators_)
  {
    evltr.prepare ( *this );
  }
  // Raise the prepare signal.
  this->prepare_sig_( *this );
  if ( not env.no_except() )
  {
    if (0 != ((~env.Inexact) & env.except()))
    {
      out << "Floating point exception during simulation preparation \"" << env.error_message () << "\"\n";
      UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                      , " Floating point exception : "+ env.error_message ());
    }
    env.reset();
  }
  // Calculate/reset the system's total energy
  this->total_energy();
  if ( not env.no_except() )
  {
    if (0 != ((~env.Inexact) & env.except()))
    {
      out << "Floating point exception during calculation of initial energy\n \"" << env.error_message () << "\"\n";
      UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                      , " Floating point exception : "+ env.error_message ());
    }
    env.reset();
  }
  // Set report interval to some meaningful default if not set
  if (this->report_interval_ == 0)
  {
    this->report_interval_ = std::max(1ul, loopcount / 5);
  }
  
  
  // Run the simulation
  for (size_t d9m8h2 = 0; d9m8h2 != loopcount; ++d9m8h2)
  {
    this->outer_loop_index_ += 1;
    for ( size_t t1b6b8 = 0; t1b6b8 != this->inner_loop_size_; ++t1b6b8 )
    {
      // select a move
      double sofar = this->ranf_.uniform( 0.0, 1.0 );
      std::unique_ptr< trial::change_set > trial;
      for ( auto & choice: this->choices_ )
      {
        sofar -= choice.probability();
        if (sofar <= 0.0)
        {
          trial = choice.generate( *this );
          break;
        }
      }
      if ( not trial->fail() )
      {
        // Calculate distance vectors.
        for ( auto & atom : *trial )
        {
          if (atom.do_old)
          {
            this->compute_distances( atom.old_position, this->ensemble_->get_coordinates(), atom.old_rij, this->ensemble_->size(), 0 );
          }
          if (atom.do_new)
          {
            this->compute_distances( atom.new_position, this->ensemble_->get_coordinates(), atom.new_rij, this->ensemble_->size(), 0 );
          }
        }
        // Calculate energy
        for ( auto const& evaluator : this->evaluators_ )
        {
          evaluator.compute_potential( this->species_, *this->ensemble_, *trial, 0 );
          if ( trial->fail() ) break;
        }
        if ( not env.no_except() )
        {
          if (0 != ((~env.Inexact) & env.except()))
          {
            out << "Floating point exception in computation of trial\n \"" << env.error_message () << "\"\n";
            out << "Loop [" << d9m8h2 << "][" << t1b6b8 << "]\n";
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
            this->ensemble_->commit( *trial, this->species_ );
          }
        }
      }
      for ( auto & evaluator : this->evaluators_ )
      {
        evaluator.on_conclude_trial( *trial );
      }
      this->reporter_->on_trial_end( *trial );
      if (not env.no_except())
      {
        if (0 != ((~env.Inexact) & env.except()))
        {
          out << "Floating point exception in inner sampling\n \"" << env.error_message () << "\"\n";
          out << "Loop [" << d9m8h2 << "][END]\n";
          UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                          , " Floating point exception : "+ env.error_message ());
        }
        env.reset();
      }
    }
    // Note this->outer_loop_index_ is a 1-based count
    out << " STEP : " << this->outer_loop_index_ << "\n";
    this->reporter_->on_sample( *this );
    if ((this->outer_loop_index_ % this->report_interval_) == 0)
    {
      this->do_report( out );
    }
    if (not env.no_except())
    {
      if (0 != ((~env.Inexact) & env.except()))
      {
        out << "Floating point exception in outer sampling\n \"" << env.error_message () << "\"\n";
        out << "Loop [" << d9m8h2 << "][END]\n";
        UTILITY_ALWAYS (0 == ((~env.Inexact) & env.except())
                        , " Floating point exception : "+ env.error_message ());
      }
      env.reset();
    }
    if ((this->outer_loop_index_ % this->report_interval_) == 0)
    {
      if (this->checkpoint_function_)
      {
        this->checkpoint_function_( this );
      }
    }
  }
  

}

// Report simulator statistics and issue the 'report' signal.
//
// * monitor that running energy and computed energy remain close
void simulator::do_report(std::ostream & os) 
{
  // CALLED AT EACH CHECKPOINT
  os << "SIMULATION STAGE: " << this->outer_loop_index_ << "\n";
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
  
  this->reporter_->on_report( *this, os );
  os << core::strngs::horizontal_bar() << "\n";

}

// Implementation of computing a directory from a given format text and
// creating the directory if necessary.
std::string simulator::compute_dir(std::string fmt) const 
{
  boost::format res(fmt);
  // Ignore error of too many arguments.
  res.exceptions( boost::io::all_error_bits ^ boost::io::too_many_args_bit  );
  res % this->run_index_ % this->run_uuid_;
  return res.str();

}

// Access the list of evaluators
//
// (non python)
const_range_t simulator::get_evaluators() const 
{
  return const_range_t (this->evaluators_.begin (), this->evaluators_.end ());
}

// Get the key of a specie from a label. 
//
// \pre has_specie(label)
// (C++ throws out_of_range if not found)
// (Python raises KeyError if not found)

std::size_t simulator::get_specie_key(std::string label) const 
{
  for (std::size_t ix = 0; ix != this->species_.size(); ++ix)
  {
    if (label == species_[ix].label ()) return ix;
  }
  throw std::out_of_range(label+" is not a specie label");
  return this->species_.size();

}

// Do we have a specie with this label?
bool simulator::has_specie(std::string name) const 
{
  for (auto const& spc: this->species_)
  {
    if (name == spc.label ()) return true;
  }
  return false;

}

//Calculate the current ionic strength (N/volume)
double simulator::ionic_strength() const 
{
  double result = 0.0; // ionic strength
  for (auto const& spc: this->species_)
  {
     result += spc.concentration();
  }
  return result;

}

// Calculate the permittivity at the given location.
//
// Default implementation returns value of get_permittivity
double simulator::permittivity_at(const geometry::coordinate &) const
{
  return this->permittivity_;
}

// Calculate the total energy
void simulator::total_energy() 
{
  UTILITY_REQUIRE(not this->evaluators_.empty (), "Can not calculate energy without evaluators.");
  UTILITY_REQUIRE(this->ensemble_, "Can not calculate energy without ensemble.");
  this->energy_ = 0.0;
  for ( auto & potl : this->evaluators_ )
  {
     potl.prepare( *this );
     this->energy_ += potl.compute_total_potential( *this );
  }

}

// Write contents of simulation as an input document
void simulator::write_document(core::input_document & wr) 
{
  // Add simulator section
  std::size_t ix = wr.add_section( core::strngs::simulator_label() );
  auto &sec = wr[ ix ];
  sec.add_entry( core::strngs::fsntrg(), this->target_count() );
  sec.add_entry( core::strngs::fstsi(), this->get_temperature() );
  sec.add_entry( core::strngs::fsnstp(), this->production_interval() );
  sec.add_entry( core::strngs::fsnavr(), this->equilibration_interval() );
  sec.add_entry( core::strngs::fsisav(), this->report_interval() );
  sec.add_entry( core::strngs::inner_label(), this->inner_loop_size() );
  sec.add_entry( core::strngs::fsname(), " \"" + this->run_title() + "\"" );
  sec.add_entry( core::strngs::outputdir_label(), " \"" + this->output_dir_fmt() + "\"" );
  sec.add_entry( core::strngs::inputpattern_label(), " \"" + this->filename_base() + "\"" );
  this->do_write_document( wr, ix );
  
  // Add specie definitions.
  {
     // Update specie position information.
     std::vector< std::size_t > cursors( this->specie_count() );
     for(std::size_t idx = 0; idx != this->ensemble_->size(); ++ idx)
     {
        const std::size_t ispec { this->ensemble_->key( idx ) };
        if ( ispec != particle::specie_key::nkey )
        {
           this->species_[ ispec ].update_position( cursors[ ispec ], this->ensemble_->position( idx ) );
           ++cursors[ ispec ];
        }
     }
     // Write updated specie definitions.
     for(std::size_t ispec = 0; ispec != this->specie_count(); ++ ispec)
     {
        this->species_[ ispec ].write_document( wr );
     }
  }
  
  // Add evaluator definitions
  for(auto const& evltr : this->evaluators_)
  {
     evltr.write_document( wr );
  }
  
  // Add observables
  this->reporter_->write_document( wr );
  
  // TODO Write choice rates
  for (auto const& choice : this->choosers_)
  {
     choice.write_document( wr );
  }

}


} // namespace platform

BOOST_CLASS_EXPORT_IMPLEMENT(platform::simulator);

