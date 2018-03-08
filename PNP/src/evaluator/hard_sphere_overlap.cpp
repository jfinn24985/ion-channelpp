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

#include "evaluator/hard_sphere_overlap.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "particle/change_set.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "core/input_document.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "core/input_parameter_memo.hpp"

// manual includes
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "particle/ensemble.hpp"
// -
namespace evaluator {

// Energy is always zero, but set fail if particles overlap as
// hard spheres.
void hard_sphere_overlap::compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const
{
  if( not changes.fail() )
  {
    auto const& ens = pman.get_ensemble();
    for( auto const& atom : changes )
    {
      if( atom.do_new )
      {
        // check for overlap to existing particles.
        const double iradius = this->radii_[ atom.key ];
        for( std::size_t idx = 0; idx != atom.new_rij2.size() and not changes.fail(); ++idx )
        {
          const std::size_t jspec = ens.key( idx );
          if( jspec < this->radii_.size() )
          {
            const double min_distance{ std::pow( iradius + this->radii_[ jspec ], 2 ) };
            if( min_distance > atom.new_rij2[ idx ] )
            {
              bool do_fail = true;
              for( auto const& etom : changes )
              {
                // if we have more than one particle in motion
                // then we need to check that idx is not one of
                // the atom indices. If it then ignore overlap.
                if( idx == etom.index )
                {
                  do_fail = false;
                }
              }
              if( do_fail )
              {
                changes.set_fail();
              }
            }
          }
        }
      }
    }
    // check for overlap within trial.
    if( not changes.fail() and ( changes.size() > 1 ) )
    {
      for( std::size_t idx = 0; idx + 1 != changes.size() and not changes.fail(); ++idx )
      {
        if( changes[idx].do_new )
        {
          for( std::size_t jdx = idx + 1; jdx != changes.size() and not changes.fail(); ++jdx )
          {
            if( changes[jdx].do_new )
            {
              const double rij2 = gman.calculate_distance_squared( changes[idx].new_position, changes[jdx].new_position );
              const double minij2 = std::pow( this->radii_[ changes[idx].key ] + this->radii_[ changes[jdx].key ], 2 );
              if( minij2 > rij2 )
              {
                changes.set_fail();
              }
            }
          }
        }
      }
    }
  }

}

// Assume no-overlap in verified ensemble, so energy is always zero.
double hard_sphere_overlap::compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const
{
  return 0.0;
}

// Log message descibing the derived evaluator and its parameters
void hard_sphere_overlap::do_description(std::ostream & os) const
{
  os << " Check for overlap of hard sphere particles\n";

}

// Capture specie radii.
void hard_sphere_overlap::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman)
{
  if( this->radii_.size() != pman.specie_count() )
  {
    this->radii_.resize( pman.specie_count() );
  }
  for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
  {
    this->radii_[ ispec ] = pman.get_specie( ispec ).radius();
  }

}

std::string hard_sphere_overlap::type_label_()
{
  return std::string("hard-sphere");
}

// No derived content
void hard_sphere_overlap::do_write_document(core::input_document & wr, std::size_t ix) const
{}

// Create and add a evaluator_definition to the meta object.
void hard_sphere_overlap::add_definition(evaluator_meta & meta)
{
  std::string desc( "Checks for hard-sphere particle overlap." );
  std::unique_ptr< evaluator_definition > defn( new evaluator_definition( hard_sphere_overlap::type_label_(), desc, &hard_sphere_overlap::make_evaluator ) );
  // no extra parameter definitions.
  
  meta.add_definition( defn );

}

// Set up the evaluator using the given map of name/value pairs.
std::unique_ptr< base_evaluator > hard_sphere_overlap::make_evaluator(const std::vector< core::input_parameter_memo > & param_set)
{
// Check no parameters have been set
UTILITY_REQUIRE( param_set.size() == 1ul, "Parameter check should have been performed in reader.do_read_end" );
std::unique_ptr< evaluator::base_evaluator > cc( new hard_sphere_overlap );
return cc;

}


} // namespace evaluator

BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::hard_sphere_overlap );