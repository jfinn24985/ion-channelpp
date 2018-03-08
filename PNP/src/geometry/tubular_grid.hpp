#ifndef IONCH_GEOMETRY_TUBULAR_GRID_HPP
#define IONCH_GEOMETRY_TUBULAR_GRID_HPP

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

#include <boost/serialization/vector.hpp>
#include <cstddef>
#include "geometry/coordinate.hpp"
#include "utility/archive.hpp"

#include "geometry/grid.hpp"

namespace utility { class random_distribution; } 

namespace geometry {

// 2D grid in circle
class circle_grid
 {
   private:
    // inter-grid spacing
    double spacing_;

    // The number of x grid elements at a particular y
    std::vector<std::size_t> width_;

    // The number of elements in an xy slice
    std::size_t xy_size_;


   public:
    // Initialise the tubular grid with the given length and radius.
    //
    // This leaves a minimum of 'spacing' distance between any grid
    // point and the cylinder edge. Will have a grid point at origin (0,0,0)
    // or no points.
    //
    //\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
    //\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
    //\param spacing : No two grid points will be less than spacing apart and closer than spacing/2 to cylinder bounds.
    //\param shuffle : whether to provide grid points in linear (false)
    //   or random (true) order
    circle_grid(double radius, double spacing);

    // Default ctor (for serialize only)
    circle_grid()
    : spacing_(0.0)
    , width_()
    , xy_size_(0)
    {}
    circle_grid(const circle_grid & source)
    : spacing_( source.spacing_ )
    , width_( source.width_ )
    , xy_size_( source.xy_size_ )
    {}

    circle_grid(circle_grid && source)
    : spacing_( std::move( source.spacing_ ) )
    , width_( std::move( source.width_ ) )
    , xy_size_( std::move( source.xy_size_ ) )
    {}

    circle_grid & operator=(circle_grid source)
    {
      this->swap( source );
      return *this;
    }

    ~circle_grid() = default;

    void swap(circle_grid & source)
    {
      std::swap( spacing_, source.spacing_ );
      std::swap( width_, source.width_ );
      std::swap( xy_size_, source.xy_size_ );
    }

    // The number of grid points in the circle
    std::size_t size() const
    {
      return this->xy_size_;
    }

    // Set the  pnt.x and pnt.y based on the given index 
    // into the circle grid
    //
    // @require index < size
    void set_xy(coordinate & pnt, std::size_t index) const;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & spacing_;
        ar & width_;
        ar & xy_size_;
      };


};
// Generate a cubic grid of points within a
// cylinder/tube. The grid starts a minimum of half
// the 'spacing' distance from the cylinder walls
// and end. Grid points are retrieved sequentially
// or in random order.
//
// NOTES:
//
// * cylinder axis is along z direction with centre of the cylinder at z=0.
//
// * cylinder circle is in xy plane
// 
// * z-grid equally spaced in positive and negative direction
// 
// * absolute minimum value in z direction = spacing/2
//
// * absolute minimum in xy plane is (0,0)
//
// Given tubelength and radius to generate the grid:
//
// * absolute maximum in z direction is <= (tubelength/2 - spacing/2)
//
// * absolute maximum in xy place sqrt(x*x + y*y) < (radius - spacing/2)

