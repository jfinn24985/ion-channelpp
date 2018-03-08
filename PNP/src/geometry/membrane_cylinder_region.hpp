#ifndef IONCH_GEOMETRY_MEMBRANE_CYLINDER_REGION_HPP
#define IONCH_GEOMETRY_MEMBRANE_CYLINDER_REGION_HPP

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
#include "geometry/membrane_cell_inserter.hpp"
#include "utility/archive.hpp"

#include <boost/ptr_container/serialize_ptr_vector.hpp>
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

// Cylindrical region centred on the origin with linear axis
// in z direction. All walls are considered hard. Cylinder has
// a membrane bissecting the cell in the centre providing two
// large compartments.  The two compartments are displaced by
// "offset" from the origin along the z-axis. A tube with rounded
// edges links the two compartments.
//
// Dimensions:
//
// Rad : Compartment radius
// HL  : Compartment length
// offset : Membrane half width
// rad : Radius of joining tube (rad << Rad)
// arc : Radius of rounded edge at tube mouth (rad + arc < Rad and arc < offset) 
//
// Valid coordinate range: 
//
// Compartments
// * [offset <= |z| <= offset + HL] and [x*x + y*y <= Rad*Rad]
//
// Tube
// * [|z| < offset] and [x*x + y*y <= rad*rad]
// * plus [offset - arc < |z| < offset] and in arc section.
//

class membrane_cylinder_region : public cylinder_region
 {
   private:
    // Cached data for use in generating a new position.
    mutable boost::ptr_vector< membrane_cell_inserter > cache_;

    // Offset of cell compartments from origin along z-axis.
    double channel_half_length_;

    // The radius of the connecting tube/channel.
    double channel_radius_;

    // The radius of the arc that rounds the tube into the membrane wall.
    double arc_radius_;

    // For serialization only
    membrane_cylinder_region() = default;


   public:
    // Create new cylinder with the given dimensions.
    //
    // Cylinder extent is z in +/-(offset + half_len) 
    //
    // \param rad : the radius of the cylinder.
    // \param half_len : half the length of the cylinder.
    // \param offset : the offset along the z-axis.
    membrane_cylinder_region(std::string label, double rad, double half_len, double offset, double tube_radius, double arc_radius)
    : cylinder_region( label, rad, half_len )
    , cache_()
    , channel_half_length_( offset )
    , channel_radius_( tube_radius )
    , arc_radius_( arc_radius )
    {}

    virtual ~membrane_cylinder_region();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< geometry::cylinder_region >(*this);
      ar & this->cache_;
      ar & this->channel_half_length_;
      ar & this->channel_radius_;
      ar & this->arc_radius_;
    }


   public:
    // Get the halflength of the gap between the two cylinder halves
    double arc_radius() const
    {
      return this->arc_radius_;
    }

    // Set the halflength of the gap between the two cylinder halves
    void arc_radius(double val)
    {
      this->arc_radius_ = val;
    }


   private:
    // Increase or decrease the system dimensions to match the new
    // volume. Cylinders maintain ratio of half_length to radius.
    //
    // * NOTE : This is expected to be used only during system
    // initialization, ie before checking particle positions or
    // asking for new particle positions. Whether any check or
    // particle postion calculated before calling this method is
    // still valid is undefined.
    
    virtual void do_change_volume(double vol, double rad) override;


   public:
    // Get the halflength of the gap between the two cylinder halves
    double channel_half_length() const
    {
      return this->channel_half_length_;
    }

    // Set the halflength of the gap between the two cylinder halves
    void channel_half_length(double val)
    {
      this->channel_half_length_ = val;
    }

    // Get the halflength of the gap between the two cylinder halves
    double channel_radius() const
    {
      return this->channel_radius_;
    }

    // Set the halflength of the gap between the two cylinder halves
    void channel_radius(double val)
    {
      this->channel_radius_ = val;
    }


   private:
    // Calculate a bounding box for the valid positions of
    // spheres of the given radius within the region. The
    // bounding box is defined by its most negative
    // (small_corner) and most positive (big_corner).
    //
    // \pre fits(radius)
    // \post big.(x|y|z) - small.(x|y|z) >= 0
    void do_extent(coordinate & small_corner, coordinate & big_corner, double radius) const override;

    // True if there is any position in the region where a spherical
    // particle of the given size can be completely included, this
    // ignores the length.
    virtual bool do_fits(double radius) const override;

    // Calculate if a sphere centred at the given point and with
    // the given radius is within the region. The checks if the
    // sphere is in the cell compartments or the channel.
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
    // be inside the region. The position distribution should be uniform 
    // over the volume.
    //
    // \pre fits(radius)
    // \post is_inside(pos, radius)
    //
    // PROs: bound number of calls to rgnr and bound calculation time.
    //
    // CONs: complex, requires careful testing to guarrantee equivolume 
    //   distribution.
    
    virtual coordinate do_new_position(utility::random_distribution & rgnr, double radius) const override;


   public:
    // Calculate new random position for particle of the given
    // radius to be inside the region. This alternate version
    // randomly allocates a point in the bounding cylinder (V_bound)
    // and tests to see if it is in the allowed volume, repeating
    // the procedure until a point lies within the allowed volume
    // (V_allowed). The run time of the method is therefore unbound,
    // but should average to V_bound/V_allowed times longer than a
    // cylinder of V_bound.
    //
    // \pre fits(radius)
    // \post is_inside(pos, radius)
    //
    // PROs: robust, simple to guarrantee equivolume 
    //   distribution.
    //
    // CONs: unbound number of calls to rgnr.
    //
    
    coordinate alt_new_position(utility::random_distribution & rgnr, double radius) const;


   private:
    // Calculate volume accessible to centre-points of spherical particles
    // with the given radius
    //
    // NOTE: can assume fits(radius)
    double do_volume(double radius) const override
    {
      return compute_volume( this->radius(), this->half_length() * 2 );
    }

   public:
    // Volume of cell between z0 and z1
    //
    // \pre radius < channel_radius
    // \pre z0, z1 in [ 0, channel_half_length + half_length - radius ]
    // \pre z0 < z1
    double sub_volume(double z0, double z1, double radius);


   private:
    // Update wr[ix] section with information from the derived class.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    static std::string type_label()
    {
      const static std::string result( "cell-membrane" );
      return result;
    }


   private:
    // Factory method for creating membrane_cylinder region objects from
    // input data.
    static boost::shared_ptr< base_region > region_factory(std::string label, const std::vector< core::input_parameter_memo > & params);


   public:
    // Create and add a region_definition to the meta object.
    static void add_definition(region_meta & meta);


};

} // namespace geometry

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( geometry::membrane_cylinder_region );

#endif
