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

#include "particle/change_set.hpp"

// Default inc
#include "utility/config.hpp"
#include "utility/utility.hpp"
// End
namespace particle {

change_atom::change_atom(bool use_old, bool use_new, std::size_t aindex, const geometry::coordinate & aoldpos, const geometry::coordinate & anewpos, std::size_t akey)
: old_rij()
, new_rij()
, old_rij2()
, new_rij2()
, do_old(use_old)
, do_new(use_new)
, energy_old(0.0)
, energy_new(0.0)
, key(akey)
, index(aindex)
, new_position(anewpos)
, old_position(aoldpos)
{}


//Default ctor
change_atom::change_atom() 
: old_rij ()
, new_rij ()
, old_rij2 ()
, new_rij2 ()
, do_old (true)
, do_new (true)
, energy_old (0.0)
, energy_new (0.0)
, key (particle::specie_key::nkey)
, index(0)
, new_position ()
, old_position ()
{}

//Construct with specie key
change_atom::change_atom(std::size_t ispec) 
: old_rij()
, new_rij()
, old_rij2()
, new_rij2()
, do_old(true)
, do_new(true)
, energy_old(0.0)
, energy_new(0.0)
, key(ispec)
, index(0)
, new_position()
, old_position()
{}

change_atom::change_atom(change_atom && source)
: old_rij( std::move( source.old_rij ) )
, new_rij( std::move( source.new_rij ) )
, old_rij2( std::move( source.old_rij2 ) )
, new_rij2( std::move( source.new_rij2 ) )
, do_old( std::move( source.do_old ) )
, do_new( std::move( source.do_new ) )
, energy_old( std::move( source.energy_old ) )
, energy_new( std::move( source.energy_new ) )
, key( std::move( source.key ) )
, index( std::move( source.index ) )
, new_position( std::move( source.new_position ) )
, old_position( std::move( source.old_position ) )
{}

bool change_atom::equivalent(const change_atom & rhs) const 
{
  return (&rhs == this) or
    (rhs.do_old == this->do_old and
      rhs.do_new == this->do_new and
      rhs.key == this->key and
      rhs.index == this->index and
      rhs.energy_old == this->energy_old and
      rhs.energy_new == this->energy_new and
      rhs.new_position == this->new_position and
      rhs.old_position == this->old_position and
      rhs.new_rij == this->new_rij and
      rhs.old_rij == this->old_rij and
      rhs.new_rij2 == this->new_rij2 and
      rhs.old_rij2 == this->old_rij2);

}

// Fill old_rij and new_rij from old_rij2 and new_rij2.
void change_atom::ensure_rij()
{
  if( not this->old_rij2.empty() )
  {
    if( this->old_rij.empty() )
    {
      this->old_rij.resize( this->old_rij2.size() );
      std::transform( this->old_rij2.begin(), this->old_rij2.end(), this->old_rij.begin(), static_cast<double (*)(double)>(std::sqrt) );
    }
  }
  if( not this->new_rij2.empty() )
  {
    if( this->new_rij.empty() )
    {
      this->new_rij.resize( this->new_rij2.size() );
      std::transform( this->new_rij2.begin(), this->new_rij2.end(), this->new_rij.begin(), static_cast<double (*)(double)>(std::sqrt) );
    }
  }

}

std::ostream& operator<<(std::ostream & os, const change_atom & rhs) 
{
  os << "[change atom]\n";
  os << "atom.do_old = " << (rhs.do_old ? "true" : "false" ) << "\n";
  os << "atom.do_new = " << (rhs.do_new ? "true" : "false" ) << "\n";
  os << "atom.energy_old = " << rhs.energy_old << "\n";
  os << "atom.energy_new = " << rhs.energy_new << "\n";
  os << "atom.key = " << rhs.key << "\n";
  os << "atom.index = " << rhs.index << "\n";
  os << "atom.old_rij.size() = " << rhs.old_rij.size() << "\n";
  os << "atom.new_rij.size() = " << rhs.new_rij.size() << "\n";
  os << "atom.old_position = " << rhs.old_position << "\n";
  os << "atom.new_position = " << rhs.new_position << "\n";
  os << "atom.old_rij = ";
  for (auto rij : rhs.old_rij) os << rij << " ";
  os << "\n";
  os << "atom.new_rij = ";
  for (auto rij : rhs.new_rij) os << rij << " ";
  os << "\n";
  return os;

}

double change_set::energy() const 
{
  double result (0.0);
  for (std::size_t i = 0; i != this->size (); ++i)
  {
    result += changes_[i].energy_new - changes_[i].energy_old;
  }
  return result;

}

//Calculate the Metropolis-Hasting probability for this change
//
///pre is_valid
// /return  (0.0, VERY_LARGE)
double change_set::metropolis_factor() const 
{
  UTILITY_REQUIRE (not this->fail_, "Can not get result from invalid change.");
  const double deltu (-(this->energy () - this->exponential_factor_));
  UTILITY_CHECK (not std::isnan(deltu), "Delta Potential is NaN");
  // avoid NaN of exponential of large or small energies
  return std::exp(std::min(250.0, std::max (-250.0,deltu))) * this->probability_factor_;

}

// Stream insert function for visualizing object, possibly
// for debugging etc.
std::ostream& operator<<(std::ostream & os, const change_set & rhs) 
{
  os << "[change set]\n";
  os << "trial.fail = " << (rhs.fail_ ? "true" : "false" ) << "\n";
  os << "trial.accept = " << (rhs.accept_ ? "true" : "false" ) << "\n";
  os << "trial.exp_factor = " << rhs.exponential_factor_ << "\n";
  os << "trial.prob_factor = " << rhs.probability_factor_ << "\n";
  os << "trial.size = " << rhs.size() << "\n";
  os << "trial.energy = " << rhs.energy() << "\n";
  for (auto const& atom : rhs.changes_)
  {
    os << atom;
  }
  return os;

}


} // namespace particle

//#include <boost/serialization/export.hpp>
//BOOST_CLASS_EXPORT_GUID(particle::change_set, "particle::change_set");