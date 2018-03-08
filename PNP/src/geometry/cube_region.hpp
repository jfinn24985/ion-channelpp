#ifndef IONCH_GEOMETRY_CUBE_REGION_HPP
#define IONCH_GEOMETRY_CUBE_REGION_HPP

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

#include "geometry/non_periodic_region.hpp"
#include "geometry/coordinate.hpp"
#include <string>
#include <boost/serialization/shared_ptr.hpp>
#include <cstddef>
#include <boost/serialization/vector.hpp>

namespace geometry { class cubic_grid; } 
namespace geometry { class grid_generator; } 
namespace utility { class random_distribution; } 
namespace core { class input_document; } 
namespace geometry { class base_region; } 
namespace core { class input_parameter_memo; } 
namespace geometry { class region_meta; } 

namespace geometry {

// Non-periodic cube region. Can be open or closed.
//
// An open cube sees all objects whose centrepoint is inside
// the cube as being inside the cube. A closed cube sees only
// objects that are completely within the cube as being in the
// cube.
//
// The cube extent = [ origin.x : origin.x + length ],
// [origin.y : origin.y + length ], [ origin.z : origin.z + length ] 
class cube_region : public non_periodic_region
 {
   private:
    // The most negative position in the cube.
    coordinate origin_;

    // Length of the cube
    double length_;

    // Whether the cube is open or closed.
    bool is_open_;

    // For serialization only
    cube_region() = default;


   public:
    // Create cube with the given dimension.
    //
    // \param label : region label
    // \param len : the length of the cube.
    // \param origin : The most negative corner of the cube.
    // \param is_open : Whether cube is open of closed. (see class desc)
    cube_region(std::string label, double len, const coordinate & origin, bool is_open)
    : non_periodic_region( label )
    , origin_( origin )
    , length_( len )
    , is_open_( is_open )
    {}

    virtual ~cube_region();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object< geometry::base_region >(*this);
        ar & this->origin_;
        ar & this->length_;
        ar & this->is_open_;
    }

    // Adjust cube length to accomodate volume.
    //
    // if is_open()
    //  len = cuberoot( vol )
    // else
    //  len + 2 rad = cuberoot( vol )
    //
    // * NOTE: can never give: not fits(rad)
    virtual void do_change_volume(double vol, double rad);

    // Calculate a bounding box for the valid positions of
    // spheres of the given radius within the region. The
    // bounding box is defined by its most negative
    // (small_corner) and most positive (big_corner).
    //
    // \pre fits(radius)
    // \post big.(x|y|z) - small.(x|y|z) >= 0
    void do_extent(coordinate & small_corner, coordinate & big_corner, double radius) const override;

    // True if there is any position in the region where a spherical
    // particle of the given size can be in a valid position. 
    //
    // * NOTE: While a particle of any size can theoretically exist
    // anywhere in a periodic cube, particles that are larger than the cube
    // would occlude themselves and are therefore said not to fit.
    virtual bool do_fits(double radius) const;

    // Check if sphere at the given position and radius is inside
    // the cube. As all positions can be mapped inside the
    // cube, this method answers the question of whether the
    // position is inside the primary cube. 
    //
    // not fits(r) --> not is_inside(..., r)
    //
    // * NOTE: Does not consider periodicity but ignores the radius if
    // fits(radius) is true.
    bool do_is_inside(const coordinate & pos, double radius) const override;


   public:
    // Is this cube open or closed (see class desc)
    bool is_open() const
    {
      return this->is_open_;
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
    
    virtual boost::shared_ptr< grid_generator > do_make_gridder(double spacing, utility::random_distribution & rgenr) const;

    // Create a gridder object for this system. The gridder is
    // guarranteed to have the minimum given number of points.
    // The gridder provides the positions in a random sequence.
    //
    // As the grid is highly regular, the number of nodes will be an
    // integral function of the number of points in each dimension.
    // For a cube will always have the same number of nodes in
    // each dimension and so the total number will be the cube of a
    // whole number. The actual number of nodes will therefore be
    // the smallest solution to this node function that is larger
    // than count.
    
    virtual boost::shared_ptr< grid_generator > do_make_gridder(utility::random_distribution & rgenr, std::size_t count) const;

    // Calculate new random position for particle. IGNORES RADIUS
    //
    // * NOTE: For this periodic system all positions are valid and
    // so any new position should be anywhere in the cube. Limiting
    // particle to be all inside would not be the desired behavior. 
    
    virtual coordinate do_new_position(utility::random_distribution & rgnr, double radius) const override;


   public:
    // Get the radius
    double length() const
    {
      return this->length_;
    }
    // Set the radius
    void set_length(double val)
    {
      this->length_ = val;
    }

    // Get cube's origin point
    coordinate const& origin() const
    {
      return this->origin_;
    }


   private:
    // Calculate volume accessible to centre-points of spherical particles
    // with the given radius. For this periodic system this is always 
    // equivalent to volume().
    //
    // NOTE: can assume fits(radius)
    double do_volume(double radius) const override;
    // Update wr[ix] section with information from the derived class.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;

    // Factory method for creating cube_region objects from
    // input data.
    static boost::shared_ptr< base_region > region_factory(std::string label, const std::vector< core::input_parameter_memo > & params);


   public:
    // Create and add a region_definition to the meta object.
    static void add_definition(region_meta & meta);


   private:
    static std::string type_label()
    {
      const static std::string result( "cube" );
      return result;
    }


};

} // namespace geometry

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( geometry::cube_region );

#endif