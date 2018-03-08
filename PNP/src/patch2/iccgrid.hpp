#ifndef IONCH_PATCH_ICCGRID_HPP
#define IONCH_PATCH_ICCGRID_HPP


#include "utility/multi_array.hpp"
#include <cstddef>
#include <iostream>

namespace patch {

class iccgrid
 {
   private:
    double dx_zaxis_;

    double dx_radial_;

    int nsub0_;


   public:
    enum 
     {
      iX =  0
    // 1: Y coordinate of a patch
	,
      iY =  1
    // 2: Z coordinate of a patch
	,
      iZ =  2
    // 3: X dimension of normal vector to centre of patch
	,
      iUX =  3
    // 4: Y dimension of normal vector to centre of patch
	,
      iUY =  4
    // 5: Z dimension of normal vector to centre of patch
	,
      iUZ =  5
    // 6: Surface area of a patch
	,
      iArea =  6
    // 7: The effective dielectric constant on the outside of a patch
	,
      iDEps =  7
      

    };


   private:
    boost::multi_array< double, 2 > tiles_;

    // Number of grid points/tiles
    std::size_t npch_;

    // Allocated array size
    std::size_t max_size_;


   public:
    // ----------------------------------------
    // Number of tiles around cylinder section
    //
    // DIFF use sqrt(radius) when calculating the 
    // "circumference" to get fewer tiles further from the 
    // centre. In all cases minimum is returned if greater
    // than that calculated.
    //
    //   n = 2*pi* {sqrt(r)} /dxf
    //
    // post: result >= minimum
    inline int cylindrical_tile_count(double radius, int minimum) const {
          return std::max< int >(std::nearbyint((2 * constants::pi() * std::sqrt(radius))/this->dx_radial_), minimum);
        };

    // Number of tiles along z-axis of cylinder section
    //
    // DIFF use sqrt(radius) when calculating the 
    // "circumference" to get fewer tiles further from the 
    // centre. In all cases minimum is returned if greater
    // than that calculated.
    //
    // post: result >= minimum
    inline int line_tile_count(double length, int minimum) const {
          return std::max< int >(std::nearbyint(length / this->dx_zaxis_), minimum);
        };

    // Number of tiles around arc slice section
    //
    // This is the number of slices the arc is split up
    // into around the curve parallel to z axis
    //
    // Arc is 1/4 circle. 
    //   n = 2*pi*r/dxf * 1/4
    //
    // @param radius: Radius of the arc segment parallel to
    //               Z axis
    // @param minimum: The smallest number of tiles
    //
    // @post: result >= minimum
    inline int arc_tile_count(double radius, int minimum) const {
          return std::max< int > (std::nearbyint((constants::pi() * radius)/(2 * this->dx_zaxis_)), minimum);
        };

    // Number of sub-tiles in each dimension when integrating between
    // two separate tiles
    inline int nsub_other() const { return this->nsub0_; };

    // Number of sub-tiles in each dimension when integrating a tile
    // with itself
    inline int nsub_self() const { return this->nsub0_ * 5; };

    iccgrid(double dxw, double dxf, int nsub, int max_size);

    ~iccgrid();

    // ----------------------------------------
    // Calculate tiles along a cylinder, arc or wall
    //
    // @param a_zleft : the z-value of left end of cylinder
    // @param a_zright : the z-value of right end of cylinder
    // @param a_radius : the radius of the cylinder
    // @param a_deps : the change in permitivity across boundary
    // @param a_z_min : the minimum number of tile in the z direction
    //       (Fortran ionch used 10 for inner and ?4? for outer
    // @param is_internal : whether the tiles face towards (true) the
    //       z-axis or away. 
    void add_line(std::ostream & a_os, double a_zleft, double a_zright, double a_radius, double a_deps, int a_z_min, bool is_internal);

    void add_arc(std::ostream & a_os, double za0, double ra0, double ra, double ta1, double ta2, double a_deps, int a_z_min, bool is_outer);

    void add_wall(std::ostream & a_os, double a_z, double a_rl1, double a_rl2, double a_deps, int min_phi, int min_rad, bool is_to_positive);

    // ----
    // integrate grid
    void generate_matrix(int file_id, boost::multi_array< double, 2 > & a_matrix);

    // -- grid point attributes
    inline double x(int jpatch) const { return this->tiles_[jpatch][iX]; };

    inline double y(int jpatch) const { return this->tiles_[jpatch][iY]; };

    inline double z(int jpatch) const { return this->tiles_[jpatch][iZ]; };

    inline double area(int jpatch) const { return this->tiles_[jpatch][iArea]; };

    inline double ux(int jpatch) const { return this->tiles_[jpatch][iUX]; };

    inline double uy(int jpatch) const { return this->tiles_[jpatch][iUY]; };

    inline double uz(int jpatch) const { return this->tiles_[jpatch][iUZ]; };

    inline double deps(int jpatch) const { return this->tiles_[jpatch][iDEps]; };

    // Number of grid points
    inline std::size_t size() const { return this->npch_; };


};

} // namespace patch
#endif
