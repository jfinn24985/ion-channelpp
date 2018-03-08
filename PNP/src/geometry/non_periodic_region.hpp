#ifndef IONCH_GEOMETRY_NON_PERIODIC_REGION_HPP
#define IONCH_GEOMETRY_NON_PERIODIC_REGION_HPP

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

#include "geometry/base_region.hpp"
#include <string>
#include <boost/serialization/vector.hpp>
#include <cstddef>

namespace geometry { class coordinate; } 
namespace geometry { class coordinate_set; } 

namespace geometry {

class non_periodic_region : public base_region
 {
   protected:
    non_periodic_region() = default;


   public:
    non_periodic_region(std::string label);

    virtual ~non_periodic_region();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object< geometry::base_region >(*this);
    }

    //Compute the distances between given position and existing positions.
    //
    //\pre rij.size == endindex
    //\post rij[0:startindex] === 0
    void do_calculate_distances_sq(const coordinate & pos, const coordinate_set & others, std::vector< double >& result, std::size_t startindex, std::size_t endindex) const override;


   protected:
    //Compute distance between two coordinates.
    double calculate_distance_squared(const coordinate & pos1, const coordinate & pos2) const;


   private:
    // Can not wrap particle so just return is_inside(pos, radius)
    //
    // \return is_inside(pos, radius)
    virtual bool do_new_position_wrap(coordinate & pos, double radius) const override;


};

} // namespace geometry

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( geometry::non_periodic_region );

#endif
