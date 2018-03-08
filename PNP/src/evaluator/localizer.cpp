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

#include "evaluator/localizer.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "particle/change_set.hpp"
#include "core/input_document.hpp"
#include "evaluator/evaluator_manager.hpp"

// manual includes
#include "core/input_base_reader.hpp"
#include "core/strngs.hpp"
#include "particle/ensemble.hpp"
// -
namespace evaluator {

localizer::localizer()
: spring_factor_( localizer::default_spring_factor() )
{}

localizer::~localizer() = default;

// Create and add a evaluator_definition to the meta object.
void localizer::add_definition(evaluator_meta & meta)
{
  std::string desc( "Calculate a harmonic restraining potential for localized ion species." );
  std::unique_ptr< evaluator_definition > defn( new evaluator_definition( localizer::type_label_(), desc, &localizer::make_evaluator ) );
  // Add parameter definitions.
  defn->add_definition( { core::strngs::fskmob(), "number", ">0", "4.5", "The spring factor of the harmonic localization force. The default value is 4.5. This value when paired with R values equal to 2 x X-ray B-factor/RMSD give a force field that should produce an atom distribution similar to that of the X-ray or atomic simulation." } );
  meta.add_definition( defn );

}

// Set up the evaluator using the given map of name/value pairs
//
// Valid for all simtypes. Optional spring factor constant "mobk" parameter
std::unique_ptr< base_evaluator > localizer::make_evaluator(const std::vector< core::input_parameter_memo > & param_set)
{
// Evaluator meta should ensure only valid parameters were
// set.
bool has_kmob{ false };
double spring_factor;
if( param_set.size() != 1 )
{
  for( std::size_t idx{ 0ul }; idx != param_set.size(); ++idx )
  {
    if( core::strngs::fskmob() == param_set[ idx ].name() )
    {
      spring_factor = param_set[ idx ].get_float( "Harmonic potential factor", core::strngs::evaluator_label(), true, true );
      has_kmob = true;
    }
    else
    {
      UTILITY_REQUIRE( param_set[ idx ].name() == core::strngs::fsend(), "Parameter check should have been performed in reader.do_read_end" );
    }
  }
}
std::unique_ptr< localizer > cc( new localizer );
if( has_kmob )
{
  cc->spring_factor( spring_factor );
}
std::unique_ptr< base_evaluator > result( cc.release() );
return result;

}

std::string localizer::type_label_()

{
  return core::strngs::localizer_label();

}

// Calculate change in localization energy if one of the atoms
// in the change set has moved.
void localizer::compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const
{
  auto const& spcs = pman.get_species();
  auto const& ens = pman.get_ensemble();
  for (auto &atom: changes)
  {
     if ( particle::specie_key::nkey ==  atom.key )
     {
        continue;
     }
     // ASSUMPTION: localized atoms can only be moved, not
     // added or removed.
     if ( spcs[ atom.key ].is_localized() )
     { 
        // Get localization data after converting global index into a per-specie index
        const geometry::centroid rs( spcs[ atom.key ].get_localization_data( ens.specie_index( atom.index ) ) );
        // new penalty
        const double r_sq{ gman.calculate_distance_squared( atom.new_position, rs.position() ) };
        if (rs.r * rs.r < r_sq)
        {
           // New position outside of cutoff, fail move.
           changes.set_fail();
           return; 
        }
        atom.energy_new += this->spring_factor_ * r_sq / (rs.r * rs.r);
  
        // old penalty
        atom.energy_old += this->spring_factor_ *
              ( gman.calculate_distance_squared( atom.old_position, rs.position() ) ) / (rs.r * rs.r);
  
     }
  }

}

//Calculate the total potential energy.
double localizer::compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const
{
  double energy = 0.0;
  auto const& ens = pman.get_ensemble();
  for( std::size_t idx = 0; idx != ens.size(); ++idx )
  {
    std::size_t ispec = ens.key( idx );
    if( particle::specie_key::nkey == ispec )
    {
      continue;
    }
    // ASSUMPTION: localized atoms can only be moved, not
    // added or removed.
    if( pman.get_specie( ispec ).is_localized() )
    {
      // Get localization data after converting global index into a per-specie index
      const geometry::centroid rs( pman.get_specie( ispec ).get_localization_data( ens.specie_index( idx ) ) );
      // penalty
      const double r_sq
      {
        gman.calculate_distance_squared( ens.position( idx ), rs.position() )
      };
      energy += this->spring_factor_ * r_sq / ( rs.r * rs.r );
    }
  }
  return energy;
  

}

// Log message descibing the evaluator and its parameters
void localizer::do_description(std::ostream & os) const
{
  os << " Compute the harmonic potential to localize specific particles\n\n";
  os << "  For a harmonic oscillator the potential stored in a particle is\n\n";
  os << "     U = 1/2 k r^2\n\n";
  os << "  Where k is the spring constant and r is the distance from the mean position\n";
  os << "  of the oscillation. The potential to approximate a Uniform distribution is:\n\n";
  os << "     U  = k_bT/2 ( r^2 )/( sigma^2 )  -- eqn 1\n\n";
  os << "  We reformulate this as (now in units of \"k_bT\"):\n\n";
  os << "     U = C_mob ( r^2 )/( R_i^2 ) -- eqn 2\n\n";
  os << "  Where \"C_mob\" is a simulation wide constant (the spring factor) and R_i\n";
  os << "  is a per-particle is the cut-off distance.\n\n";
  os << "  To get eqn 2 to model a Uniform distribution our values of the spring factor\n";
  os << "  (C_mob, parameter \"mobk\") and R_i must satisfy the relations.\n\n";
  os << "     C_mob = 1/2 n^2 and R_i = n * sigma -- eqn 3\n\n";
  os << "  where n is any factor. Conversely, given kmob and R_i the distribution\n";
  os << "  models a Uniform distribution with \n\n";
  os << "     sigma^2 = R_i^2 / (2 * C_mob)\n\n";
  os << "  DEFAULT SETTINGS:\n\n";
  os << "  In a Uniform distribution, greater that 99.5\% of the distribution\n";
  os << "  is within 3 standard deviations of the mean position. We take this as\n";
  os << "  the default by defining (n) in eqn 3 as 3 giving:\n\n";
  os << "     C_mob = 4.5\n";
  os << "     R_i  = 3 * sigma\n\n";
  os << "  If we have B factors from an X-ray we can determine R_i from\n\n";
  os << "     R_i = 3 * sigma = 3 { 1/(2 pi) sqrt( B_i/2 ) }\n\n";
  os << " Spring factor: " << this->spring_factor_ << " (kT)\n";
  

}

// Write evaluator specific content of input file section.
//
// only throw possible should be from os.write() operation
void localizer::do_write_document(core::input_document & wr, std::size_t ix) const
{
wr[ ix ].add_entry( core::strngs::fskmob(), this->spring_factor_ );


}

// This is a no-op; a localizer objects does not need any
// preparation before a simulation.
void localizer::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman)
{}


} // namespace evaluator

BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::localizer );