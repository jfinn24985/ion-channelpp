#ifndef IONCH_PLATFORM_MPI_STORAGE_TEST_HPP
#define IONCH_PLATFORM_MPI_STORAGE_TEST_HPP

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

#include <string>

namespace platform { class mpi_storage; } 
namespace platform { class simulation; } 
namespace platform { class simulation_definition; } 
namespace platform { class simulation_manager; } 
namespace platform { class simulator_meta; } 
namespace platform { class standard_simulation; } 
namespace platform { class storage_meta; } 

namespace platform {

// Unit tests for the MPI storage_manager.
class mpi_storage_test
 {
   public:
    // Test input file content.
    static std::string canon_input();


};

} // namespace platform
#endif
