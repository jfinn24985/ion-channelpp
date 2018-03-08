#ifndef IONCH_GEOMETRY_BASE_REGION_HPP
#define IONCH_GEOMETRY_BASE_REGION_HPP

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

#include "geometry/distance_calculator.hpp"
#include <string>
#include "geometry/coordinate.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include <cstddef>

namespace geometry { class grid_generator; } 
namespace utility { class random_distribution; } 
namespace core { class input_document; } 

namespace geometry {

class geometry_manager;

// A region of coordinate space.
class base_region : public distance_calculator
 {
   private:
    // A user-defined label for the region.
    std::string label_;


   public:
    base_region(std::string label)
    : label_( label )
    {}


   protected:
    // Default ctor for serialization only
    base_region() = default;


   public:
    virtual ~base_region() = default;


  friend class boost::serialization::access;
   private:
    //Write the grid parameters to an archive
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
      ar & this->label_;
    }


   public:
    // Calculate a bounding box for the valid positions of spheres
    // of the given radius within the region. The bounding box is
    // defined by its most negative (small_corner) and most positive
    // (big_corner).
    //
    // \pre fits(radius) 
    // \post big.(x|y|z) - small.(x|y|z) >= 0
    
    void extent(coordinate & small_corner, coordinate & big_corner, double radius) const;


   private:
    // Calculate a bounding box for the valid positions of
    // spheres of the given radius within the region. The
    // bounding box is defined by its most negative
    // (small_corner) and most positive (big_corner).
    //
    // \pre fits(radius)
    // \post big.(x|y|z) - small.(x|y|z) >= 0
    virtual void do_extent(coordinate & small_corner, coordinate & big_corner, double radius) const = 0;


   public:
    // True if there is any position in the region where a spherical
    // particle of the given size can be in a valid position.
    //
    // False if radius is negative.
    //
    // * NOTE: for periodic systems the sphere diameter must be 
    // less than the periodicity in the periodic dimensions.
    bool fits(double radius) const
    {
      return radius >= 0.0 and this->do_fits( radius );
    }

   private:
    // True if there is any position in the region where a spherical
    // particle of the given size can be in a valid position.
    //
    // False if radius is negative.
    //
    // * NOTE: for periodic systems the sphere diameter must be 
    // less than the periodicity in the periodic dimensions.
    virtual bool do_fits(double radius) const = 0;


   public:
    // Calculate if a sphere centred at the given point and with
    // the given radius is in a valid position within the region.
    //
    // false if r < 0
    // not fits(r) --> not is_inside(..., r)
    //
    // * NOTE: Key point is that this returns whether the position
    // is a valid position in the region. For periodic systems
    // this implies the radius must be ignored in the periodic
    // dimension(s).
    
    bool is_inside(const coordinate & pos, double radius) const
    {
      return this->fits( radius ) and this->do_is_inside( pos, radius );
    }


   private:
    // Calculate if a sphere centred at the given point and with
    // the given radius is in a valid position within the region.
    //
    // not fits(r) --> not is_inside(..., r)
    //
    // * NOTE: Key point is that this returns whether the position
    // is a valid position in the region. For periodic systems
    // this implies the radius must be ignored in the periodic
    // dimension(s).
    
    virtual bool do_is_inside(const coordinate & pos, double radius) const = 0;


   public:
    // Get the region label
    std::string const& label() const
    {
      return this->label_;
    }

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
    
    boost::shared_ptr< grid_generator > make_gridder(utility::random_distribution & rgenr, std::size_t count) const
    {
      return this->do_make_gridder( rgenr, (count == 0 ? 1 : count) );
    }


   private:
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
    
    virtual boost::shared_ptr< grid_generator > do_make_gridder(utility::random_distribution & rgenr, std::size_t count) const = 0;


   public:
    // Create a gridder object for this system. The gridder inter-node
    // distance is guarranteed to have the given minimum spacing.
    // Every node is also guarranteed to be a minimum of (spacing/2)
    // distant from any side.  The gridder provides the positions
    // in a random sequence.
    //
    // \pre fits(spacing/2)
    //
    // The spacing may be increased slightly to more evenly fit the
    // particles into the region. The gridder calculates the maximum
    // number of nodes that can fit within the region and have the
    // given spacing. It then relaxes the grid to evenly fit in the
    // region and recalculates the spacing.
    
    boost::shared_ptr< grid_generator > make_gridder(double spacing, utility::random_distribution & rgenr) const
    {
      UTILITY_REQUIRE( this->fits( spacing / 2 ), "Requested inter-node spacing is too large for region." );
      return this->do_make_gridder( spacing, rgenr );
    }

   private:
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
    
    virtual boost::shared_ptr< grid_generator > do_make_gridder(double spacing, utility::random_distribution & rgenr) const = 0;


   public:
    // Calculate new random position for particle of the given radius to
    // be in a valid position in the region.  The position distribution
    // function should be equivolumetric.
    //
    // \pre fits(radius)
    // \post is_inside(result, radius)
    coordinate new_position(utility::random_distribution & rgnr, double radius) const;


   private:
    // Calculate new random position for particle of the given radius to
    // be inside the system.
    virtual coordinate do_new_position(utility::random_distribution & rgnr, double radius) const = 0;


   public:
    // Calculate new random position in a sphere centred on pos and
    // assign to pos. The particle is displaced a uniform linear
    // distance in a random direction. This means that it is not an
    // equivolume distribution (unlike new_position)
    //
    // * NOTE: for periodic systems the position should be 
    // automatically wrapped to be inside the system and
    // so should always return true.
    //
    // \pre is_inside(pos, radius) and fits(radius)
    // \return is_inside(pos, radius)
    
    bool new_position_offset(utility::random_distribution & rgnr, coordinate & pos, double offset, double radius) const;

    // Calculate new random position in a sphere centred on pos and
    // assign to pos. The particle is displaced a uniform linear
    // distance in a random direction. This means that it is not an
    // equivolume distribution (unlike new_position)
    //
    // pos.x += r cos(theta) sin(phi)
    //
    // pos.y += r sin(theta) sin(phi)
    //
    // pos.z += r cos(phi)
    
    static void do_offset(utility::random_distribution & rgnr, coordinate & pos, double offset);


   private:
    // Try to bring new position into region.
    //
    // \return is_inside(pos, radius)
    virtual bool do_new_position_wrap(coordinate & pos, double radius) const = 0;


   public:
    // Set the region label
    void set_label(std::string label)
    {
      this->label_ = label;
    }

    // Calculate volume accessible to centre-points of spherical particles
    // with the given radius. 
    //
    // \pre fits(radius)
    double volume(double radius) const
    {
      UTILITY_REQUIRE( this->fits( radius ), "Sphere must fit in space to be able to calculate the volume." );
      return this->do_volume( radius );
    }


   private:
    // Calculate volume accessible to centre-points of spherical particles
    // with the given radius. 
    //
    // NOTE: can assume fits(radius)
    virtual double do_volume(double radius) const = 0;


   public:
    // Add an input file section to wr.
    //
    // Output of this method is something like
    //
    // region
    // name [label]
    // <call do_write_input_section>
    // end
    
    void write_document(core::input_document & wr) const;


   private:
    // Update wr[ix] section with information from the derived class.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const = 0;


};

} // namespace geometry
#endif
