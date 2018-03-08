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

#include "observable/acceptance_observable.hpp"
#include "utility/estimate_array.hpp"
#include "particle/change_hash.hpp"
#include "observable/sampler_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "observable/base_sink.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "particle/change_set.hpp"
#include "observable/report_manager.hpp"
#include "core/input_document.hpp"

// manual includes
#include "core/strngs.hpp"
#include "observable/field_format.hpp"
#include "observable/output_series_datum.hpp"
#include "observable/output_series.hpp"
// - 
#include <boost/format.hpp>
// --
namespace observable {

// Add definition for generating objects of this 
// class to the meta object.
void acceptance_observable::add_definition(sampler_meta & meta)
{
  std::string desc( "Monitor the trial acceptance rate for the different trial types. This is a metric to optimise the simulation and is not a result of the simulation." );
  std::unique_ptr< tracked_definition > result( new tracked_definition( type_label_(), desc, &acceptance_observable::make_sampler ) );
  // no extra parameters
  meta.add_tracked_type( result );
  

}

// Log message descibing the observable and its parameters
void acceptance_observable::description(std::ostream & out) const
{
  out << " " << this->type_label_() << "\n";
  out << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  out << "    Collect and report trial acceptance probabilities.\n";

}

//Retrieve the current value of the variable. In this case the
//value is of type map< change_hash, estimater_array >.
boost::any acceptance_observable::get_value() const
{
  boost::any result = this->data_;
  return result;

}

//Make a trial_acceptance_observable from input file
//
//no parameters
boost::shared_ptr< tracked_observable > acceptance_observable::make_sampler(std::vector< core::input_parameter_memo > const& param_set)
{
  UTILITY_REQUIRE( param_set.size() == 1, "This sampler requires no parameters." );
  boost::shared_ptr< tracked_observable > tmp(new acceptance_observable);
  return tmp;

}

// Save statistical data to permanent storage. 
void acceptance_observable::on_report(std::ostream & out, base_sink & sink)
{
  // data is  [change_hash] : <<success/trial>> : <<attempts>>
  // (0, 1, 1, 0) 0.66666667          0 0.30000000 0.00000000
  const std::string label( "acceptance.dat" );
  if( not this->data_.empty() )
  {
    // LOG OUTPUT
    // ----------
    {
      const static boost::format title_str( " %14s %10s %10s %10s %10s" );
      out << "Trial occurence data since last report.\n";
      out << boost::format( title_str ) % "Trial" % "Accept.av" % "Accept.var" % "Trial.av" % "Trial.var" << "\n";
      const static boost::format format_str( " %14s %10.8f %10.8f %10.8f %10.8f" );
      for( auto const& entry : this->data_ )
      {
        std::stringstream hashval;
        hashval << entry.first;
        out << boost::format( format_str ) % hashval.str() % entry.second.mean( 0 ) % entry.second.variance( 0 ) % entry.second.mean( 1 ) % entry.second.variance( 1 ) << "\n";
      }
    }
    // SAVED OUTPUT
    // ------------
    if( not this->locked_ )
    {
      // need to give a data set definition
      std::unique_ptr< observable::output_series >definition{ new observable::output_series( label, true ) };
      definition->set_title( "Trial acceptance and rate data." );
      {
        std::unique_ptr< observable::key_output > field{ new observable::key_output( "Trial", "ID", true ) };
        // Add keys.
        for( auto const& entry : this->data_ )
        {
          std::stringstream hashval;
          hashval << entry.first;
          field->push_back( hashval.str() );
        }
        definition->push_back_field( std::move( field ) );
      }
      {
        std::unique_ptr< observable::mean_variance_output > field{ new observable::mean_variance_output( "Accept.mean Accept.var", "Rate Rate2" ) };
        definition->push_back_field( std::move( field ) );
      }
      {
        std::unique_ptr< observable::mean_variance_output > field{ new observable::mean_variance_output( "Trial.mean Trial.var", "Rate Rate2" ) };
        definition->push_back_field( std::move( field ) );
      }
      if( not sink.has_dataset( label ) )
      {
        sink.add_dataset( std::move( definition ) );
      }
      // else
      //  sink.replace_dataset( std::move( definition ) );
      this->locked_ = true; // Can not add any new trials.
    }
    // CHECK ALL COUNTS ARE EQUAL
    UTILITY_ENSURE_OLD( const std::size_t check_counter = this->data_.begin()->second.count() );
    // Convert map to series.
    std::size_t rank{ 0 };
    for( auto & entry : this->data_ )
    {
      UTILITY_ENSURE( check_counter == entry.second.count(), "Serious error, estimater counters should all be equal" );
      std::unique_ptr< observable::output_series_datum > datum( new observable::output_series_datum{ rank, new utility::estimate_array{ 2 } } );
      datum->arr->swap( entry.second );
      sink.receive_data( label, std::move( datum ) );
      ++rank;
    }
  }

}

//Accumulate data after a sequence of trials.
void acceptance_observable::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  // If we have a vary rare trial it might not appear in the
  // first sample window. Therefore we may need to add an
  // estimater after the first sample window. This would
  // require incrementing the counter of new estimater by
  // calling append "count" times with zero.
  std::size_t count = 0;
  if( not this->data_.empty() )
  {
    count = this->data_.begin()->second.count();
  }
  for( auto & entry : this->dictionary_ )
  {
    // entry.first : hash
    // entry.second.first : success
    // entry.second.second : attempts
    auto iter = this->data_.find( entry.first );
    // iter->first : hash
    // iter->second.first : success chance per attempt
    // iter->second.second : attempt chance
    if( this->data_.end() == iter )
    {
      iter = this->data_.insert( std::make_pair( entry.first, utility::estimate_array{ 2ul } ) ).first;
      // for late added estimaters we need to increase their
      // counter
      double dummy[ 2 ]{ 0.0, 0.0 };
      for( std::size_t catchup = 0; catchup != count; ++catchup)
      {
         iter->second.append( &dummy[0], &dummy[2] );
      }
    }
    if( 0 == entry.second.second )
    {
      double dummy[ 2 ]{ 0.0, 0.0 };
      iter->second.append( &dummy[0], &dummy[2] );
    }
    else
    {
      double dummy[ 2 ]{ double(entry.second.first)/double(entry.second.second), double(entry.second.second)/double(this->total_) };
      iter->second.append( &dummy[0], &dummy[2] );
    }
    // reset counters
    entry.second.first = 0ul;
    entry.second.second = 0ul;
  }
  // reset total count
  this->total_ = 0ul;

}

