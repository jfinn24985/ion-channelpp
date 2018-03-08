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

#include "cscpbc/add_specie_pbc.hpp"
#include "particle/change_set.hpp"
#include "platform/simulator.hpp"
#include "utility/random.hpp"

// manual includes
#include "particle/ensemble.hpp"
#include "cscpbc/periodic_system.hpp"
// -
namespace periodic_cube {

add_specie_pbc::~add_specie_pbc()
{

}

// Create a particle addition/remove trial
//
// (C++ forwards to two argument method)
std::unique_ptr< particle::change_set > add_specie_pbc::generate(platform::simulator & sys) 
{
  return this->generate(sys, sys.get_random());
}

// Create a particle addition/remove trial using 
// an external random number source. This
// version accepts a constant simulator object.
std::unique_ptr< particle::change_set > add_specie_pbc::generate(const platform::simulator & sys, utility::random_distribution & ranf) 
{
std::unique_ptr< trial::change_set > resultset (new trial::change_set (*this));
const double cell_length( dynamic_cast< periodic_cube::periodic_system const& >( sys ).length() );
// ADD
// ---
const auto& ens = sys.get_ensemble();
if (ens.count() + 1 == ens.max_size())
{
   resultset->set_fail(); // Too many particles
   return resultset;
}
trial::change_atom result;
result.key = this->key().key;
result.do_old = false;
// give particle a random position within boundary
result.new_position.x = ranf.uniform( 0.0, cell_length );
result.new_position.y = ranf.uniform( 0.0, cell_length );
result.new_position.z = ranf.uniform( 0.0, cell_length );
result.eps_new = sys.permittivity_at( result.new_position );
// Use sys.size as 'invalid' particle index
result.index = sys.get_ensemble().size();
const auto& spc = sys.get_specie( result.key );
resultset->update_exponential_factor( spc.chemical_potential() );
resultset->update_probability_factor( std::pow( cell_length, 3 ) / (spc.count() + 1) );
resultset->add_atom( result );
return resultset;


}

std::string add_specie_pbc::label() const 
{
  return "-- Add";
}


} // namespace periodic_cube

BOOST_CLASS_EXPORT_IMPLEMENT(periodic_cube::add_specie_pbc);