class tubular_grid : public grid_generator
 {
   private:
    // inter-grid spacing
    double spacing_;

    //  Minimum value the spacing can reach when attempting
    //  to build a grid with a target number of positions.
    static const double min_spacing;

    // maximum grid widths in direction of tube
    std::size_t zmax_;

    //  The XY circle grid slice.
    circle_grid circle_;

    // Grid point selector array
    std::vector<std::size_t> selector_;


   public:
    // Initialise the tubular grid with the given length and radius.
    //
    // This leaves a minimum of 'spacing' distance between any grid
    // point and the cylinder edge. Point closest to origin
    // will be (0,0,+/-spacing/2) or no points.
    //
    //\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
    //\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
    //\param spacing : No two grid points will be less than spacing apart and closer than spacing/2 to cylinder bounds.
    tubular_grid(double tubelength, double radius, double spacing);

    // Initialise the tubular grid with the given length and radius. Vary grid spacing until
    // we have approximately npart grid points.
    //
    // This leaves a minimum of 'spacing' distance between any grid
    // point and the cylinder edge. Point closest to origin
    // will be (0,0,+/-spacing/2) or no points.
    //
    //\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
    //\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
    //\param npart : Target number of grid points.
    tubular_grid(double tubelength, double radius, std::size_t npart);

    // Initialise the tubular grid with the given length and radius. Vary grid spacing until
    // we have approximately npart grid points.
    //
    // This leaves a minimum of 'spacing' distance between any grid
    // point and the cylinder edge. Point closest to origin
    // will be (0,0,+/-spacing/2) or no points.
    //
    //\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
    //\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
    //\param npart : Target number of grid points.
    tubular_grid(double tubelength, double radius, int npart)
    : tubular_grid( tubelength, radius, std::size_t( npart ) )
    {}

    // Initialise the tubular grid with the given length and radius.
    //
    // This leaves a minimum of 'spacing' distance between any grid
    // point and the cylinder edge. Point closest to origin
    // will be (0,0,+/-spacing/2) or no points.
    //
    //\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
    //\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
    //\param spacing : No two grid points will be less than spacing apart and closer than spacing/2 to cylinder bounds.
    //\param shuffler : object to randomly reorder the access sequence.
    tubular_grid(double tubelength, double radius, double spacing, utility::random_distribution & shuffler);

    // Initialise the tubular grid with the given length and radius. Vary grid spacing until
    // we have approximately npart grid points.
    //
    // This leaves a minimum of 'spacing' distance between any grid
    // point and the cylinder edge. Point closest to origin
    // will be (0,0,+/-spacing/2) or no points.
    //
    //\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
    //\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
    //\param npart : Target number of grid points.
    tubular_grid(double tubelength, double radius, int npart, utility::random_distribution & ranf)
    : tubular_grid( tubelength, radius, std::size_t( npart ), ranf )
    {}

    // Initialise the tubular grid with the given length and radius. Vary grid spacing until
    // we have approximately npart grid points.
    //
    // This leaves a minimum of 'spacing' distance between any grid
    // point and the cylinder edge. Point closest to origin
    // will be (0,0,+/-spacing/2) or no points.
    //
    //\param tubelength : Z coordinates will be within | -(tubelength-spacing)/2 <= z <= (tubelength-spacing)/2 |
    //\param radius : XY coordinates will satisfy sqrt(x*x + y*y) <= radius - (spacing/2)
    //\param npart : Target number of grid points.
    //\param shuffler : object to randomly reorder the access sequence.
    tubular_grid(double tubelength, double radius, std::size_t npart, utility::random_distribution & shuffler);


   protected:
    // Default ctor (for serialize only)
    tubular_grid() = default;

   public:
    virtual ~tubular_grid();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< geometry::grid_generator >(*this);
        ar & spacing_;
        ar & zmax_;
        ar & circle_;
        ar & selector_;
      };


   public:
    // The number of remaining points in the grid.
    bool empty() const override
    {
      return this->selector_.empty();
    }

    // Get the next location from the grid.
    //
    // @require size > 0
    bool next(coordinate & pnt);

    // Get the next location from the grid.
    //
    // @require size > 0
    coordinate next()
    {
      coordinate Result;
      this->next (Result);
      return Result;
    }

    // The number of remaining points in the grid.
    std::size_t size() const
    {
        return this->selector_.size();
    }

    //  The inter-gridpoint distance.
    virtual double spacing() const override
    {
      return this->spacing_;
    }


};
//  Adapt the standard tubular grid by splitting the tube into two
//  equal parts and inserting a non-grid space in between. This
//  inserts the zoffset space into the z coordinate output by the
//  gridder.
//

