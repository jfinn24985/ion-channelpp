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

#include "observable/report_manager.hpp"
#include "observable/base_observable.hpp"
#include "observable/base_sink.hpp"
#include "particle/change_set.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "core/input_document.hpp"
#include "core/input_delegater.hpp"

// Manual includes
#include "core/strngs.hpp"
#include "observable/acceptance_observable.hpp"
#include "observable/density_zaxis.hpp"
#include "observable/d3_distribution.hpp"
#include "observable/metropolis_sampler.hpp"
#include "observable/rdf_sampler.hpp"
#include "observable/sampler_meta.hpp"
#include "observable/specie_count.hpp"
#include "observable/specie_region_count.hpp"
#include "observable/trial_observer.hpp"
#include "observable/widom.hpp"
// -

namespace observable {

// LIFETIME METHODS

report_manager::report_manager()
: variables_()
, samples_()
, sink_()
{}


// MODIFY METHODS

//add sampled variable to list.
//
///pre not has_sample(var.label())
void report_manager::add_sample(boost::shared_ptr< base_observable > var)
{
  UTILITY_REQUIRE(this->samples_.empty() or not this->has_sample(var->get_label()), "Attempt to add two variables with the same name");
  this->samples_.push_back(var);
  

}

//add variable to list.
//
///pre not has_tracked(var.label())
void report_manager::add_tracked(boost::shared_ptr< tracked_observable > var)
{
  UTILITY_REQUIRE(this->variables_.empty() or not this->has_tracked(var->get_label()), "Attempt to add two variables with the same name");
  this->variables_.push_back(var);
  

}

//Accumulate data after every trial
void report_manager::on_trial_end(const particle::change_set & trial)
{
  for (auto & obs : this->variables_)
  {
    obs->on_trial_end(trial);
  }
  

}

//Accumulate data after a sequence of trials
void report_manager::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  for (auto & obs : this->samples_)
  {
    obs->on_sample( pman, gman, eman );
  }
  for (auto & obs : this->variables_)
  {
    obs->on_sample( pman, gman, eman );
  }

}

// Get all samplers to save statistical data to permanent storage
//
// \pre has_sink
void report_manager::on_report(std::ostream & out)
{
  UTILITY_REQUIRE( this->has_sink(), "Can not output report before setting sink." );
  for (auto & obs : this->variables_)
  {
    obs->on_report( out, *(this->sink_) );
  }
  for (auto & obs : this->samples_)
  {
    obs->on_report( out, *(this->sink_) );
  }
  this->sink_->write_dataset();

}

// Get all samplers to prepare themselves for a simulation. Check for
// connection to signals of interest and connect if necessary.

void report_manager::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) const
{
  for (auto & obs : this->variables_)
  {
    obs->prepare( pman, gman, eman, *this );
  }
  for (auto & obs : this->samples_)
  {
    obs->prepare( pman, gman, eman, *this );
  }

}

// remove sampled variable from list.
//
// \pre has_sample(var->get_label()) and get_sample(var->get_label()).get() == var.get()
// \post not has_sample(var->get_label())
void report_manager::remove_sample(boost::shared_ptr< base_observable > & var)
{
  auto itr = this->get_sample_(var->get_label());
  UTILITY_REQUIRE(this->samples_.end() != itr, "Attempt to remove sample variable that is not in the report manager.");
  UTILITY_REQUIRE(itr->get() == var.get(), "Variable in report manager with matching name does not have the same pointer.");
  this->samples_.erase(itr);

}

// remove tracked variable from list.
//
// \pre has_tracked(var->get_label()) and get_tracked(var->get_label()).get() == var.get()
//
// \post not has_sample(var->get_label())
void report_manager::remove_tracked(boost::shared_ptr< tracked_observable > & var)
{
  auto itr = this->get_tracked_(var->get_label());
  UTILITY_REQUIRE(this->variables_.end() != itr, "Attempt to remove sample variable that is not in the report manager.");
  UTILITY_REQUIRE(itr->get() == var.get(), "Variable in report manager with matching name does not have the same pointer.");
  this->variables_.erase(itr);

}

// ACCESSOR METHODS

// Get all samplers to write their descriptions.
//
// \pre has_sink
void report_manager::description(std::ostream & out) const
{
  UTILITY_REQUIRE( this->has_sink(), "Can not output description before setting sink." );
  out << core::strngs::horizontal_bar() << "\n";
  out << " Data accumulators:\n";
  out << "-------------------\n";
  this->sink_->description( out );
  for (auto & obs : this->variables_)
  {
    obs->description( out );
  }
  for (auto & obs : this->samples_)
  {
    obs->description( out );
  }
  

}

// Get reference to the data output sink
//
// \pre has_sink
base_sink & report_manager::get_sink() const
{
  UTILITY_REQUIRE( this->has_sink(), "Cannot retrieve sink object before it has been set." );
  return *(this->sink_);
}

