#ifndef IONCH_PARTICLE_PARTICLE_MANAGER_HPP
#define IONCH_PARTICLE_PARTICLE_MANAGER_HPP

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

#include "utility/archive.hpp"

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include "particle/specie.hpp"
#include <cstddef>
#include <iostream>
#include <string>

namespace particle { class ensemble; } 
namespace core { class input_document; } 
namespace particle { class change_set; } 
namespace geometry { class geometry_manager; } 
namespace utility { class random_distribution; } 
namespace core { class input_delegater; } 

namespace particle {

// Manage the ensemble of particles and their type information
//
// TODO: write_document (need to update specie location info)
//     generate_simulation (copy from specie location info)
//    ** Scan simulator class for places where ensemble and specie state
//    ** is changed.
//
// MAXIMUM ENSEMBLE SIZE:
//
// * In a non-GC simulation the ensemble size will not vary
// after initialization.
//
// * In a GC simulation the ensemble size will change, but it
// should form a normal distribution around the "target_count" (N)
// with a standard deviation of sqrt(N).
//
// * The system now has no hard-limit on the ensemble
// size. If such is required then we suggest something like
// next64(2*"target_count") which gives a maximum at least 6
// standard deviations from the mean. The probability of reaching
// such a maximum is so small that actually reaching it is most
// likely to indicate an error condition.

class particle_manager
 {
   private:
    // The set of particles
    boost::shared_ptr< ensemble > ensemble_;

    // Set of species.
    std::vector< specie > species_;

    //The target number of particles
    std::size_t target_particles_;


   public:
    particle_manager();

    ~particle_manager() = default;

    particle_manager(particle_manager && source)
    : ensemble_( std::move( source.ensemble_ ) )
    , species_( std::move( source.species_ ) )
    , target_particles_( std::move( source.target_particles_ ) )
    {}
    particle_manager(const particle_manager & source)
    : ensemble_( source.ensemble_ )
    , species_( source.species_ )
    , target_particles_( source.target_particles_ )
    {}

    particle_manager & operator=(particle_manager source)
    {
      this->swap( source );
      return *this;
    }
    void swap(particle_manager & source)
    {
      std::swap( this->ensemble_, source.ensemble_ );
      std::swap( this->species_, source.species_ );
      std::swap( this->target_particles_, source.target_particles_ );
    }


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version);

   public:
    // Current charge of system
    double charge() const;

    // Details about the current ensemble of particles.
    virtual void description(std::ostream & os) const;

    // Get the current particle set
    const ensemble& get_ensemble() const
    {
      return *(this->ensemble_.get ());
    }

    // Get the definition of a specie by index
    specie const& get_specie(std::size_t key) const
    {
      return this->species_[key];
    }
    // Get the definition of a specie by index
    //
    // \pre key < specie_count : out_of_range
    specie& get_specie(std::size_t key)
    {
      return this->species_.at( key );
    }

    // Get the key of a specie from a label. 
    //
    // \pre has_specie(label)
    // (C++ throws out_of_range if not found)
    // (Python raises KeyError if not found)
    
    std::size_t get_specie_key(std::string label) const;

    // Get the complete specie set
    std::vector< specie > const& get_species() const
    {
      return this->species_;
    }

    // Do we have a specie with this label?
    bool has_specie(std::string name) const;

    // [Molar] Calculate the current ionic strength
    // (N/volume). This calculation only includes the solute
    // particles in the particle count.
    
    double ionic_strength(double volume) const;

    // The number of different species
    //
    // ( equivalent to get_species().size() (
    std::size_t specie_count() const
    {
      return this->species_.size();
    }

    // [Molar] Sum of the specie target concentrations,
    double target_ionic_strength() const;


   private:
    // [Molar] Calculate the target ionic strength (N/volume) based
    // on the target particle count set in the input.
    
    double target_ionic_strength(double volume) const;


   public:
    // How many particles we want in the simulation
    //
    // The intended average (solute) particle number in the
    // simulation.
    std::size_t target_count() const
    {
      return this->target_particles_;
    }


   private:
    // [N] Calculate the count based on the target ionic strength
    // (C/volume). 
    //
    // return : count as a real number.
    
    double target_count(double volume) const;

    // Calculated from the target particle count and target
    // ionic strength.
    //
    // \pre target_ionic_strength /= 0
    double target_volume() const;


   public:
    // Write ensemble and specie information as an input document
    void write_document(core::input_document & wr);

    // Add (copy) specie to set and return the index of the specie
    std::size_t add_specie(const specie & spc);

    // Copy particles defined in the input file into the ensemble 
    // (Should only be called when generating the initial ensemble)
    void add_predefined_particles();

    // Change, add or remove a particles defined by change set if
    // change has not failed.
    //
    // \pre not atomset.fail
    void commit(change_set & atomset);

    // Calculate the system size parameters ( concentration, volume,
    // particle count ) based on which of these have been provided
    // in the input file.  If all are provided then priority is
    // concentration > particle count > volume.
    //
    // Returns concentration, volume and count as well as boolean
    // values noting if the values where altered.  Most valid inputs
    // will have none of the parameters set to zero and one or more
    // of the booleans should be true.
    //
    // It is usually an error if no concentration and no solute
    // particles are (pre)defined.  This leads to both the
    // concentration and count parameters being at zero.  It is not
    // an error if the system has non-solute species and particles
    // defined (and no Grand-canonical trials), as this would be a
    // simulation without solute.  This method tests for this special
    // case before throwing an error.
    //
    
    std::tuple< double, bool, double, bool, std::size_t, bool > compute_initial_size(double input_volume);

    // Generate an initial ensemble to run a simulation.
    //
    // Throws an INPUT error if there is an error in the predefined
    // particles or if it can not add enough particles to satisfy
    // the target concentration.  The last case probably indicates
    // an input error as the grid based position generator should
    // give structures close to that of a crystal before it fails.
    
    void generate_simulation(geometry::geometry_manager & gman, utility::random_distribution & ranf, std::ostream & oslog);

    // Set the intended average number of particles in the simulation.
    void set_target_count(std::size_t val)
    {
      this->target_particles_ = val;
    }

    // Calculated the target particle count from target
    // ionic strength and the given volume.
    //
    // \pre target_ionic_strength /= 0
    // \pre vol /= 0
    //
    // count = vol * target_ionic_strength / to_SI
    void set_target_count_by_volume(double vol);

    // Adds factory methods and input parsers for all the types that can be instantiated from 
    // the input file.
    static void build_input_delegater(boost::shared_ptr< particle_manager > pman, core::input_delegater & delegate);


};
template<class Archive> inline void particle_manager::serialize(Archive & ar, const unsigned int version) 
{
  ar & species_;
  ar & ensemble_;
  ar & target_particles_;

}


} // namespace particle
#endif
