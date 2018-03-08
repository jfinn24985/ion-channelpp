#ifndef IONCH__TEST_CHOICE_HPP
#define IONCH__TEST_CHOICE_HPP

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

#include "trial/choice.hpp"
#include <cstddef>
#include "particle/change_hash.hpp"
#include "utility/unique_ptr.hpp"
#include <boost/serialization/map.hpp>
#include <string>

namespace particle { class change_set; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace utility { class random_distribution; } 
namespace particle { class specie; } 

// Provides no-op implementation of "generate" to give a concrete 
// class for testing abstract base class base_choice.
// 
class test_choice : public trial::base_choice
 {
   protected:
    // For serialization
    test_choice() = default;

   public:
    test_choice(std::size_t ispec)
    : trial::base_choice( particle::change_hash( ispec, 0, 0, 0 ) )
    {}

    test_choice(particle::change_hash key)
    : trial::base_choice( key )
    {}


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
      };


   public:
    //Generate a new change set based on this choice type.
    virtual std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr) override 
    {
      return std::unique_ptr< particle::change_set >{};
    }

    // Can this specie be used to create a trial of the current type?
    static bool permitted(const particle::specie & spec)
    {
      return true;
    }

    // Generate a choice object.
    static std::unique_ptr< trial::base_choice > make_choice(std::size_t ispec, const std::map< std::string, std::string >& params)
    {
      return std::unique_ptr< trial::base_choice >( new test_choice( ispec ) );
    }


};

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT_KEY(test_choice);
#endif
