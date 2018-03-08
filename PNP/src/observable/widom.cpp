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

#include "observable/widom.hpp"
#include "trial/std_choices.hpp"
#include "trial/choice.hpp"
#include "particle/change_set.hpp"
#include "particle/ensemble.hpp"
#include "utility/machine.hpp"
#include "core/input_reader.hpp"
#include "core/strngs.hpp"
#include "observable/sampler_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "observable/base_sink.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "observable/report_manager.hpp"
#include "core/input_document.hpp"

// Manual includes
#include "core/input_error.hpp"
#include "observable/output_row_datum.hpp"
#include "observable/output_rows.hpp"
#include "observable/widom_row.hpp"
//-
#include <fstream>
#include <boost/tokenizer.hpp>
// -
namespace observable {

// Output a space separated list of the field labels.
void widom::widom_datum::labels(std::ostream & os) const
{
  os << label_ << "_EXCESS0 " << label_ << "_POTENTIAL " << label_ << "_EXCESS " << label_ << "_COUNT";

}

// Merge the given row into this row.
void widom::widom_datum::merge(widom::widom_datum & source)
{
  UTILITY_REQUIRE( same_specie( source ), "Can not merge data from different species." ); 
  specie_count_.merge( source.specie_count_ );
  exp_potential_.merge( source.exp_potential_ );
  count_ += source.count_;

}

// Output a space separated list of the field entries.
void widom::widom_datum::row(std::ostream & os) const
{
  const double mu{ -std::log( exp_potential_.mean() ) };
  const double mu_ex{ mu - std::log( core::constants::to_SI() * specie_count_.mean() / volume_ ) };
  os  << excess_potential_ << " " << mu << " " << mu_ex << " " <<  exp_potential_.count();

}

// Output a space separated list of the field units.
void widom::widom_datum::units(std::ostream & os) const
{
  os << "ENERGY ENERGY ENERGY COUNT";

}

void widom::widom_datum::reset()
{
  if( this->specie_count_.count() > 0 )
  {
    this->specie_count_.reset();
    this->exp_potential_.reset();
  }
  // reset the per-cycle iteration
  // counter to zero.
  this->count_ = 0ul;

}

// default ctor (for serialize and factory only)
widom::widom() 
: tracked_observable()
, data_()
, key_labels_()
, ranf_( boost::shared_ptr< boost::mt19937 >( new boost::mt19937 ) )
, trials_( 0 )
, loop_count_( 0 )
, rank_( 0 )
{}

widom::~widom() = default;

// Add definition for generating objects of this 
// class to the meta object.
void widom::add_definition(sampler_meta & meta)
{
  std::string desc( "Calculate the particle free energy using the Widom method. This calculates the excess chemical potential from insertion trials. This class calculates an average global value." );
  std::unique_ptr< tracked_definition > result( new tracked_definition( type_label_(), desc, &widom::make_sampler ) );
  // two extra parameters
  result->add_definition( { core::strngs::fsiwid(), "number", ">=0", "0", "Minimum number of insertion attempts per sampling period." } );
  result->add_definition( { core::strngs::fsspec(), "list", "specie labels", "all \""+ core::strngs::fsfree()+"\" species", "List of labels of the species to sample. Each species must be of solute type ("+ core::strngs::fsfree()+"). Default is to use all solute species." } );
  meta.add_tracked_type( result );

}

//Log message descibing the observable and its parameters
//
//TODO: Develop description further
void widom::description(std::ostream & out) const 
{
  out << " " << this->get_label() << "\n";
  out << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  out << "    Sample the free energy distribution of the ensemble\n";
  out << "    - Trials per sampling loop : " << this->trials_ << "\n";
  

}

//value object is of type vector< widom_datum >
boost::any widom::get_value() const
{
  boost::any result = this->data_;
  return result;

}

// Create a new widom sampler using the given parameter set.
boost::shared_ptr< tracked_observable > widom::make_sampler(std::vector< core::input_parameter_memo > const& params)
{
  std::unique_ptr< widom > smplr( new widom );
  // Check parameters for usable values
  for( auto & param : params )
  {
    if( param.name() == core::strngs::fsiwid() )
    {
      smplr->trials_ = param.get_ordinal( "Widom insertion counts/cycle", core::strngs::sampler_label() );
    }
    else if( param.name() == core::strngs::fsspec() )
    {
      smplr->key_labels_ = param.get_text( "Specie label list", core::strngs::sampler_label() );
    }
    else
    {
      UTILITY_REQUIRE( param.name() == core::strngs::fsend(), "Parameter names should have been checked in meta.do_read_end" );
    }
  }
  boost::shared_ptr< tracked_observable > result;
  result.reset( smplr.release() );
  return result;
  

}

// Save energy estimates to permanent storage
void widom::on_report(std::ostream & out, base_sink & sink)
{
  // write report to log
  {
    out << " Results widom test particle method : \n";
    out << " " << std::string( 56+5,'-' ) << "\n";
    out << " " << std::setw( 6 ) << "SPECIE";
    out << " " << std::setw( 10 ) << "SAMPLES";
    out << " " << std::setw( 10 ) << "<U> (E)";
    out << " " << std::setw( 10 ) << "<U>VAR";
    out << " " << std::setw( 10 ) << "EXCESS";
    out << "\n";
    for( widom_datum const& datum : this->data_ )
    {
      if( datum.exp_potential_.count() > 0 )
      {
        out << " CP " << std::setw( 3 ) << datum.label_;
        out << " " << std::setw( 10 ) << datum.exp_potential_.count();
        const double mu = -std::log( datum.exp_potential_.mean() );
        out << " " << std::setw( 10 ) << mu;
        out << " " << std::setw( 10 ) << datum.exp_potential_.variance();
        const double mu_ex = mu - std::log( core::constants::to_SI() * datum.specie_count_.mean() / datum.volume_ );
        out << " " << std::setw( 10 ) << mu_ex;
        out << "\n";
      }
      else
      {
        out << "CP " << std::setw( 8 ) << datum.label_;
        out << " No results as no samples taken.\n";
      }
    }
    out << core::strngs::horizontal_bar() << "\n";
  }
  // write data to output
  {
    const std::string label{ "widom.dat" };
    if( not sink.has_dataset( label ) )
    {
      // need to give a data set definition
      std::unique_ptr< observable::output_rows >definition{ new observable::output_rows( label, true ) };
      definition->set_title( "Series of Widom method chemical potential estimates." );
      sink.add_dataset( std::move( definition ) );
    }
    std::unique_ptr< observable::widom_row > row( new observable::widom_row( this->loop_count_ ) );
    row->append( this->data_.begin(), this->data_.end() );
  
    std::unique_ptr< observable::output_row_datum > datum( new observable::output_row_datum( std::move( row ), this->rank_ ) );
    sink.receive_data( label, std::move( datum ) );
    ++this->rank_;
  }
  // reset accumulators
  for( widom_datum & datum : this->data_ )
  {
    datum.reset();
  }

}

// Perform insertion trials to reach 'trials' attempts per sample cycle.
void widom::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  ++this->loop_count_;
  utility::fp_env &env( utility::fp_env::env_ );
  // Perform insertion trials until minimum number of trials
  // has been reached.
  auto const& ens = pman.get_ensemble();
  const std::size_t nspec = pman.specie_count();
  for( widom_datum & datum : this->data_ )
  {
    datum.specie_count_.append( double( ens.specie_count( datum.key_ ) ) );
    if( datum.count_ < this->trials() )
    {
      trial::add_specie adder( datum.key_ );
      for( std::size_t imove = datum.count_; imove != this->trials(); ++imove )
      {
        std::unique_ptr< particle::change_set > move_( adder.generate( pman, gman, this->ranf_ ) );
        // Calculate distance vectors.
        for( auto & atom : *move_ )
        {
          gman.calculate_distances( atom.new_position, ens.get_coordinates(), atom.new_rij, 0, ens.size() );
          // check for overlap to existing particles.
          const double iradius = pman.get_specie( atom.key ).radius();
          for( std::size_t idx = 0; idx != atom.new_rij.size() and not move_->fail(); ++idx )
          {
            if( idx != atom.index )
            {
              const std::size_t jspec = ens.key( idx );
              if( jspec < nspec )
              {
                const double min_distance = iradius + pman.get_specie( jspec ).radius();
                if( min_distance > atom.new_rij[ idx ] )
                {
                  move_->set_fail();
                }
              }
            }
          }
        }
        // Calculate energy
        eman.compute_potential( pman, gman, *move_ );
        if( not env.no_except() )
        {
          UTILITY_ALWAYS( 0 == ( ( ~env.Inexact ) & env.except() )
                          , " Floating point exception : "+ env.error_message() );
          env.reset();
        }
        // Update data
        this->update_data( ( *move_ )[0], move_->fail() );
      }
      // reset counter.
      datum.count_ = 0ul;
    }
    else
    {
      // reset counter.
      // ** if test assures this->count_[key] >= this->trials() **
      datum.count_ = 0ul;
    }
  }
  
  
  

}

