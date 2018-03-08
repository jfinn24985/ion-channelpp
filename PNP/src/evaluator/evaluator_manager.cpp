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

#include "evaluator/evaluator_manager.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "particle/change_set.hpp"
#include "core/input_document.hpp"
#include "core/input_delegater.hpp"

// manual includes
#include "core/strngs.hpp"
#include "evaluator/coulomb.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "evaluator/hard_sphere_overlap.hpp"
#include "evaluator/lennard_jones.hpp"
#include "evaluator/localizer.hpp"
// -
namespace evaluator {

evaluator_manager::evaluator_manager()
: evaluators_()
, permittivity_( standard_aqueous_permittivity() )
, temperature_( standard_room_temperature() )
{}

evaluator_manager::~evaluator_manager()
{}

// Transfer ownership of an energy evaluators into the evaluators list

void evaluator_manager::add_evaluator(std::unique_ptr< base_evaluator >& evltr) 
{
   this->evaluators_.push_back( evltr.release() );
}

// Transfer ownership of an energy evaluators into the evaluators list

void evaluator_manager::add_evaluator(std::unique_ptr< base_evaluator >&& evltr) 
{
   this->evaluators_.push_back( evltr.release() );
}

//Compute the change in potential energy.
void evaluator_manager::compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const
{
  // Calculate energy
  for ( auto const& evaluator : this->evaluators_ )
  {
    evaluator.compute_potential( pman, gman, changes );
    if ( changes.fail() ) break;
  }

}

//Calculate the total potential energy.
double evaluator_manager::compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const
{
  double result = 0.0;
  for (auto const& evltr : this->evaluators_)
  {
    result += evltr.compute_total_potential( pman, gman );
  }
  return result;

}

// Details about the current simulation to be written to the
// log at the start of the simulation.
void evaluator_manager::description(std::ostream & os) const 
{
  os << core::strngs::horizontal_bar() << "\n";
  os << "ENERGY CALCULATIONS:\n";
  os << "--------------------\n";
  os << "           Temperature: " << this->temperature_ << " (Kelvin)\n";
  os << "         beta (1/k_bT): " << (1.0/(core::constants::boltzmann_constant()*this->temperature_)) << " J{-1}\n";
  os << " Permittivity of media: " << this->permittivity_ << " (relative to vacuum)\n";
  if( not this->evaluators_.empty() )
  {
    os << core::strngs::horizontal_bar() << "\n";
    for( auto const& evltr : this->evaluators_ )
    {
      evltr.description( os );
    }
  }
  

}

// Access the list of evaluators
const_range_t evaluator_manager::get_evaluators() const 
{
  return const_range_t( this->evaluators_.begin (), this->evaluators_.end () );
}

//  Called after the trial is complete. (default do nothing)
void evaluator_manager::on_conclude_trial(const particle::change_set & changes)
{
  for ( auto & evaluator : this->evaluators_ )
  {
    evaluator.on_conclude_trial( changes );
  }
  

}

void evaluator_manager::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman)
{
  // reset the evaluators
  for (auto &evltr : this->evaluators_)
  {
    evltr.prepare( pman, gman, *this );
  }
  

}

// Write contents of simulation as an input document
void evaluator_manager::write_document(core::input_document & wr) 
{
  // Add evaluator definitions
  for(auto const& evltr : this->evaluators_)
  {
     evltr.write_document( wr );
  }

}

// Adds factory methods and input parsers for all the types that can be instantiated from 
// the input file.
void evaluator_manager::build_input_delegater(boost::shared_ptr< evaluator_manager > eman, core::input_delegater & delegate)
{
  //////////////////
  // Evaluator types
  boost::shared_ptr< evaluator_meta > emeta { new evaluator_meta( eman ) };
  // Lennard Jones
  lennard_jones::add_definition( *emeta );
  // Coulomb electrostatics
  coulomb::add_definition( *emeta );
  // Harmonic localization
  localizer::add_definition( *emeta );
  // Hard sphere overlap
  hard_sphere_overlap::add_definition( *emeta );
  delegate.add_input_delegate( emeta );

}


} // namespace evaluator


// Should not be needed for base type
// #include <boost/serialization/export.hpp>
// BOOST_CLASS_EXPORT_GUID(evaluator::evaluator_manager, "evaluator::evaluator_manager");