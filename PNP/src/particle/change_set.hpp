#ifndef IONCH_PARTICLE_CHANGE_SET_HPP
#define IONCH_PARTICLE_CHANGE_SET_HPP

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

#include "particle/specie_key.hpp"
#include <boost/serialization/vector.hpp>
#include <cstddef>
#include "geometry/coordinate.hpp"
#include <iostream>
#include "particle/change_hash.hpp"

namespace particle {

//Describe a change to a single particle.
class change_atom
 {
   public:
    //The inter-particle distances from any new position on
    //this atom.
    std::vector<double> old_rij;

    //The inter-particle distances from any new position on
    //this atom.
    std::vector<double> new_rij;

    //The inter-particle distances from any new position on
    //this atom.
    std::vector<double> old_rij2;

    //The inter-particle distances from any new position on
    //this atom.
    std::vector<double> new_rij2;

    //Calculate the energy of the old position.
    bool do_old;

    //Calculate the energy of the new position
    bool do_new;

    double energy_old;

    double energy_new;

    //The key for the given particle.
    std::size_t key;

    //The (possible) index of the particle.
    std::size_t index;

    //The possible new changed position
    geometry::coordinate new_position;

    //The possible old position
    geometry::coordinate old_position;


   protected:
    change_atom(bool use_old, bool use_new, std::size_t aindex, const geometry::coordinate & aoldpos, const geometry::coordinate & anewpos, std::size_t akey);


   public:
    //Default ctor
    change_atom();

    //Construct with specie key
    change_atom(std::size_t ispec);

    ~change_atom()
    {}

    change_atom(const change_atom & source) = default;
    change_atom(change_atom && source);

    change_atom & operator=(change_atom source)
    {
      this->swap( source );
      return *this;
    }

    void swap(change_atom & source)
    {
      std::swap (old_rij, source.old_rij);
      std::swap (new_rij, source.new_rij);
      std::swap (old_rij2, source.old_rij2);
      std::swap (new_rij2, source.new_rij2);
      std::swap (do_old, source.do_old);
      std::swap (do_new, source.do_new);
      std::swap (energy_old, source.energy_old);
      std::swap (energy_new, source.energy_new);
      std::swap (key, source.key);
      std::swap (index, source.index);
      std::swap (new_position, source.new_position);
      std::swap (old_position, source.old_position);
    }


  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & old_rij;
        ar & new_rij;
        ar & old_rij2;
        ar & new_rij2;
        ar & do_old;
        ar & do_new;
        ar & energy_old;
        ar & energy_new;
        ar & key;
        ar & index;
        ar & new_position;
        ar & old_position;
      };


   public:
    bool equivalent(const change_atom & rhs) const;

    // Fill old_rij and new_rij from old_rij2 and new_rij2.
    void ensure_rij();

    friend std::ostream& operator<<(std::ostream & os, const change_atom & rhs);

    friend bool operator==(const change_atom & lhs, const change_atom & rhs)
    {
      return lhs.equivalent( rhs );
    }

    friend bool operator!=(const change_atom & lhs, const change_atom & rhs)
    {
      return not lhs.equivalent( rhs );
    }


};
//The set of change steps making up the trial move.
//
//NOTE: Does not serialize pointer to choice. All deserialized
//objects will have choice_ == nullptr (see below)
//
//A trial change is composed of two stages: generating the
//trial change and evaluating the change.  This class maintains
//information from both stages.  One important piece of
//information is whether the change is somehow not valid, this is
//tested using the ``fail`` method.  This value does not indicate
//the result of the Metropolis acceptance test.  An invalid
//change set will always be rejected in the Metropolis test,
//however a valid change set may also be rejected.  The validity
//of the change set therefore has the following consequences.
//
//+ A client object calls ``set_fail`` if it detects that the
//current changes are invalid for some reason.
//
//Serialization: This object is not intended to be serialized
//into an archive. Serialization of this object is targeted at
//transfer between parts of a parallel simulation. For this
//purpose, only the source task will be generating and approving
//change_sets and so sink tasks do not need the link to the
//choice object that generated the change_set.

