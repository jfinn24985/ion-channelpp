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

#include "cscchannel/add_choice_cell.hpp"
#include "particle/change_set.hpp"
#include "platform/simulator.hpp"
#include "utility/random.hpp"

// manual includes
#include "particle/ensemble.hpp"
// -
namespace ion_channel {

add_choice_cell::~add_choice_cell()
{

}

//Create a particle addition/remove trial
//
//(C++ forwards to two argument method)
std::unique_ptr< particle::change_set > add_choice_cell::generate(platform::simulator & sys) 
{
  return this->generate(sys, sys.get_random());
}

//Create a particle addition/remove trial using
//an external random number source. This
//version accepts a constant simulator object.
std::unique_ptr< particle::change_set > add_choice_cell::generate(const platform::simulator & sys, utility::random_distribution & ranf)
{
std::unique_ptr< trial::change_set > resultset (new trial::change_set (*this));
// ADD
// ---
const auto& ens = sys.get_ensemble();
// Check if number of in use particles will exceed the maximum number
if (ens.count() + 1 == ens.max_size())
{
   resultset->set_fail(); // Too many particles
   return resultset;
}

trial::change_atom result {};
result.key = this->key().key;
result.do_old = false;

// give particle a random position within subregion
this->region_.generate( result.new_position, ranf );
result.eps_new = sys.permittivity_at( result.new_position );

// Use current ensemble size as 'invalid' particle index
result.index = ens.size();

const auto& spc = sys.get_specie( result.key );
// Set GC data
resultset->update_exponential_factor( spc.chemical_potential() );
resultset->update_probability_factor( this->region_.volume() / ( spc.count() + 1 ) );

// Add atom
resultset->add_atom( result );

if (not sys.is_valid_position( result.new_position, result.key ))
{
   resultset->set_fail ();
}

return resultset;


}

std::string add_choice_cell::label() const 
{
  return "-- Add";
}


} // namespace ion_channel


#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(ion_channel::add_choice_cell, "ion_channel::add_choice_cell");