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

#include "geometry/geometry_manager.hpp"
#include "utility/archive.hpp"

#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"
#include "core/input_document.hpp"
#include "core/input_delegater.hpp"

// manual
#include "geometry/cube_region.hpp"
#include "geometry/cylinder_region.hpp"
#include "geometry/open_cylinder_region.hpp"
#include "geometry/open_split_cylinder_region.hpp"
#include "geometry/periodic_cube_region.hpp"
#include "geometry/region_meta.hpp"
// -
namespace geometry {

geometry_manager::geometry_manager(boost::shared_ptr< base_region > system_region)
: regions_()
{
  this->regions_.push_back( system_region );
}

geometry_manager::~geometry_manager()
{

}

// Add region to manager. If a region with the same label
// exists in the manager, the new object replaces the old.
void geometry_manager::add_region(boost::shared_ptr< base_region > regn)
{
  bool found = false;
  for (boost::shared_ptr< geometry::base_region > &rgn : this->regions_)
  {
    if( rgn->label() == regn->label() )
    {
      found = true;
      rgn = regn;
      break;
    }
  }
  if( not found )
  {
    this->regions_.push_back( regn );
  }

}

// Increase or decrease the system dimensions to match the new
// volume. Derived classes each have rules to decide how the
// region's dimensions change to accomodate the new volume. For
// example a cube simply adjusts one length while a cylinder must
// distribute the change over the radial and linear dimensions.
//
// Pre/post-conditions from region.change_volume(...)
//
// \pre rad >= 0
//
// \pre vol >= 0
//
// \post volume(rad) == vol
//
// \pre AFTER VOLUME CHANGE fits(rad)
//
// * NOTE : This is expected to be used only during system
// initialization, ie before checking particle positions or
// asking for new particle positions. Whether any check or
// particle postion calculated before calling this method is
// still valid is undefined.

void geometry_manager::change_volume(double vol, double rad)
{
  this->regions_[0]->change_volume( vol, rad );
}

// Puts the set of regions the given point is in into
// regns. The set may be empty.
void geometry_manager::locate_region(const coordinate & pos, double rad, std::vector< boost::shared_ptr< base_region > >& regns) const
{
  regns.clear();
  for (boost::shared_ptr< base_region > const& rgn : this->regions_)
  {
    if( rgn->is_inside( pos, rad ) )
    {
      regns.push_back( rgn );
    }
  }

}

// Find a regions key from its label/name
//
// \return index if found or region_count if not found.
std::size_t geometry_manager::region_key(std::string label) const
{
  for (std::size_t ii = 0; ii != this->regions_.size(); ++ii)
  {
    if( this->regions_[ii]->label() == label )
    {
      return ii;
    }
  }
  return this->regions_.size();

}

// Get all samplers to write their input information.
void geometry_manager::write_document(core::input_document & wr) const 
{
  for (auto const& region : this->regions_)
  {
    region->write_document( wr );
  }

}

// Adds factory methods and input parsers for all the types that can be instantiated from 
// the input file.
void geometry_manager::build_input_delegater(boost::shared_ptr< geometry_manager > gman, core::input_delegater & delegate)
{
  ///////////////
  // Region types
  boost::shared_ptr< region_meta > gmeta { new region_meta( gman ) };
  
  // Cube
  cube_region::add_definition( *gmeta );
  
  // Cylinder (basic)
  cylinder_region::add_definition( *gmeta );
  
  // Open cylinder
  open_cylinder_region::add_definition( *gmeta );
  
  // Open+Split cylinder
  open_split_cylinder_region::add_definition( *gmeta );
  
  // Periodic Cube
  periodic_cube_region::add_definition( *gmeta );
  
  delegate.add_input_delegate( gmeta );
  

}


} // namespace geometry