//Attempt to retrieve a variable by label.
//
///pre has_sample
boost::shared_ptr< base_observable > report_manager::get_sample(std::string name)
{
  auto result = this->get_sample_(name);
  UTILITY_REQUIRE(this->samples_.end() != result, "Attempt to retrieve non-existent variable.");
  return *result;

}

//Attempt to retrieve a variable by label.
//
///pre has_sample
boost::shared_ptr< base_observable > report_manager::get_sample(std::string name) const
{
  auto result = this->get_sample_(name);
  UTILITY_REQUIRE(this->samples_.end() != result, "Attempt to retrieve non-existent variable.");
  return *result;

}

//Attempt to retrieve a variable by label.
//
///pre has_tracked
boost::shared_ptr< tracked_observable > report_manager::get_tracked(std::string name)
{
  auto result = this->get_tracked_(name);
  UTILITY_REQUIRE(this->variables_.end() != result, "Attempt to retrieve non-existent variable.");
  return *result;

}

//Attempt to retrieve a variable by label.
//
///pre has_tracked
boost::shared_ptr< tracked_observable > report_manager::get_tracked(std::string name) const
{
  auto result = this->get_tracked_(name);
  UTILITY_REQUIRE(this->variables_.end() != result, "Attempt to retrieve non-existent variable.");
  return *result;

}

//Attempt to retrieve a variable by label
//
///nothrow
bool report_manager::has_sample(std::string name) const
{
  return not this->samples_.empty()
    and (this->samples_.end() != this->get_sample_(name));

}

//Attempt to retrieve a variable by label
//
///nothrow
bool report_manager::has_tracked(std::string name) const
{
  return not this->variables_.empty()
    and (this->variables_.end() != this->get_tracked_(name));

}

// Get all samplers to write their input information.
void report_manager::write_document(core::input_document & wr) const 
{
  for (auto const& obsr : this->variables_)
  {
    obsr->write_document( wr );
  }
  for (auto const& obsr : this->samples_)
  {
    obsr->write_document( wr );
  }
  

}

//Retrieve iterator pointing to sampled variable
//with the given name or "end".
//
///nothrow

std::vector< boost::shared_ptr< base_observable > >::iterator report_manager::get_sample_(std::string name)
{
  return std::find_if(this->samples_.begin(),
          this->samples_.end(), [&name](decltype(*(this->samples_.begin()))& item)->bool
          {
            return item->get_label() == name;
          });

}

//Retrieve iterator pointing to sampled variable
//with the given name or "end".
//
///nothrow

std::vector< boost::shared_ptr< base_observable > >::const_iterator report_manager::get_sample_(std::string name) const
{
  return std::find_if(this->samples_.begin(),
          this->samples_.end(), [&name](decltype(*(this->samples_.begin()))& item)->bool
          {
            return item->get_label() == name;
          });

}

//Retrieve iterator pointing to tracked variable
//with the given name or "end".
//
///nothrow

std::vector< boost::shared_ptr< tracked_observable > >::iterator report_manager::get_tracked_(std::string name)
{
  return std::find_if(this->variables_.begin(),
          this->variables_.end(), [&name](decltype(*(this->variables_.begin())) item)->bool
          {
            return item->get_label() == name;
          });

}

//Retrieve iterator pointing to tracked variable
//with the given name or "end".
//
///nothrow

std::vector< boost::shared_ptr< tracked_observable > >::const_iterator report_manager::get_tracked_(std::string name) const
{
  return std::find_if(this->variables_.begin(),
          this->variables_.end(), [&name](decltype(*(this->variables_.begin())) item)->bool
          {
            return item->get_label() == name;
          });

}

// Adds factory methods and input parsers for all the types that can be instantiated from 
// the input file.
void report_manager::build_input_delegater(boost::shared_ptr< report_manager > rman, core::input_delegater & delegate)
{
  ///////////////////
  // Observable types
  boost::shared_ptr< observable::sampler_meta > ometa { new observable::sampler_meta( rman ) };
  
  // Monitor trial occurence and success
  observable::acceptance_observable::add_definition( *ometa );
  
  // Density histogram in zaxis
  observable::density_zaxis::add_definition( *ometa );
  
  // 3D density histogram
  observable::d3_distribution::add_definition( *ometa );
  
  // Monitor trial acceptance and energy.
  observable::metropolis_sampler::add_definition( *ometa );
  
  // Log trial data. This collects only the raw data.
  observable::trial_observer::add_definition( *ometa );
  
  // Radial distribution histograms
  observable::rdf_sampler::add_definition( *ometa );
  
  // Specie particle count average
  observable::specie_count::add_definition( *ometa );
  
  // Specie particle count per region average
  observable::specie_region_count::add_definition( *ometa );
  
  // Whole-cell potential energy average using Widom method
  observable::widom::add_definition( *ometa );
  
  delegate.add_input_delegate( ometa );

}


} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(observable::report_manager, "observable::report_manager");