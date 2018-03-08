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

#include "evaluator/coulomb.hpp"
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

// manual includes
#include "core/input_error.hpp"
// -
namespace evaluator {

coulomb::~coulomb() = default;

// Default ctor, Should be called only by serialize and make_evaluator method
coulomb::coulomb()
: base_evaluator ()
, scalar_()
{}

//Calculate the change in Coulomb Electrostatic
//energy on the ensemble in 'sys' caused by changes in
//'changes'.
void coulomb::compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const
{
if( not changes.fail() )
{
  auto const& spcs = pman.get_species();
  auto const& ens = pman.get_ensemble();
  // For each atom
  for (auto &atom : changes)
  {
    const std::size_t idx = atom.index;
    double energy_old = 0.0;
    double energy_new = 0.0;
    // Ensure rij data exists.
    atom.ensure_rij();
    // Cache minimum inter-particle distances
    const double ri =  spcs[ atom.key ].radius();
    for (std::size_t j = 0; j != ens.size(); ++j)
    {
      // Only use active particles.
      if (j != idx and particle::specie_key::nkey != ens.key (j))
      {
        if (atom.do_new)
        {
          // NEW
          // --------------------------------
          // Check for overlap
          if ( atom.new_rij[j] < spcs[ ens.key (j) ].radius() + ri )
          {
            changes.set_fail();
            return;
          }
          // Use atom.new_rij
          energy_new += (spcs[ ens.key (j) ].valency() / atom.new_rij[j]);
        }
        if (atom.do_old)
        {
          // OLD
          // --------------------------------
          // Use atom.old_rij
          energy_old += (spcs[ ens.key (j) ].valency() / atom.old_rij[j]);
        }
      }
    }
    atom.energy_old += energy_old * spcs[ atom.key ].valency() * this->scalar_;
    atom.energy_new += energy_new * spcs[ atom.key ].valency() * this->scalar_;
  }
}

}

// Calculate the total in Coulomb Electrostatic
// energy on the ensemble.
double coulomb::compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const
{
// Calculate the pairwise coulomb energies between
// all charged particles.
double energy = 0.0;
auto const& ens = pman.get_ensemble();
std::vector< double > rij( ens.size(), 0.0 );
for (std::size_t idx = 0; idx != ens.size() - 1; ++idx)
{
   const std::size_t ispec = ens.key( idx );
   if (particle::specie_key::nkey == ispec)
   {
      continue;
   }
   const double qi = pman.get_specie( ispec ).valency();
   // calculate rij vector
   gman.calculate_distances( ens.position( idx ), ens.get_coordinates(), rij, idx + 1, ens.size() );
   
   for (std::size_t jdx = idx + 1; jdx != ens.size(); ++jdx)
   {
      const std::size_t jspec = ens.key( jdx );
      if (particle::specie_key::nkey == jspec)
      {
         continue;
      }
      energy += qi * pman.get_specie( jspec ).valency() / rij[ jdx ];
   }
}
// scale energy
energy *= this->scalar_;
return energy;

}

// Log message descibing this evaluator subclass and its parameters
void coulomb::do_description(std::ostream & os) const 
{
  os << " Compute the change in pair-wise Coulomb potential\n";
  os << " Scale factor: " << invariant_factor() << " / ( permittivity * T )\n";
  os << "             = " << this->factor() << "\n";

}

// Return the invariant part of the energy scaling factor
//
// = (electron_charge^2) / (4.pi.epsilon_0.boltzmann_constant.angstrom)
double coulomb::invariant_factor()
{
  return std::pow( core::constants::electron_charge(), 2 ) / ( 4 * core::constants::pi() * core::constants::epsilon_0() * core::constants::boltzmann_constant() * core::constants::angstrom () );
}

//Prepare the evaluator for use with the given simulator and
//stepper.
//
//Base specie type defines all parameters necessary for the
//coulomb evaluator, so no species are remove from specs.
void coulomb::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman)
{
  this->scalar_ = coulomb::invariant_factor() /( eman.permittivity() * eman.temperature() );
}

// Write evaluator specific content of input file section.
//
// only throw possible should be from os.write() operation
void coulomb::do_write_document(core::input_document & wr, std::size_t ix) const
{}

// Create and add a evaluator_definition to the meta object.
void coulomb::add_definition(evaluator_meta & meta)
{
  std::string desc( "Evaluate the pairwise (Coulomb) electrostatic potential between charged particles. No cut-off is used." );
  std::unique_ptr< evaluator_definition > defn( new evaluator_definition( coulomb::type_label_(), desc, &coulomb::make_evaluator ) );
  meta.add_definition( defn );

}

std::string coulomb::type_label_()

{
  return std::string("coulomb");

}

// Set up the evaluator using the given map of name/value pairs.
std::unique_ptr< base_evaluator > coulomb::make_evaluator(const std::vector< core::input_parameter_memo > & param_set)
{
// Check no parameters have been set
UTILITY_REQUIRE( param_set.size() == 1ul, "Parameter check should have been performed in reader.do_read_end" );
std::unique_ptr< evaluator::base_evaluator > cc( new coulomb );
return cc;


}


} // namespace evaluator

BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::coulomb );