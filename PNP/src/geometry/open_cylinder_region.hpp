#ifndef IONCH_GEOMETRY_OPEN_CYLINDER_REGION_HPP
#define IONCH_GEOMETRY_OPEN_CYLINDER_REGION_HPP

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

#include "geometry/cylinder_region.hpp"
#include "utility/archive.hpp"

#include <string>
#include "geometry/coordinate.hpp"
#include <cstddef>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace utility { class random_distribution; } 
namespace core { class input_document; } 
namespace geometry { class base_region; } 
namespace core { class input_parameter_memo; } 
namespace geometry { class region_meta; } 

namespace geometry {

// Cylindrical region centred on the origin with linear axis in z
// direction. The curved wall is considered a hard wall but the
// ends are not when considering valid positions.
//
// Coordinate range:
// * x*x + y*y <= rad*rad
// * -len/2 <= x <= +len/2
class open_cylinder_region : public cylinder_region
 {
   protected:
    // For serialization only
    open_cylinder_region() = default;


   public:
    // Create new cylinder with the given dimensions.
    //
    // \param label : user-defined label for the region.
    // \param rad : the radius of the cylinder.
    // \param half_len : half the length of the cylinder.
    open_cylinder_region(std::string label, double rad, double half_len)
    : cylinder_region( label, rad, half_len )
    {}
    

    virtual ~open_cylinder_region();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< geometry::cylinder_region >(*this);
    }

    // Increase or decrease the system dimensions to match the new
    // volume. Cylinders maintain ratio of half_length to radius.
    //
    // * NOTE : This is expected to be used only during system
    // initialization, ie before checking particle positions or
    // asking for new particle positions. Whether any check or
    // particle postion calculated before calling this method is
    // still valid is undefined.
    
    virtual void do_change_volume(double vol, double rad) override;

    // True if there is any position in the region where a spherical
    // particle of the given size can be completely included, this
    // ignores the length.
    virtual bool do_fits(double radius) const;

    // Calculate if a sphere centred at the given point and with
    // the given radius is within the region. This class ignores the 
    // radius when considering the z direction.
    virtual bool do_is_inside(const coordinate & pos, double radius) const override;

    // Calculate new random position for particle of the given radius to
    // be inside the region. This class ignores the radius when calculating
    // the new z coordinate.
    //
    // \pre fits(radius)
    // \post is_inside(pos, radius)
    virtual coordinate do_new_position(utility::random_distribution & rgnr, double radius) const override;

    // Calculate volume accessible to centre-points of spherical particles
    // with the given radius
    //
    // NOTE: can assume fits(radius)
    double do_volume(double radius) const override
    {
      return compute_volume( this->radius() - radius, this->half_length() * 2 );
    }
    // Update wr[ix] section with information from the derived class.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;

    static std::string type_label()
    {
      const static std::string result( "open-cylinder" );
      return result;
    }

    // Factory method for creating cube_region objects from
    // input data.
    static boost::shared_ptr< base_region > region_factory(std::string label, const std::vector< core::input_parameter_memo > & params);


   public:
    // Create and add a region_definition to the meta object.
    static void add_definition(region_meta & meta);


};

} // namespace geometry

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( geometry::open_cylinder_region );

#endif
