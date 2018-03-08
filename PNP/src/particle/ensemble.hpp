#ifndef IONCH_PARTICLE_ENSEMBLE_HPP
#define IONCH_PARTICLE_ENSEMBLE_HPP

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

// Default inc
#include "utility/config.hpp"
#include "utility/archive.hpp"
// End
#include "particle/specie_key.hpp"
#include <boost/serialization/vector.hpp>
#include <cstddef>
#include "geometry/coordinate_set.hpp"
#include "geometry/coordinate.hpp"
#include <iostream>

// EXTRA INCS
// END
namespace particle { class change_set; } 

namespace particle {

// A collection of particles defined by their position and
// specie ID.
//
// PROGRAMMING NOTES: To maintain invariant the size < max_size
// "resize" is used to change the ensemble size. The "commit"
// and "append_position" methods use it to expand the ensemble
// as necessary.
//
// ENSEMBLE MAX SIZE: The maximum size of the ensemble is the
// maximum of max_size or 2*pman.target_count in GC .

class ensemble {
   private:
    //The list of deleted particles.
    std::vector<std::size_t> deletion_list_;

    // Cached count of the number of particles of each specie in the ensemble.
    std::vector<std::size_t> key_counts_;

    //The index of the specie of each particle (0 == invalid)
    std::vector<std::size_t> key_;

    // The set of x,y,z coordinates
    geometry::coordinate_set xyz_;

    //The number of in-use particle position definitions.
    //
    //Number of valid particles = inuse_particle - deletion_list.size
    std::size_t inuse_particles_;

    //The maximum number of particles.
    std::size_t max_particles_;


   public:
    //  Access the set of coordinates
    geometry::coordinate_set const& get_coordinates() const
    {
       return this->xyz_;
    }

    //Get the particle ID. If ID == spec::nkey(C++) or
    //sim.nspec() then particle is not active.
    std::size_t key(std::size_t aindex) const
    {
      return this->key_[aindex];
    }


   private:
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
    std::size_t do_nth_index(std::size_t index) const;


   public:
    //Get X coordinate of particle
    //
    //\undefined aindex >= size
    inline double x(std::size_t index) const
    {
      return this->xyz_.x( index );
    }

    //Get Y coordinate of particle
    //
    //\undefined aindex >= size
    inline double y(std::size_t index) const
    {
      return this->xyz_.y( index );
    }

    //Get Z coordinate of particle
    //
    //\undefined aindex >= size
    inline double z(std::size_t index) const
    {
      return this->xyz_.z( index );
    }

    //Number of actual particles in ensemble
    std::size_t count() const
    {
      return this->inuse_particles_ - this->deletion_list_.size ();
    }

    // The number of known specie keys
    std::size_t known_keys() const
    {
      return this->key_counts_.size();
    }


   private:
    //The current allocated number of particles in the simulation.
    std::size_t max_size() const
    {
      return this->max_particles_;
    }


   public:
    //Determine the actual index of the nth valid
    //position.
    //
    //\pre index < count()
    //
    //This conformation class does not rearrange
    //indices after a particle deletion. This results
    //in invalid definitions within the particle list
    //that are detectable with key(i) == nkey.  Therefore
    //the nth index position is not necessarily the
    //nth valid particle. This method returns the
    //index of the nth valid particle
    //
    //\pre index < count
    //
    //\post result >= index and result < size
    
    std::size_t nth_index(std::size_t index) const
    {
      return ( this->deletion_list_.empty() ?  index : do_nth_index( index ) );
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
    std::size_t nth_specie_index(std::size_t key, std::size_t lindx) const;

    //Get the coordinates of a particle at an index
    geometry::coordinate position(std::size_t idx) const
    {
      return geometry::coordinate( this->xyz_.x( idx ), this->xyz_.y( idx ), this->xyz_.z( idx ) );
    }

    //Number of particle positions in ensemble (not 
    //all positions may be active)
    std::size_t size() const
    {
      return this->inuse_particles_;
    }

    // The count of particles of each specie index in ensemble
    //
    // \pre ispec < known_keys
    std::size_t specie_count(std::size_t ispec) const;

    // Get the specie local index of the particle at gidx. Complementary
    // method to nth_specie_index.
    //
    // The return value is the ordinal of this particle in the set of particles
    // of this specie.
    //
    // \pre gidx < size
    std::size_t specie_index(std::size_t gidx) const;

    //Change, add or remove a particle defined by atom.
    void commit(change_set & atomset);

    // Add a predefined particle to the ensemble. Unlike 'commit'
    // this does not update specie counters.
    void append_position(std::size_t key, geometry::coordinate pos);

    //Write a human readable representation of the 
    //ensemble to 'out'
    void description(std::ostream & out) const;


   private:
    //Reserve space for a simulation with, on average,
    //npart particles.  This resizes the internal
    //arrays to 2*npart and sets max_particles to
    //2*npart. If 2*npart < max_particles nothing is
    //changed otherwise space may be allocated so the
    //object can hold more particles
    
    void resize(const std::size_t & npart);


   public:
    // Set the highest key value.
    //
    // \post known_keys >= sz
    void set_known_keys(std::size_t sz)
    {
      if( sz > this->key_counts_.size() ) { this->key_counts_.resize( sz ); }
    }
    //Default constructor.
    ensemble()
    : deletion_list_ ()
    , key_counts_()
    , key_()
    , xyz_ ()
    , inuse_particles_(0)
    , max_particles_(0)
    {}
    ensemble(ensemble && source);

    ensemble(const ensemble & source);

    ensemble & operator=(ensemble source)
    {
       this->swap( source );
       return *this;
    }

    void swap(ensemble & source);

    //Destructor
    ~ensemble() = default;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & deletion_list_;
        ar & key_counts_;
        ar & key_;
        ar & xyz_;
        ar & inuse_particles_;
        ar & max_particles_;
      }


   public:
    // Check that this ensemble matches the following invariants
    //
    // deletion list is sorted high to low
    // for each index in the deletion list key_[idx] = nkey
    // any value in key_ equal to nkey has index in deletion list
    // (following are optional if sim is not nul)
    // - count of species in key_ equal to value stored in each specie
    // - charge is correct
    
    bool check_invariants() const;

    //Write a dense debug representation of the 
    //ensemble to 'out'
    void dump(std::ostream & out) const;

    // Compare two ensembles for equalivalence
    //
    // NOTE: max_particles does not need to be equal
    // for equivalence, this implies that arrays are
    // only checked in range [0, inuse_particles).
    bool equivalent(const ensemble & other) const;

    friend bool operator==(const ensemble & lhs, const ensemble & rhs)
    {
      return lhs.equivalent( rhs );
    }

};

} // namespace particle
#endif