//Update variable after every trial.
void acceptance_observable::on_trial_end(const particle::change_set & trial)
{
  auto iter = this->dictionary_.find( trial.id() );
  if( this->dictionary_.end() == iter )
  {
    UTILITY_REQUIRE( not this->locked_, "Attempt to extend acceptance sampler after defining the first report.\n\n** Increase reporting interval or trial probability. **\n" );
    iter = this->dictionary_.insert( std::pair< particle::change_hash, std::pair< std::size_t, std::size_t > >( trial.id(), { 0ul, 0ul } ) ).first;
  }
  ++this->total_;
  ++(iter->second.second);
  if (trial.accept())
  {
    ++(iter->second.first);
  }

}

// Prepare the sampler for a simulation phase. Reset
// all data.
void acceptance_observable::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  this->data_.clear();
  this->dictionary_.clear();
  this->total_ = 0ul;
  this->locked_ = false;

}

//Type label is "trial-success"
std::string acceptance_observable::type_label_()

{
  return std::string("trial-rate");
}

// Write an input file section.
//
// only throw possible should be from os.write() operation
void acceptance_observable::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ix].add_entry(core::strngs::fstype(), this->type_label_());
  

}

// Does this have an estimate of the acceptance data?
//
// This requires at least one sampling cycle to be complete
// before it will be true.
const utility::estimate_array& acceptance_observable::get_datum(const particle::change_hash & id) const
{
  auto iter = this->data_.find( id );
  UTILITY_REQUIRE( this->data_.end() != iter, "Change hash not in acceptance object data set." );
  return iter->second;

}

// Get counts of acceptance and trials in the current
// sample cycle.
//
// \pre has_sample( id )
std::pair< std::size_t, std::size_t > acceptance_observable::get_sample(const particle::change_hash & id) const
{
  auto iter = this->dictionary_.find( id );
  UTILITY_REQUIRE( this->dictionary_.end() != iter, "Change hash not in acceptance object data set." );
  return iter->second;
}

// Does this have an estimate of the acceptance data?
//
// This requires at least one sampling cycle to be complete
// before it will be true.
bool acceptance_observable::has_datum(const particle::change_hash & id) const
{
  return this->data_.end() != this->data_.find( id );
}

// Get counts of acceptance and trials in the current
// sample cycle.
//
// \pre has_sample( id )
bool acceptance_observable::has_sample(const particle::change_hash & id) const
{
  return this->dictionary_.end() != this->dictionary_.find( id );
}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::acceptance_observable );