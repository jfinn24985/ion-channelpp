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

// headers
#include "cscpbc/periodic_system.hpp"
#include "platform/mpi_application.hpp"

#ifndef DEBUG
#define DEBUG 0
#endif

//END

int main (int argc, char** argv)
{
   // Output the program specific description.
  platform::mpi_application::simulator_.reset( new periodic_cube::periodic_system );
   // Process the input data
   if ( not platform::mpi_application::main( argc, argv ) )
   {
      // error or help request in input
      return 1;
   }
   return 0;
}