class split_tube_grid : public tubular_grid
 {
   private:
    // The membrane half width.
    double zoffset_;


   public:
    // Create a cubic grid of points within a bisected tube of the given 
    // half length, bisection offset and radius. NOTE: the outer extent
    // of the tube is offset + length/2 in -ve and +ve z direction.
    //
    // NOTES:
    //
    // * Grid z coordinate will be either +ve or -ve
    //
    // * Z coordinate range offset < |z| < length/2 + offset
    //
    // * XY coordinates in circle of given radius
    //
    // * Sequence of coordinates generated by next are in random
    // a order.
    //
    
    split_tube_grid(double offset, double length, double radius, std::size_t npart, utility::random_distribution & ranf);

    // Create a cubic grid of points within a bisected tube of the given 
    // half length, bisection offset and radius.  NOTE: the outer extent
    // of the tube is -/+(offset + length/2) in z direction.
    //
    // NOTES:
    //
    // * Grid z coordinate will be either +ve or -ve
    //
    // * Z coordinate range offset < |z| < length/2 + offset
    //
    // * XY coordinates in circle of given radius
    //
    // * Sequence of coordinates generated by next are sequential
    // in z.
    //
    
    split_tube_grid(double offset, double length, double radius, std::size_t npart);

    // Create a cubic grid of points within a bisected tube of the given 
    // half length, bisection offset and radius. NOTE: the outer extent
    // of the tube is +/-(offset + length/2) in z direction.
    //
    // NOTES:
    //
    // * Grid z coordinate will be either +ve or -ve
    //
    // * Z coordinate range offset < |z| < length/2 + offset
    //
    // * XY coordinates in circle of given radius
    //
    // * Sequence of coordinates generated by next are in random
    // a order.
    //
    
    split_tube_grid(double offset, double length, double radius, int npart, utility::random_distribution & ranf);

    // Create a cubic grid of points within a bisected tube of the given 
    // half length, bisection offset and radius.  NOTE: the outer extent
    // of the tube is +/-(offset + length/2) in z direction.
    //
    // NOTES:
    //
    // * Grid z coordinate will be either +ve or -ve
    //
    // * Z coordinate range offset < |z| < length/2 + offset
    //
    // * XY coordinates in circle of given radius
    //
    // * Sequence of coordinates generated by next are sequential
    // in z.
    //
    
    split_tube_grid(double offset, double length, double radius, int npart);

    // Create a cubic grid of points within a bisected tube of the given 
    // half length, bisection offset and radius. NOTE: the outer extent
    // of the tube is +/-(offset + length/2) in z direction.
    //
    // NOTES:
    //
    // * Grid z coordinate will be either +ve or -ve
    //
    // * Z coordinate range offset < |z| < length/2 + offset
    //
    // * XY coordinates in circle of given radius
    //
    // * Sequence of coordinates generated by next are in random
    // a order.
    //
    
    split_tube_grid(double offset, double length, double radius, double spacing, utility::random_distribution & ranf);

    // Create a cubic grid of points within a bisected tube of the given 
    // half length, bisection offset and radius.  NOTE: the outer extent
    // of the tube is +/-(offset + length/2) in z direction.
    //
    // NOTES:
    //
    // * Grid z coordinate will be either +ve or -ve
    //
    // * Z coordinate range offset < |z| < length/2 + offset
    //
    // * XY coordinates in circle of given radius
    //
    // * Sequence of coordinates generated by next are sequential
    // in z.
    //
    
    split_tube_grid(double offset, double length, double radius, double spacing);


   private:
    // Default ctor (for serialize)
    split_tube_grid() = default;


   public:
    virtual ~split_tube_grid() {}

    // Get the next location from the grid.
    //
    // @require size > 0
    bool next(coordinate & pnt) override;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
       ar & boost::serialization::base_object< geometry::tubular_grid >(*this);
        ar & zoffset_;
      }


};

} // namespace geometry
#endif

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY(geometry::tubular_grid);
BOOST_CLASS_EXPORT_KEY(geometry::split_tube_grid);