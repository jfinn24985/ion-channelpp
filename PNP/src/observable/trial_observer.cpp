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

#include "observable/trial_observer.hpp"
#include "particle/change_set.hpp"
#include "observable/sampler_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "observable/base_sink.hpp"
#include "observable/report_manager.hpp"
#include "core/input_document.hpp"

// manuals
#include "core/strngs.hpp"
#include "observable/output_buffer_datum.hpp"
#include "observable/output_text.hpp"
namespace observable {

// Set record from a change_set object.
void trial_observer::record::set(const particle::change_set & source)
{
  this->hash_ = source.id();
  this->energy_ = source.energy();
  this->probability_factor_ = source.probability_factor();
  this->exponential_factor_ = source.exponential_factor();
  this->fail_ = source.fail();
  this->accept_ = source.accept();

}

// For serialization and factory
trial_observer::trial_observer()
: tracked_observable()
, buffer_()
, index_()
{}

trial_observer::~trial_observer() = default;
// Add definition for generating objects of this 
// class to the meta object.
void trial_observer::add_definition(sampler_meta & meta)
{
  std::string desc( "Record the outcome of every trial. This is used to monitor the simulation and is not an outcome of the simulation." );
  std::unique_ptr< tracked_definition > result( new tracked_definition( type_label_(), desc, &trial_observer::make_sampler ) );
  // no extra parameters
  meta.add_tracked_type( result );
  

}

//Log message descibing the observable and its parameters
void trial_observer::description(std::ostream & out) const 
{
  out << " " << this->type_label_() << "\n";
  out << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  out << "    Record each trial\n";

}

std::string trial_observer::get_label() const 
{
  return trial_observer::type_label_();
}

//Retrieve the current estimate of the boltzmann factor and
//energy change.
//
///return Type pair< const vector<record>*, std::size_t >
boost::any trial_observer::get_value() const
{
  boost::any result = std::make_pair( &this->buffer_, this->index_ );
  return result;

}

boost::shared_ptr< tracked_observable > trial_observer::make_sampler(const std::vector< core::input_parameter_memo >& param_set)
{
  UTILITY_REQUIRE( param_set.size() == 1, "This sampler requires no parameters." );
  boost::shared_ptr< tracked_observable > smplr(new trial_observer);
  return smplr;

}

//Collect information about the a_trial move
void trial_observer::on_trial_end(const particle::change_set & trial) 
{
  if( this->index_ == this->buffer_.size() )
  {
    this->buffer_.resize( this->buffer_.size() + 64 );
  }
  this->buffer_[ this->index_ ].set( trial );
  ++this->index_;

}

// Do nothing.
void trial_observer::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{}

// Reporting operations.
void trial_observer::on_report(std::ostream & out, base_sink & sink)
{
  if( this->index_ != 0ul )
  {
    std::string label = this->type_label_() + ".dat";
    if( not sink.has_dataset( label ) )
    {
      // need to give a data set definition
      std::unique_ptr< observable::output_text >definition{ new observable::output_text( label, true ) };
      definition->set_title( "Record of each trial" );
      definition->set_field_labels( "CHANGE_ID ENERGY PROB_FACTOR EXP_FACTOR FAIL ACCEPT" );
      definition->set_units( "(A,B,C,D) ENERGY NUMBER NUMBER BOOLEAN BOOLEAN" );
      sink.add_dataset( std::move( definition ) );
    }
  
    std::stringstream os;
    for( std::size_t idx = 0ul; idx != this->index_ ; ++idx )
    {
      os << this->buffer_[ idx ];
    }
  
    std::unique_ptr< observable::output_buffer_datum > datum( new observable::output_buffer_datum{ os.str() } );
    sink.receive_data( label, std::move( datum ) );
    // Reset index.
    this->index_ = 0ul;
  }

}

// Prepare for a main simulation loop
void trial_observer::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  this->index_ = 0ul;

}

std::string trial_observer::type_label_()

{
  return "trial-log";
}

// Add type of sampler to wr[ix] document section
void trial_observer::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::trial_observer );