#ifndef IONCH__GEOMETRY_TEST_HPP
#define IONCH__GEOMETRY_TEST_HPP

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
#include <cstddef>

namespace geometry { class centroid; } 
namespace geometry { class coordinate; } 
namespace geometry { class coordinate_set; } 
namespace geometry { class cube_region; } 
namespace geometry { class cubic_grid; } 
namespace geometry { class cylinder_region; } 
namespace geometry { class digitizer_3d; } 
namespace utility { template<class Float_type> struct ftoi; } 
namespace geometry { class geometry_manager; } 
namespace geometry { class membrane_cell_inserter; } 
namespace geometry { class open_cylinder_region; } 
namespace geometry { class open_split_cylinder_region; } 
namespace geometry { class periodic_cube_region; } 
namespace geometry { class region_meta; } 
namespace geometry { class tubular_grid; } 
namespace geometry { class base_region; } 
namespace geometry { class grid_generator; } 

// TODO derived region static and serialization/new position tests.
class geometry_test
 {
   public:
    // Check if a region calculates the correct volume for
    // itself both for the full volume and for the volume
    // accessible to a particle of the given radius.
    static void calculate_volume_test(const geometry::base_region & reg, double full_volume, double radius, double reduced_volume);

    // Test serialization from base pointer.
    static void region_serialization_test(boost::shared_ptr< geometry::base_region > regn);

    // Test result of generating a new position. If fits is always
    //  true then set bigradius to a negative number to avoid certain
    //  tests.
    // 
    //  + fits(okradius) & (bigradius > 0 and not fits(bigradius))
    //  + is_inside(inside) & not is_inside(outside)
    //  + is_inside(new_position(rand, okradius))
    //  + bigradius > 0 and throw on new_position(rand, bigradius)
    //  + is_inside(newpos) == new_position_offset(rand, inside, off, okradius)
    //  + throw on new_position_offset(rand, outside, off, okradius)
    //  + throw on new_position_offset(rand, inside, off, bigradius)
    static void region_new_position_test(const geometry::base_region & regn, const geometry::coordinate & inside, const geometry::coordinate & outside, double okradius, double bigradius);

    // Test result of generating a gridder. Ideal count values are
    // very low to large. Ideal test spacing is from small to a
    // value such that regn.fits(space/2) is false (if possible).
    // All positions should be inside the regn, and if closed should
    // be a minimum of spacing/2 from an edge.
    static void region_make_gridder_test(const geometry::base_region & regn, std::size_t min_count, std::size_t max_count, double min_space, double max_space, bool is_closed);

    //Test the functionality of the base class
    static void test_grid_generator(boost::shared_ptr< geometry::grid_generator > gridder);
































































































































};
#endif
