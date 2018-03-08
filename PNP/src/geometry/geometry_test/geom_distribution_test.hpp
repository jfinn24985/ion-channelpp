#ifndef IONCH__GEOM_DISTRIBUTION_TEST_HPP
#define IONCH__GEOM_DISTRIBUTION_TEST_HPP

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

namespace geometry { class coordinate_set; } 
namespace geometry { class cube_region; } 
namespace geometry { class cylinder_region; } 
namespace utility { template<class Float_type> struct ftoi; } 
namespace geometry { class geometry_manager; } 
namespace geometry { class membrane_cell_inserter; } 
namespace geometry { class membrane_cylinder_region; } 
namespace geometry { class open_cylinder_region; } 
namespace geometry { class open_split_cylinder_region; } 
namespace geometry { class periodic_cube_region; } 
namespace geometry { class tubular_grid; } 
namespace geometry { class base_region; } 

// Test if region new position generation is equivolume.
class geom_distribution_test
 {
   public:
    // Check if a region calculates the correct volume for
    // itself both for the full volume and for the volume
    // accessible to a particle of the given radius.
    static void calculate_volume_test(const geometry::base_region & reg, double full_volume, double radius, double reduced_volume);

    // Test distribution of new positions for a cubic region
    // is equivolume.
    //
    // \require fits(okradius)
    static void cubic_region_new_position_test(const geometry::base_region & regn, double okradius, std::string title);

    // Test distribution of new positions for a cylindrical region
    // is equivolume.
    //
    // \param regn : region to test
    // \param offset : Any split and offset of the cylinder from the z origin
    // \param okradius : a valid sphere radius for the cylinder
    //
    // \require fits(okradius)
    static void cylinder_region_new_position_test(const geometry::base_region & regn, double okradius, std::string title);

    // Test distribution of new positions for a cylindrical region
    // is equivolume.
    //
    // \param regn : region to test
    // \param offset : Any split and offset of the cylinder from the z origin
    // \param okradius : a valid sphere radius for the cylinder
    //
    // \require fits(okradius)
    static void general_new_position_test(const geometry::base_region & regn, double okradius, std::string title);


};
#endif
