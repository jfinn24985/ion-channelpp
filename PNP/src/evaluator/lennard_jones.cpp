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

#include "evaluator/lennard_jones.hpp"
#include "utility/archive.hpp"

#include "particle/ensemble.hpp"
#include "core/constants.hpp"
#include "core/strngs.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "particle/change_set.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "core/input_document.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "core/input_parameter_memo.hpp"

// Manual incls
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "particle/specie_meta.hpp"
// -
#include <boost/algorithm/string.hpp>
#include <iomanip>
// -
namespace evaluator {

lennard_jones::~lennard_jones() = default;

// Default ctor, Should be called only by serialize and make_evaluator method
lennard_jones::lennard_jones()
: base_evaluator ()
, epsilon_cache_()
, sigma_cache_()
{}

//Calculate the change in Lennard-Jones formulation of
//van de Waals energy on the ensemble by changes
void lennard_jones::compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const
{
if( not changes.fail() )
{
  auto const& ens = pman.get_ensemble();
  // For each atom
  for( auto &atom : changes )
  {
    const std::size_t idx = atom.index;
    const std::size_t cache_offset = idx * pman.specie_count();
    double energy_old = 0.0;
    double energy_new = 0.0;
    // Use cache epsilon and sigma
    for( std::size_t j = 0; j != ens.size(); ++j )
    {
      // Only use active particles.
      if( j != idx and particle::specie_key::nkey != ens.key( j ) )
      {
        const std::size_t jspec = ens.key( j );
        const double eps_ij{ this->epsilon_cache_[ cache_offset + jspec ] };
        const double sig_ij2{ std::pow( this->sigma_cache_[ cache_offset + jspec ], 2 ) };
        if( atom.do_new )
        {
          // NEW
          // Use atom.new_rij2
          const double r6 { std::pow( sig_ij2 / atom.new_rij2[ j ], 3 ) };
          const double r12 { std::pow( r6, 2 ) };
          energy_new += eps_ij * ( r12 - r6 );
        }
        if( atom.do_old )
        {
          // OLD
          // --------------------------------
          // Use atom.old_rij2
          const double r6 { std::pow( sig_ij2 / atom.old_rij2[ j ], 3 ) };
          const double r12 { std::pow( r6, 2 ) };
          energy_old += eps_ij * ( r12 - r6 );
        }
      }
    }
    atom.energy_new += energy_new;
    atom.energy_old += energy_old;
  }
}

}

// Calculate the total in Coulomb Electrostatic
// energy on the ensemble.
double lennard_jones::compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const
{
// Calculate the pairwise coulomb energies between
// all charged particles.
double energy = 0.0;
auto const& ens = pman.get_ensemble();
std::vector< double > rij2( ens.size(), 0.0 );

for( std::size_t idx = 0; idx != ens.size() - 1; ++idx )
{
  const std::size_t ispec = ens.key( idx );
  if( particle::specie_key::nkey == ispec )
  {
    continue;
  }
  const std::size_t cache_offset = ispec * pman.specie_count();
  // calculate rij2 vector
  gman.calculate_distances_sq( ens.position( idx ), ens.get_coordinates(), rij2, idx + 1, ens.size() );

  for( std::size_t jdx = idx + 1; jdx != ens.size(); ++jdx )
  {
    const std::size_t jspec = ens.key( jdx );
    if( particle::specie_key::nkey != jspec )
    {
      const double eps_ij{ this->epsilon_cache_[ cache_offset + jspec ] };
      const double sig_ij2{ std::pow( this->sigma_cache_[ cache_offset + jspec ], 2 ) };
      const double r6 = std::pow( sig_ij2 / rij2[ jdx ], 3 );
      const double r12 = std::pow( r6, 2 );
      energy += eps_ij * ( r12 - r6 );
    }
  }
}
return energy;


}

// Log message descibing this evaluator subclass and its parameters
void lennard_jones::do_description(std::ostream & os) const 
{
  os << " Compute the change in pair-wise Lennard-Jones potential\n";

}

//Prepare the evaluator for use with the given simulator and
//stepper.
//
//Base specie type defines all parameters necessary for the
//coulomb evaluator, so no species are remove from specs.
void lennard_jones::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman)
{
// It is an error in the input for a species not to have a
// matching entry in epsilon or sigma
const std::string eps_label( "epsilon" );
const std::string sig_label( "sigma" );

// calculate "half" the epsilon and sigma value
std::vector< double > eps( pman.specie_count() );
std::vector< double > sig( pman.specie_count() );
for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
{
  auto const& spc = pman.get_specie( ispec );
  std::string missing;
  if( not spc.has_parameter( eps_label ) )
  {
    missing = eps_label;
  }
  if( not spc.has_parameter( sig_label ) )
  {
    if( missing.empty() ) { missing = sig_label; }
    else { missing += " " + sig_label; }
  }
  if( not missing.empty() )
  {
    auto & end = spc.parameter( core::strngs::fsend() );
    throw core::input_error::missing_parameters_error( "Lennard-Jones parameter(s) in", core::strngs::fsspec(), missing, end.filename(), end.line_number() );
  }
  eps[ ispec ] = std::sqrt( spc.parameter( eps_label ).get_float( "Lennard-Jones parameter", core::strngs::fsspec(), true, false ) );
  sig[ ispec ] = spc.parameter( sig_label ).get_float( "Lennard-Jones parameter", core::strngs::fsspec(), true, false ) / 2;
}
// Build epsilon and sigma value cache
std::vector< double > epsilon( pman.specie_count() * pman.specie_count() );
std::vector< double > sigma( pman.specie_count() * pman.specie_count() );
for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
{
  const std::size_t offset = ispec * pman.specie_count();
  const double ep1 = eps[ ispec ];
  const double sg1 = sig[ ispec ];
  for( std::size_t jspec = 0; jspec != pman.specie_count(); ++jspec )
  {
    epsilon[ offset + jspec ] = ep1 * eps[ jspec ];
    sigma[ offset + jspec ] = sg1 + sig[ jspec ];
  }
}
std::swap( this->epsilon_cache_, epsilon );
std::swap( this->sigma_cache_, sigma );

}

