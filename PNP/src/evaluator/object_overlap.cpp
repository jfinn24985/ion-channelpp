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

#include "evaluator/object_overlap.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "particle/change_set.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "core/input_document.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "core/input_parameter_memo.hpp"

// manual includes
#include "core/strngs.hpp"
// -
namespace evaluator {

// Energy is always zero, but set fail if particles overlaps object as
// detected by the system region.
void object_overlap::compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const
{
  if( not changes.fail() )
  {
    // System region object is responsible for overlap between particles and
    // objects.
    for( auto const& atom : changes )
    {
      if( not gman.system_region().is_inside( atom.new_position, pman.get_specie( atom.key ).radius() ) )
      {
        changes.set_fail();
        return;
      }
    }
  }

}

// Assume no-overlap in verified ensemble, so energy is always zero.
double object_overlap::compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const
{
  return 0.0;
}

// Log message descibing the derived evaluator and its parameters
void object_overlap::do_description(std::ostream & os) const
{
  os << " Check for overlap of particles with objects.\n";

}

// Do nothing.
void object_overlap::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman)
{}

std::string object_overlap::type_label_()
{
  return std::string("object-overlap");
}

// No derived content
void object_overlap::do_write_document(core::input_document & wr, std::size_t ix) const
{}

// Create and add a evaluator_definition to the meta object.
void object_overlap::add_definition(evaluator_meta & meta)
{
  std::string desc( "Checks for particle-object overlap." );
  std::unique_ptr< evaluator_definition > defn( new evaluator_definition( object_overlap::type_label_(), desc, &object_overlap::make_evaluator ) );
  // no extra parameter definitions.
  
  meta.add_definition( defn );

}

// Set up the evaluator using the given map of name/value pairs.
std::unique_ptr< base_evaluator > object_overlap::make_evaluator(const std::vector< core::input_parameter_memo > & param_set)
{
// Check no parameters have been set
UTILITY_REQUIRE( param_set.size() == 1ul, "Parameter check should have been performed in reader.do_read_end" );
std::unique_ptr< evaluator::base_evaluator > cc( new object_overlap );
return cc;

}


} // namespace evaluator

BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::object_overlap );