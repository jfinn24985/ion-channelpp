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
//${butter_generic}
//HDR=$(MPI_INC)

#ifndef DEBUG
#define DEBUG 0
#endif

// Default to a periodic cube base region. This can be
// overridden in the input file.
#include "geometry/periodic_cube_region.hpp"
// Use MPI storage for this execution.
#include "platform/mpi_storage.hpp"
#include "platform/simulation.hpp"

#include <boost/mpi.hpp>

int main( int argc, char ** argv )
{
  int result = 0;
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::mpi_storage > stor( new platform::mpi_storage );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );

  return ( not sim->main( argc, argv, *sim ) ? 0 : 1 );
}