// Update variable after every trial. Sample every insertion attempt.
void widom::on_trial_end(const particle::change_set & trial)
{
  // We cannot use "failed" trials but we
  // can use all other trials.
  for( auto const& atom : trial )
  {
    if( atom.do_new and not atom.do_old )
    {
      this->update_data( atom, trial.fail() );
    }
  }

}

// Prepare Widom observable for a new simulation
//
// Reset all accumulated data. Build list of specie
// keys from input list of labels (key_labels) and
// adjust key_labels to contain only used keys. 
// Allocate storage for accumulating data once the
// number of species to observe is known. 
void widom::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  this->loop_count_ = 0ul;
  this->data_.clear();
  if( this->key_labels_.empty() )
  {
    // Use all solute species if none specified in the input file.
    for( std::size_t key = 0; key != pman.specie_count(); ++key )
    {
      auto const& spc = pman.get_specie( key );
      if( spc.is_solute() )
      {
        widom_datum datum( key );
        datum.label_ = spc.label();
        datum.excess_potential_ = spc.excess_potential();
        datum.conc_ = spc.concentration();
        datum.volume_ = gman.system_region().volume( spc.radius() );
        this->data_.push_back( datum );
      }
    }
  }
  else
  {
    // Use species specified in the input file.
    boost::tokenizer<> tok { this->key_labels_ };
    // construct a new label list with no duplicate labels
    std::string used_labels;
    for( std::string lbl : tok )
    {
      if( not pman.has_specie( lbl ) )
      {
        throw core::input_error::parameter_value_error( "Specie list", core::strngs::sampler_label(), core::strngs::fsspec(), this->key_labels_, nullptr, "Specie label \"" + lbl + "\" must match one of the defined species." ); 
      }
      std::size_t key = pman.get_specie_key( lbl );
      if( not pman.get_specie( key ).is_solute() )
      {
        throw core::input_error::parameter_value_error( "Specie list", core::strngs::sampler_label(), core::strngs::fsspec(), this->key_labels_, nullptr, "Specie \"" + lbl + "\" must be a solute type specie." ); 
      }
      auto const& spc = pman.get_specie( key );
      // Ignore duplicate indices
      if( this->data_.end() == std::find_if( this->data_.begin(), this->data_.end(), [key]( const widom_datum& dat )
    {
      return dat.key_ == key;
    } ) )
      {
        widom_datum datum( key );
        datum.label_ = spc.label();
        datum.excess_potential_ = spc.excess_potential();
        datum.conc_ = spc.concentration();
        datum.volume_ = gman.system_region().volume( spc.radius() );
        this->data_.push_back( datum );
        used_labels += lbl + " ";
      }
    }
    if( not used_labels.empty() )
    {
      used_labels.pop_back();
    }
    std::swap( this->key_labels_, used_labels );
  }

}

// Add details of widom sampler to wr[ix].
void widom::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ix].add_entry(core::strngs::fstype(), this->get_label());
  wr[ix].add_entry(core::strngs::fsiwid(), this->trials());
  if (not this->key_labels_.empty())
  {
    wr[ix].add_entry(core::strngs::fsspec(), this->key_labels_);
  }

}

//Is specie of given index target of Widom observation?
//
///nothrow
bool widom::specie_of_interest(std::size_t idx) const
{
  return this->data_.end() != std::find_if(this->data_.begin(), this->data_.end(), [idx](widom_datum const& dat){ return dat.key_ == idx; });
  

}

// Collect a sample from a trial
void widom::update_data(const particle::change_atom & atom, bool is_fail)
{
  for( widom_datum &datum : this->data_ )
  {
    if( datum.key_ == atom.key )
    {
      ++datum.count_;
      if( not is_fail )
      {
        datum.exp_potential_.append( std::exp( -atom.energy_new ) );
      }
      else
      {
        // failed test has failure probability of 1
        datum.exp_potential_.append( 1.0 );
      }
      break;
    }
  }
  

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::widom );
