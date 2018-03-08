#ifndef IONCH_TRIAL_CHOICE_HPP
#define IONCH_TRIAL_CHOICE_HPP

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
#include <map>
// End
#include <boost/noncopyable.hpp>

#include "utility/archive.hpp"

#include "particle/change_hash.hpp"
#include "utility/unique_ptr.hpp"

namespace particle { class change_set; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace utility { class random_distribution; } 

namespace trial {

// Objects of this class can be used to generate a trial MC move. 
//
// Normalization of the probabilities of the choice objects is the 
// responsibility of the user (e.g. a choice_manager object).
class base_choice : public boost::noncopyable
 {
   private:
    //The ID of the change
    particle::change_hash key_;

    // The chance this choice happens. Normalization of the probabilities
    // of the choice objects is the responsibility of the user.
    double probability_;


   public:
    virtual ~base_choice();

    // Make a choice without a parent
    base_choice(const particle::change_hash & key)
    : key_ (key)
    , probability_(0.0)
    {}
    


   protected:
    //Make a choice for use in deserialization
    base_choice() = default;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & key_;
        ar & probability_;
      }


   public:
    // Generate a new change set based on this choice type.
    //
    // \post return.size() > 0
    virtual std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr) = 0;

    // The chance this choice happens. 
    double probability() const
    {
      return this->probability_;
    }

    //Change probability of this choice
    void set_probability(double value)
    {
      this->probability_ = value;
    }

    // This choice ID 
    const particle::change_hash& key() const
    {
      return this->key_;
    }


};

} // namespace trial

#endif
