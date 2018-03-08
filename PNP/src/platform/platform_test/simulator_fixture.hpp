#ifndef IONCH_PLATFORM_SIMULATOR_FIXTURE_HPP
#define IONCH_PLATFORM_SIMULATOR_FIXTURE_HPP

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

#include "utility/unique_ptr.hpp"

namespace platform { class simulator_test_dummy; } 

namespace platform {

// Class to set up and clean up a dumm simulator class
struct simulator_fixture 
{
    // Dummy simulator
    std::unique_ptr< simulator_test_dummy > sim;

    simulator_fixture();

    ~simulator_fixture();


   private:
    simulator_fixture(const simulator_fixture & source) = delete;

    simulator_fixture(simulator_fixture && source) = delete;

    simulator_fixture & operator=(const simulator_fixture & source) = delete;


   public:
    // Add solute specie definitions with predefined particles.
    void add_solute_species();

    // Add a localized specie (with predefined location/centroid)
    void add_localized_species();

    // Add solute specie definitions with predefined particles.
    void add_solute_specie_definitions();


};

} // namespace platform
#endif
