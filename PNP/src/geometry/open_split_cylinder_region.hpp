#ifndef IONCH_GEOMETRY_OPEN_SPLIT_CYLINDER_REGION_HPP
#define IONCH_GEOMETRY_OPEN_SPLIT_CYLINDER_REGION_HPP

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
#include <boost/serialization/shared_ptr.hpp>
#include <cstddef>
#include <boost/serialization/vector.hpp>

namespace geometry { class grid_generator; } 
namespace utility { class random_distribution; } 
namespace core { class input_document; } 
namespace geometry { class base_region; } 
namespace core { class input_parameter_memo; } 
namespace geometry { class region_meta; } 

namespace geometry {

// Cylindrical region centred on the origin with linear axis in z
// direction. All walls are considered soft. Cylinder is cut in the
// centre and the two parts displaced "offset" from the origin
// along the z-axis.
//
// Coordinate range:
// * x*x + y*y <= rad*rad
// * -(len/2 + offset) <= z <= -(offset) AND (offset) <= z <= +(len/2 + offset)
class open_split_cylinder_region : public cylinder_region
 {
   private:
    // Offset from origin along z-axis.
    double offset_;


   protected:
    // For serialization only
    open_split_cylinder_region() = default;


   public:
    // Create new cylinder with the given dimensions.
    //
    // Cylinder extent is z in +/-(offset + half_len) 
    //
    // \param rad : the radius of the cylinder.
    // \param half_len : half the length of the cylinder.
    // \param offset : the offset along the z-axis.
    open_split_cylinder_region(std::string label, double rad, double half_len, double offset)
    : cylinder_region( label, rad, half_len )
    , offset_( offset )
    {}

    virtual ~open_split_cylinder_region();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< geometry::cylinder_region >(*this);
      ar & this->offset_;
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
    virtual bool do_fits(double radius) const override;

    // Calculate if a sphere centred at the given point and with
    // the given radius is within the region. This class ignores the 
    // radius when considering the z direction.
    virtual bool do_is_inside(const coordinate & pos, double radius) const override;

    // Create a gridder object for this system. The gridder is
    // guarranteed to have the minimum given number of points.
    // The gridder provides the positions in a random sequence.
    //
    // (if count = 0 it is incremented to 1).
    //
    // As the grid is highly regular, the number of nodes will be an
    // integral function of the number of points in each dimension.
    // For a cube will always have the same number of nodes in
    // each dimension and so the total number will be the cube of a
    // whole number. The actual number of nodes will therefore be
    // the smallest solution to this node function that is larger
    // than count.
    
    boost::shared_ptr< grid_generator > do_make_gridder(utility::random_distribution & rgenr, std::size_t count) const override;

    // Create a gridder object for this system. The gridder inter-node
    // distance is guarranteed to have the given minimum spacing.
    // Every node is also guarranteed to be a minimum of (spacing/2)
    // distant from any side.  The gridder provides the positions
    // in a random sequence.
    //
    // The spacing may be increased slightly to more evenly fit the
    // particles into the region. The gridder calculates the maximum
    // number of nodes that can fit within the region and have the
    // given spacing. It then relaxes the grid to evenly fit in the
    // region and recalculates the spacing.
    
    boost::shared_ptr< grid_generator > do_make_gridder(double spacing, utility::random_distribution & rgenr) const override;

    // Calculate new random position for particle of the given radius to
    // be inside the region. This class ignores the radius when calculating
    // the new z coordinate.
    //
    // \pre fits(radius)
    // \post is_inside(pos, radius)
    virtual coordinate do_new_position(utility::random_distribution & rgnr, double radius) const override;


   public:
    // Get the halflength of the gap between the two cylinder halves
    double offset() const
    {
      return this->offset_;
    }

    // Set the halflength of the gap between the two cylinder halves
    void offset(double val)
    {
      this->offset_ = val;
    }


   private:
    // Calculate volume accessible to centre-points of spherical particles
    // with the given radius
    //
    // NOTE: can assume fits(radius)
    double do_volume(double radius) const override
    {
      return compute_volume( this->radius(), this->half_length() * 2 );
    }
    // Update wr[ix] section with information from the derived class.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;

    static std::string type_label()
    {
      const static std::string result( "split-cylinder" );
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
BOOST_CLASS_EXPORT_KEY( geometry::open_split_cylinder_region );

#endif
