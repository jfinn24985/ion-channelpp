#ifndef IONCH_PLATFORM_PLATFORM_TEST_HPP
#define IONCH_PLATFORM_PLATFORM_TEST_HPP

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

#include <boost/serialization/shared_ptr.hpp>
#include <string>

namespace platform { class serial_storage; } 
namespace platform { class simulation; } 
namespace platform { class simulation_definition; } 
namespace platform { class simulation_manager; } 
namespace platform { class simulator_meta; } 
namespace platform { class standard_simulation; } 
namespace platform { class storage_meta; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 

namespace platform {

// Unit tests for the simulation class and related classes.
//
// Includes:  simulation, simulation_definition, simulation_meta
class platform_test
 {
   private:
    // Mockup set of realistic species.
    //
    //  specie_count = 5
    //  solute = "Na" (x2 particles) and "Cl" (x1 particle)
    //  mobile = "CA" (x1 particle)
    //  flexible = "CO" (x1 particle)
    //  channel only = "OX" (x1 particle)
    //  
    static boost::shared_ptr< particle::particle_manager > mockup_particle_manager();

    // Create geometry manager with PBC cube "cell" and 
    // open cube "bulk" regions.
    static boost::shared_ptr< geometry::geometry_manager > mockup_geometry_manager();


   public:
    // Test input file content.
    static std::string canon_input();


};

} // namespace platform
#endif
