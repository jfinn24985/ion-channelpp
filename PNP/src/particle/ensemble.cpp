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

#include "particle/ensemble.hpp"
#include "particle/change_set.hpp"

// BEGIN EXTRA INCS
#include "utility/mathutil.hpp"
#include "utility/utility.hpp"
// -
#include <algorithm>
#include <boost/format.hpp>
#include <fstream>
// END
namespace particle {

//Determine the actual index of the nth valid 
//position when deletion list is not empty
//
//This ensemble class does not rearrange 
//indices after a particle deletion. This
//results in invalid definitions within the 
//particle list that are detectable with 
//key(i) == nkey. 
//Therefore the nth index position is not 
//necessarily the nth valid particle. This 
//method returns the index of the nth valid particle
//
//\pre not deletion_list.empty
//\pre index < count
//\post result >= index and result < size
std::size_t ensemble::do_nth_index(std::size_t index) const 
{
  // Have deletions
  UTILITY_CHECK( not this->deletion_list_.empty (), "No need to call this function when deletion list is empty" );
  UTILITY_REQUIRE( index < this->count (), "Particle number out of range" );
  UTILITY_ENSURE_OLD( const std::size_t nth( index ) );
  std::size_t result = index;
  // Increment result for every value in the
  // deletion list that is less or equal than it,
  // going reverse direction through list.
  for (auto ith = this->deletion_list_.rbegin(); ith != this->deletion_list_.rend(); ++ith)
  {
    if (*ith <= result) ++result;
  }
  UTILITY_ENSURE( result < size (), "Calculated index is out of range" );
  UTILITY_ENSURE( this->key_[result] != particle::specie_key::nkey, "Calculated index is not a valid index" );
  UTILITY_CHECK( result == nth + std::count( this->key_.begin(), this->key_.begin () + result, particle::specie_key::nkey )
                  , "Calculated nth index is not the nth valid index" );
  return result;
  

}

// Get the global index of a particle given a per-key index
// and key
//
//\pre lindx < specie[ key ].count
//
// This method finds the nth occurence of ispec in ispcbk. This
// means that it is reasonable fast, but not as fast as a direct
// array lookup. DO NOT USE THIS METHOD TO LOOP THROUGH ALL
// PARTICLES OF A SPECIE, if that is required use something like:
//
// for ii=1,size
//   if (key(ii) == ispec) then
//     ...
//
// NOTE: in earlier code scanning through per specie was generally
// only done when gathering data for all species.  Obviously, such
// code can be transformed into a single loop over particles with
// collecting data for all species.
std::size_t ensemble::nth_specie_index(std::size_t key, std::size_t lindx) const 
{
// Use lindx+1 to convert from zero-based index to one-based count.
auto result = utility::mathutil::find_n(this->key_.begin(), this->key_.begin () + this->inuse_particles_, key, lindx+1);
UTILITY_REQUIRE (result != this->key_.begin () + this->inuse_particles_, "Fewer count of given key than requested");
return std::distance (this->key_.begin(), result);

}

// The count of particles of each specie index in ensemble
//
// \pre ispec < known_keys
std::size_t ensemble::specie_count(std::size_t ispec) const
{
  UTILITY_REQUIRE( ispec < this->key_counts_.size(), "Index out of range." );
  return this->key_counts_[ ispec ];

}

// Get the specie local index of the particle at gidx. Complementary
// method to nth_specie_index.
//
// The return value is the ordinal of this particle in the set of particles
// of this specie.
//
// \pre gidx < size
std::size_t ensemble::specie_index(std::size_t gidx) const
{
  UTILITY_REQUIRE( gidx < this->size(), "Index out of range." );
  return std::count( this->key_.begin(), this->key_.begin() + gidx, this->key_.at( gidx ) );

}

//Change, add or remove a particle defined by atom.
void ensemble::commit(change_set & atomset)
{
  for (auto & atom: atomset)
  {
    if (atom.do_old)
    {
      if (atom.do_new)
      {
        // MOVE
        this->xyz_.set_x( atom.index, atom.new_position.x );
        this->xyz_.set_y( atom.index, atom.new_position.y );
        this->xyz_.set_z( atom.index, atom.new_position.z );
        if (this->key_[atom.index] != atom.key)
        {
          // SWAP trial
          --this->key_counts_[ this->key_[atom.index] ];
          ++this->key_counts_[ atom.key ];
          this->key_[atom.index] = atom.key;
        }
      }
      else
      {
        // REMOVE
        this->key_[atom.index] = particle::specie_key::nkey;
        --this->key_counts_[ atom.key ];
        // Add index to deletion list
        if (this->inuse_particles_ - 1 == atom.index)
        {
          // is last key
          --this->inuse_particles_;
        }
        else
        {
          this->deletion_list_.push_back ( atom.index );
          if (this->deletion_list_.size() > 1)
          {
            std::sort (this->deletion_list_.begin (), this->deletion_list_.end (), std::greater< std::size_t >());
          }
        }
      }
    }
    else
    {
      // ADD
      if (this->deletion_list_.empty ())
      {
        atom.index = this->inuse_particles_;
        ++this->inuse_particles_;
        // Check size
        if( this->inuse_particles_ >= this->max_particles_ )
        {
          this->resize( this->inuse_particles_ ); 
        }
      }
      else
      {
        atom.index = this->deletion_list_.back ();
        this->deletion_list_.pop_back ();
      }
      this->key_[atom.index] = atom.key;
      ++this->key_counts_[ atom.key ];
      this->xyz_.set_x( atom.index, atom.new_position.x );
      this->xyz_.set_y( atom.index, atom.new_position.y );
      this->xyz_.set_z( atom.index, atom.new_position.z );
    }
  }
  

}

// Add a predefined particle to the ensemble. Unlike 'commit'
// this does not update specie counters.
void ensemble::append_position(std::size_t key, geometry::coordinate pos)
{
  UTILITY_REQUIRE( key != particle::specie_key::nkey, "Cannot append positions of invalid specie" );
  //
  std::size_t cursor { this->inuse_particles_ };
  if (this->key_.size() == cursor)
  {
     this->resize( cursor + 1 ); 
  }
  this->key_[ cursor ] = key;
  this->xyz_.set_x( cursor, pos.x );
  this->xyz_.set_y( cursor, pos.y );
  this->xyz_.set_z( cursor, pos.z );
  
  if ( key >= this->key_counts_.size() )
  {
    this->key_counts_.resize( key + 1 );
  }
  ++this->key_counts_[ key ];
  
  ++this->inuse_particles_;

}

//Write a human readable representation of the 
//ensemble to 'out'
void ensemble::description(std::ostream & out) const 
{
  out << "ensemble\n";
  out << "   particle # :" << this->count() << "\n";
  out << "         size :" << this->inuse_particles_ << "\n";
  boost::format row(" %4d %9.5f %9.5f %9.5f %2d\n");
  for (std::size_t ii = 0; ii != this->inuse_particles_; ++ii)
  {
    if (particle::specie_key::nkey != this->key_[ii])
    {
      out << boost::format(row) % ii % this->x( ii ) % this->y( ii ) % this->z( ii ) % this->key_[ii];
    }
  }

}

//Reserve space for a simulation with, on average,
//npart particles.  This resizes the internal
//arrays to 2*npart and sets max_particles to
//2*npart. If 2*npart < max_particles nothing is
//changed otherwise space may be allocated so the
//object can hold more particles

void ensemble::resize(const std::size_t & npart) 
{
this->max_particles_ = utility::mathutil::next64( npart );
this->key_.resize( this->max_particles_, particle::specie_key::nkey );
this->xyz_.resize( this->max_particles_ );

}

ensemble::ensemble(ensemble && source)
: deletion_list_( std::move( source.deletion_list_ ) )
, key_counts_( std::move( source.key_counts_ ) )
, key_( std::move( source.key_ ) )
, xyz_( std::move( source.xyz_ ) )
, inuse_particles_( std::move( source.inuse_particles_ ) )
, max_particles_( std::move( source.max_particles_ ) )
{}



ensemble::ensemble(const ensemble & source)
: deletion_list_( source.deletion_list_ )
, key_counts_( source.key_counts_ )
, key_( source.key_ )
, xyz_( source.xyz_ )
, inuse_particles_( source.inuse_particles_ )
, max_particles_( source.max_particles_ )
{}



void ensemble::swap(ensemble & source)
{
  std::swap( this->deletion_list_, source.deletion_list_ );
  std::swap( this->key_counts_, source.key_counts_ );
  std::swap( this->key_, source.key_ );
  std::swap( this->xyz_, source.xyz_ );
  std::swap( this->inuse_particles_, source.inuse_particles_ );
  std::swap( this->max_particles_, source.max_particles_ );
  

}

// Check that this ensemble matches the following invariants
//
// deletion list is sorted high to low
// for each index in the deletion list key_[idx] = nkey
// any value in key_ equal to nkey has index in deletion list
// (following are optional if sim is not nul)
// - count of species in key_ equal to value stored in each specie
// - charge is correct

bool ensemble::check_invariants() const
{
  if( not this->deletion_list_.empty() )
  {
    std::size_t topvalue = this->inuse_particles_ + 1;
    for( auto index : this->deletion_list_ )
    {
      // deletion list is sorted high to low
      UTILITY_ALWAYS( index < topvalue, "deletion_list is not correctly sorted" );
      // for each index in the deletion list key_[idx] = nkey
      UTILITY_ALWAYS( this->key_[index] == particle::specie_key::nkey, "key entry for an index in deletion_list is not 'nkey'" );
      topvalue = index;
    }
  }
  {
    std::size_t nspec = this->key_counts_.size(); // max specie index
    std::vector< std::size_t > counts(nspec, 0ul); // count per specie
    std::size_t deleted_ = 0; // count of deleted elements
    for( std::size_t idx = 0; idx != this->inuse_particles_; ++idx )
    {
      const std::size_t keyval = this->key_[idx];
      if( keyval == particle::specie_key::nkey )
      {
        ++deleted_;
      }
      else
      {
        UTILITY_ALWAYS( keyval < nspec, "Key value out of range" );
        ++counts[keyval];
      }
    }
    // any value in key_ equal to nkey has index in deletion list
    UTILITY_ALWAYS( deleted_ == this->deletion_list_.size()
                    , "number fo 'nkey' entries in key_ does not match size of deletion list" );
    // count of species in key_ equal to value stored in each specie
    for( std::size_t idx = 0; idx != nspec; ++idx )
    {
      UTILITY_ALWAYS( this->key_counts_[ idx ] == counts[ idx ]
                      , "count of a specie in key_ does not match with specie object" );
    }
  }
  return true;
  

}

//Write a dense debug representation of the 
//ensemble to 'out'
void ensemble::dump(std::ostream & out) const 
{
  out << "[ensemble]\n";
  out << "len(del) " << this->deletion_list_.size();
  out << " count " << this->count() << " size " << this->inuse_particles_ << "\n";
  for (std::size_t ii = 0; ii != this->inuse_particles_; ++ii)
  {
    out << ii << " " << this->x( ii ) << " " << this->y( ii ) << " " << this->z( ii ) << " " << this->key_[ii] << "\n";
  }
  out << "key counts:";
  for (std::size_t ii = 0; ii != this->key_counts_.size(); ++ii)
  {
  out << " [ " << ii << " ]=" << this->key_counts_[ii];
  }
  out << "\n";

}

// Compare two ensembles for equalivalence
//
// NOTE: max_particles does not need to be equal
// for equivalence, this implies that arrays are
// only checked in range [0, inuse_particles).
bool ensemble::equivalent(const ensemble & other) const
{
  auto xyz_equal = [](geometry::coordinate_set const& lhs, geometry::coordinate_set const& rhs, std::size_t count)->bool
  {
    std::size_t equal = 0;
    for(std::size_t ii = 0; ii != count; ++ii)
    {
      equal += (lhs.x(ii) == rhs.x(ii) ? 1ul : 0ul );
    }
    if (equal != count) return false;
    equal = 0;
    for(std::size_t ii = 0; ii != count; ++ii)
    {
      equal += (lhs.y(ii) == rhs.y(ii) ? 1ul : 0ul );
    }
    if (equal != count) return false;
    equal = 0;
    for(std::size_t ii = 0; ii != count; ++ii)
    {
      equal += (lhs.y(ii) == rhs.y(ii) ? 1ul : 0ul );
    }
    return equal == count;
  };
  return this->inuse_particles_ == other.inuse_particles_
   and this->deletion_list_ == other.deletion_list_
   and this->key_counts_ == other.key_counts_
   and std::equal( this->key_.begin(), this->key_.begin() + this->inuse_particles_, other.key_.begin() )
   and xyz_equal(this->xyz_, other.xyz_, this->inuse_particles_);

}


} // namespace particle