class change_set
 {
   public:
    //Iterator type of the vector of change atoms.
    typedef std::vector< change_atom >::const_iterator const_iterator;

    //Iterator type of the vector of change atoms.
    typedef std::vector< change_atom >::iterator iterator;


   private:
    //The set of changes
    std::vector< change_atom > changes_;

    //  Change in the chemical potential (in atomic units)
    //
    //  Only valid if self.fail = True
    double exponential_factor_;

    //  Probability proportionality factor for the change in configuration
    //
    //  Only valid if self.fail = True
    double probability_factor_;

    // The change atom identifier/key
    change_hash id_;

    //Is this change still valid
    bool fail_;

    //Whether trial was commited or not
    bool accept_;


   public:
    change_set()
    : changes_()
    , exponential_factor_ (0.0)
    , probability_factor_ (1.0)
    , id_()
    , fail_ (false)
    , accept_ (false)
    {}
    ~change_set() {}

    change_set(change_set && source)
    : changes_( std::move( source.changes_ ) )
    , exponential_factor_( std::move( source.exponential_factor_ ) )
    , probability_factor_( std::move( source.probability_factor_ ) )
    , id_( std::move( source.id_ ) )
    , fail_( std::move( source.fail_ ) )
    , accept_( std::move( source.accept_ ) )
    {}

    change_set(const change_set & source)
    : changes_( source.changes_ )
    , exponential_factor_( source.exponential_factor_ )
    , probability_factor_( source.probability_factor_ )
    , id_( source.id_ )
    , fail_( source.fail_ )
    , accept_( source.accept_ )
    {}

    change_set(const change_hash & id)
    : changes_()
    , exponential_factor_( 0.0 )
    , probability_factor_( 1.0 )
    , id_( id )
    , fail_( false )
    , accept_( false )
    {}

    change_set & operator=(change_set source)
    {
      this->swap( source );
      return *this;
    }
    

    void swap(change_set & source)
    {
      std::swap( this->changes_, source.changes_ );
      std::swap( this->exponential_factor_, source.exponential_factor_ );
      std::swap( this->probability_factor_, source.probability_factor_ );
      std::swap( this->id_, source.id_ );
      std::swap( this->fail_, source.fail_ );
      std::swap( this->accept_, source.accept_ );
    }
    


  friend class boost::serialization::access;
   private:
    //NOTE: Does not serialize pointer to choice. All deserialized
    //objects will have choice_ == nullptr
    //
    
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
      ar & changes_;
      ar & exponential_factor_;
      ar & probability_factor_;
      ar & id_;
      ar & fail_;
      ar & accept_;
    }


   public:
    //Has this change been accepted 
    //
    //This is false until after calling commit, when
    //it will be true.
    bool accept() const
    {
      return this->accept_;
    }

    //Add an atom to the change set
    void add_atom(const change_atom & an_atom)
    {
       this->changes_.push_back(an_atom);
    }

    const_iterator begin() const
    {
      return this->changes_.begin();
    }

    iterator begin()
    {
      return this->changes_.begin();
    }

    const_iterator end() const
    {
      return this->changes_.end();
    }

    iterator end()
    {
      return this->changes_.end();
    }

    double energy() const;

    // Test if two change_atoms have the same data.
    inline bool equivalent(const change_set & other) const;

    double exponential_factor() const
    {
      return this->exponential_factor_;
    }

    //Is this change valid?
    bool fail() const
    {
      return this->fail_;
    }

    // Get the change set's ID/key
    change_hash id() const
    {
      return this->id_;
    }

    // Set the change set's ID/key
    void id(const change_hash & id)
    {
      this->id_ = id;
    }

    //Calculate the Metropolis-Hasting probability for this change
    //
    ///pre is_valid
    // /return  (0.0, VERY_LARGE)
    double metropolis_factor() const;

    const change_atom& operator [](std::size_t index) const
    {
      return this->changes_[index];
    }

    double probability_factor() const
    {
      return this->probability_factor_;
    }

    void set_accept(bool val)
    {
      this->accept_ = val;
    }

    void set_fail()
    {
      this->fail_ = true;
    }

    std::size_t size() const
    {
      return this->changes_. size();
    }

    //  Add to the chemical potential for the change in the configuration ("beta" units)
    //
    //   \ensure self.chemical_potential = a_value + old(self.chemical_potential)
    void update_exponential_factor(double a_value)
    {
      this->exponential_factor_ += a_value;
    }

    //  Update the pre-exponential probability factor for this change
    //
    //  \pre value > 0
    //  \ensure self.probability_factor = a_value * old(self.probability_factor)
    void update_probability_factor(double a_value)
    {
      UTILITY_REQUIRE (a_value > 0.0, "Bad update value, factor should be greater than 0.0");
      this->probability_factor_ *= a_value;
    }

    // Stream insert function for visualizing object, possibly
    // for debugging etc.
    friend std::ostream& operator<<(std::ostream & os, const change_set & rhs);

    friend bool operator==(const change_set & lhs, const change_set & rhs)
    {
      return lhs.equivalent( rhs );
    }

    friend bool operator!=(const change_set & lhs, const change_set & rhs)
    {
      return not lhs.equivalent( rhs );
    }


};
// Test if two change_atoms have the same data.
inline bool change_set::equivalent(const change_set & other) const
{
  return (this == &other) or
   (this->fail_ == other.fail_
   and this->accept_ == other.accept_
   and this->exponential_factor_ == other.exponential_factor_
   and this->changes_ == other.changes_);
  
  

}


} // namespace particle
#endif
