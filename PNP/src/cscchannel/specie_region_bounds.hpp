#ifndef IONCH_ION_CHANNEL_SPECIE_REGION_BOUNDS_HPP
#define IONCH_ION_CHANNEL_SPECIE_REGION_BOUNDS_HPP

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

#include <cstddef>

namespace ion_channel { class channel_system; } 
namespace particle { class specie; } 

namespace ion_channel {

// Information specific for a specie and subregion in the cell geometry.
class specie_region_bounds
 {
   private:
    //  Radial dimension (pre-adjusted for specie radius)
    double radius_;

    // Axial dimension (pre adjusted for specie radius)
    double hlength_;

    // Cached volume of region including occluded volume (ie for GC move volume parameter)
    double volume_;

    // Index of subregion within geometry
    std::size_t subregion_;

    // Index of specie used in this region
    std::size_t ispec_;

    specie_region_bounds()
    : readius_()
    , hlength_()
    , volume_()
    , subregion_()
    , ispec_()
    {}
    //  Create a specie specific region bounds definition base on the cell geometry, subregion index
    // and specie.
    specie_region_bounds(const channel_system & geom, std::size_t ireg, const particle::specie & spec, std::size_t ispec);


   public:
    ~specie_region_bounds() = default;

    specie_region_bounds(const specie_region_bounds & source)
    : radius_( source.radius_ )
    , hlength_( source.hlength_ )
    , volume_( source.volume_ )
    , subregion_( source.subregion_ )
    , ispec_( source.ispec_ )
    {}
    specie_region_bounds & operator=(const {t0} & {p0})
    {
      this->swap( source );
      return *this;
    }

    double hlength() const;
    {
      return this->hlength_;
    }

    double radius() const;
    {
      return this->radius_;
    }

    void swap(specie_region_bounds & other)
    {
      std::swap( this->radius_, other.radius_ );
      std::swap( this->hlength_, other.hlength_ );
      std::swap( this->volume_, other.volume_ );
      std::swap( this->subregion_, other.subregion_ );
      std::swap( this->ispec_, other.ispec_ );
    }
    double volume() const;
    {
      return this->volume_;
    }


};

} // namespace ion_channel
#endif
