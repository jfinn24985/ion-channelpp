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

#include "observable/metropolis_sampler.hpp"
#include "observable/output_file.hpp"
#include "observable/sampler_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "particle/change_set.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "observable/base_sink.hpp"
#include "observable/report_manager.hpp"
#include "core/input_document.hpp"

// Manuals
#include "core/strngs.hpp"
#include "observable/field_format.hpp"
#include "observable/output_series_datum.hpp"
#include "observable/output_series.hpp"
// - 
namespace observable {

// For serialization and factory
metropolis_sampler::metropolis_sampler()
: tracked_observable()
, boltzmann_factor_()
, energy_change_()
, energy_( 0.0, 30.0, 0.01, true )
{}

metropolis_sampler::~metropolis_sampler() = default;
// Add definition for generating objects of this 
// class to the meta object.
void metropolis_sampler::add_definition(sampler_meta & meta)
{
  std::string desc( "Monitor the Metropolis factor generated for each trial. Failed trials have a factor of 0. This is used to monitor the simulation and is not an outcome of the simulation." );
  std::unique_ptr< tracked_definition > result( new tracked_definition( type_label_(), desc, &metropolis_sampler::make_sampler ) );
  // no extra parameters
  meta.add_tracked_type( result );
  

}

//Log message descibing the observable and its parameters
void metropolis_sampler::description(std::ostream & out) const 
{
  out << " " << this->type_label_() << "\n";
  out << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  out << "    Sample the metropolis factor of each trial\n";

}

std::string metropolis_sampler::get_label() const 
{
  return metropolis_sampler::type_label_();
}

//Retrieve the current estimate of the boltzmann factor and
//energy change.
//
///return Type pair< estimater, estimater > with
//  result.first -> boltzmann factor and 
//  result.second -> energy change
boost::any metropolis_sampler::get_value() const
{
  boost::any result = std::make_pair( this->boltzmann_factor_, this->energy_change_ );
  return result;

}

boost::shared_ptr< tracked_observable > metropolis_sampler::make_sampler(const std::vector< core::input_parameter_memo >& param_set)
{
  UTILITY_REQUIRE( param_set.size() == 1, "This sampler requires no parameters." );
  boost::shared_ptr< tracked_observable > smplr(new metropolis_sampler);
  return smplr;

}

//Collect information about the a_trial move
void metropolis_sampler::on_trial_end(const particle::change_set & trial) 
{
  if( not this->energy_.is_sampling() )
  {
    this->energy_.begin_sample();
  }
  if( not trial.fail() )
  {
    // Collect factor for all trials that reached Metropolis/Hastings acceptance
    // check.
    this->boltzmann_factor_.append( trial.metropolis_factor() );
    const double tmp = std::exp( -trial.energy() );
    this->energy_change_.append( tmp );
    if( not this->energy_.auto_extendable() and not this->energy_.in_range( tmp ) )
    {
      // ignore values outside range if not extendable
      return;
    }
    this->energy_.sample_datum( tmp );
  }
  else
  {
    // For all trials that failed before Metropolis/Hastings acceptance
    // check we accept the previous trial again. The failures are due to
    // things like particle-particle or cutoff distance overlap.
    this->boltzmann_factor_.append( 0.0 );
    // The difference in energy will be zero:
    this->energy_change_.append( 1.0 );
    this->energy_.sample_datum( 1.0 );
  }
  

}

//Accumulate data after a sequence of trials.
void metropolis_sampler::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  this->energy_.end_sample();

}

// Reporting operations.
//
// NOTE: The energy histogram is extendable only until first call to "on_report"
void metropolis_sampler::on_report(std::ostream & out, base_sink & sink)
{
  out << this->get_label()
      << " : P_mean       = " << this->boltzmann_factor_.mean() << " var = "
      << this->boltzmann_factor_.variance() << "\n";
  out << std::string(this->get_label().size(),' ')
      << " : -log(P_mean) = " << -std::log(this->energy_change_.mean()) << " var = "
      << std::log(this->energy_change_.variance()) << "\n";
  out << core::strngs::horizontal_bar() << "\n";
  // Write out energy histogram data
  if( this->energy_.count() > 0 )
  {
    std::string label( metropolis_sampler::type_label_() + ".dat" );
    // Lock histogram to present size
    if( this->energy_.auto_extendable() )
    {
      this->energy_.auto_extendable( false );
    }
    auto & data = this->energy_;
    if( not sink.has_dataset( label ) )
    {
      std::unique_ptr< observable::output_series >definition{ new observable::output_series( label, false ) };
      definition->set_title( "Energy distribution histogram" );
      {
        std::unique_ptr< observable::output_field > field( new observable::digitizer_output( "EMIN EMID EMAX", "e^-E e^-E e^-E", data.axis(), observable::digitizer_output::USE_ALL ) );
        definition->push_back_field( std::move( field ) );
      }
      {
        std::unique_ptr< observable::output_field > field( new observable::mean_variance_output( "P.mean P.var", "Rate Rate2" ) );
        definition->push_back_field( std::move( field ) );
      }
      sink.add_dataset( std::move( definition ) );
    }
    {
      std::unique_ptr< observable::output_series_datum > datum( new observable::output_series_datum{ 0ul, new utility::estimate_array( std::move( data.release_data() ) ) } );
      sink.receive_data( label, std::move( datum ) );
    }
  }

}

// Prepare for a main simulation loop
void metropolis_sampler::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  this->boltzmann_factor_.reset();
  this->energy_change_.reset();
  this->energy_.reset();

}

std::string metropolis_sampler::type_label_()

{
  return "metropolis";
}

// Add type of sampler to wr[ix] document section
void metropolis_sampler::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::metropolis_sampler );