// Write evaluator specific content of input file section.
//
// only throw possible should be from os.write() operation
void lennard_jones::do_write_document(core::input_document & wr, std::size_t ix) const
{

}

// Create and add a evaluator_definition to the meta object.
void lennard_jones::add_definition(evaluator_meta & meta)
{
  std::string desc( "Calculates the pairwise 6/12 Lennard-Jones potential between species, without cut-off." );
  std::unique_ptr< evaluator_definition > defn( new evaluator_definition( lennard_jones::type_label_(), desc, &lennard_jones::make_evaluator ) );
  // add parameter definitions to specie.
  if( not particle::specie_meta::has_keyword( "epsilon" ) )
  {
    particle::specie_meta::add_keyword( { "epsilon", "list(specie label,number)", "", "required", "A list of specie labels and energy minima between two ideal particles of the labelled type." } );
  }
  if( not particle::specie_meta::has_keyword( "sigma" ) )
  {
    particle::specie_meta::add_keyword( { "sigma", "list(specie label,number)", "", "required", "A list of specie labels and the distance at the energy minima between two ideal particles of the labelled type." } );
  }
  meta.add_definition( defn );

}

std::string lennard_jones::type_label_()
{
  return std::string("lennard-jones");
}

// Set up the evaluator using the given map of name/value pairs.
std::unique_ptr< base_evaluator > lennard_jones::make_evaluator(const std::vector< core::input_parameter_memo > & param_set)
{
// Check no parameters have been set
UTILITY_REQUIRE( param_set.size() == 1, "Lennard-Jones evaluator expects no parameters." );
std::unique_ptr< lennard_jones > cc( new lennard_jones );
return std::unique_ptr< base_evaluator >( cc.release() );

}


} // namespace evaluator

BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::lennard_jones );