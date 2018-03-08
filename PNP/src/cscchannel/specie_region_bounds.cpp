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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "cscchannel/specie_region_bounds.hpp"
#include "cscchannel/channel_system.hpp"
#include "particle/specie.hpp"

namespace ion_channel {

//  Create a specie specific region bounds definition base on the cell geometry, subregion index
// and specie.
specie_region_bounds::specie_region_bounds(const channel_system & geom, std::size_t ireg, const particle::specie & spec, std::size_t ispec) 
: radius_()
, hlength_()
, volume_()
, subregion_(ireg)
, ispec_(ispec)
{
  // Definition of trial insertion subregion for this specie
  const double sphere_radius (spec.radius());
  switch(subregion_)
  {
  case izlim:
  {
     // Filter special region insertion
     if ( geom.use_version1_bounds() )
     {
        if ( spec.is_mobile() or spec.is_channel_only() )
        {
           // Structural ions have all of particle in filter
           hlength_ = geom.structural_hlength() - sphere_radius;
        }
        else
        {
           hlength_ = geom.structural_hlength();
        }
     }
     else
     {
        // Filter region insertion (all of particle in filter)
        hlength_ = geom.structural_hlength() - sphere_radius;
     }
     radius_ = geom.pore_radius() - sphere_radius ;
  }
  break;
  case ifilt:
  {
     // Centre region insertion
     if ( geom.use_version1_bounds() )
     {
        hlength_ = geom.pore_hlength();
     }
     else
     {
        // Centre region insertion (any of particle in cylinder)
        zhlength = geom.pore_hlength() + sphere_radius;
     }
     radius_ = geom.pore_radius() - sphere_radius;
  }
  break;
  case ichan:
  {
     // Total channel insertion
     if ( geom.use_version1_bounds() )
     {
        hlength_ = geom.pore_hlength() + geom.vestibule_arc_radius();
     }
     else
     {
        // Total channel insertion region (any part of particle close to channel)
        hlength_ = geom.pore_hlength() + geom.vestibule_arc_radius() + 2 * sphere_radius;
     }
     radius_ = geom.pore_radius() + geom.vestibule_arc_radius();
  }
  break;
  default: // ibulk
  {
     // Bulk insertion region (particle in simulation)
     hlength_ = geom.cell_hlength() - sphere_radius;
     radius_ = geom.cell_radius() - sphere_radius;
  }
  break;
  }
  volume_ = sqr(radius_) * core::constants::pi() * (2 * hlength_);

}

} // namespace ion_channel
