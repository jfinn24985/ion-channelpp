#ifndef IONCH_EVALUATOR_ICC_SURFACE_GRID_HPP
#define IONCH_EVALUATOR_ICC_SURFACE_GRID_HPP

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

#include "utility/archive.hpp"

#include "evaluator/integrator.hpp"
#include <boost/serialization/vector.hpp>
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "geometry/coordinate_set.hpp"
#include <cstddef>
#include <iostream>
#include "utility/multi_array.hpp"

namespace evaluator {

// Convert 2D geometry into a surface of rotation as a
// dielectric boundary surface. Discretize the surface in
// preparation of computing a set of simultaneous equations
// for calculating the surface charge on a surface element
// based on the external electric field.
class icc_surface_grid
 {
   private:
    // Surface area of a patch
    std::vector<double> area_;

    // The effective dielectric constant on the outside of a patch
    std::vector<double> deps_;

    // The permittivity on the inside of the surface (ie negative
    // direction of surface normal)
    std::vector<double> eps_in_;

    // The permittivity on the outside of the surface (ie positive
    // direction of surface normal)
    std::vector<double> eps_out_;

    // Integrator functors/closures for each patch
    boost::ptr_vector<integrator> intgs_;

    // X dimension of normal vector to centre of patch 
    std::vector<double> ux_;

    // Y dimension of normal vector to centre of patch
    std::vector<double> uy_;

    // Z dimension of normal vector to centre of patch
    std::vector<double> uz_;

    // x, y, z coordinate of patch
    geometry::coordinate_set xyz_;

    //  The desired tile size for lines parallel or radial to the z-axis.
    double dx_zaxis_;

    //  The desired tile size to split up the circumferences of circles that are 
    //  centred on the z-axis.
    double dx_radial_;

    // Number of sub-tiles in each dimension when integrating between
    // two separate tiles
    std::size_t nsub0_;

    // Number of grid points/tiles
    std::size_t npatch_;

    // Allocated array size
    std::size_t npatch_alloc_;

    //  The version of the patch file this class understands
    static const std::size_t patch_file_version_;


   public:
    icc_surface_grid();


   private:
    // Deleted
    icc_surface_grid(const icc_surface_grid & source) = delete;

    // Deleted
    icc_surface_grid(icc_surface_grid && source) = delete;


   public:
    ~icc_surface_grid();

    // Deleted
    icc_surface_grid & operator=(const icc_surface_grid & source) = delete;

    // Write the tile definitions to disk.
    void write_grid(std::ostream & output) const;



  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
       ar & area_;
       ar & deps_;
       ar & eps_in_;
       ar & eps_out_;
       ar & intgs_;
       ar & ux_;
       ar & uy_;
       ar & uz_;
       ar & xyz_;
       ar & dx_zaxis_;
       ar & dx_radial_;
       ar & nsub0_;
       ar & npatch_;
       ar & npatch_alloc_;
    }


   public:
    double area(std::size_t jpatch) const
    {
       return this->area_[ jpatch ];
    }

    // Total area of all surface elements
    double surface_area() const;

    //  Get the set of patch x,y,z coordinates
    const geometry::coordinate_set& coordinates() const
    {
      return this->xyz_;
    }

    double deps(std::size_t jpatch) const
    {
       return this->deps_[ jpatch ];
    }
    

    // Number of grid points
    std::size_t empty() const
    {
       return this->npatch_ == 0;
    }

    // The permittivity on the inside of the surface (ie negative
    // direction of surface normal)
    double eps_in(std::size_t jpatch) const
    {
       return this->eps_in_[ jpatch ];
    }

    // The permittivity on the outside of the surface (ie positive
    // direction of surface normal)
    double eps_out(std::size_t jpatch) const
    {
       return this->eps_out_[ jpatch ];
    }

    // Do we have integrators available to be able to generate
    // the matrix?
    //
    // \return not intg_.empty and intg_.size = size
    bool have_integrators() const;

    // Get the patch/tile delta to divide up the circumference
    // of circles normal to and centred on the Z-axis
    double get_dxf() const
    {
       return this->dx_radial_;
    }

    // Get the patch/tile delta that divides lines and arcs that
    // are parallel or radial to the z-axis.
    double get_dxw() const
    {
       return this->dx_zaxis_;
    }

    // Get the subtiling factor that determines how many subtiles
    // a patch/tile is divided into during integration 
    std::size_t get_nsub0() const
    {
       return this->nsub0_;
    }

    // Number of grid points
    inline std::size_t size() const
    {
       return this->npatch_;
    }

    double ux(std::size_t jpatch) const
    {
       return this->ux_[ jpatch ];
    }

    double uy(std::size_t jpatch) const
    {
       return this->uy_[ jpatch ];
    }

    double uz(std::size_t jpatch) const
    {
       return this->uz_[ jpatch ];
    }
    

    // -- grid point attributes
    inline double x(std::size_t jpatch) const
    {
       return this->xyz_.x( jpatch );
    }

    double y(std::size_t jpatch) const
    {
       return this->xyz_.y( jpatch );
    }

    double z(std::size_t jpatch) const
    {
       return this->xyz_.z( jpatch );
    }

    // ----------------------------------------
    // Calculate tiles along a cylinder. When is_internal is false
    // the calculation of patches is changed to create larger
    // patches.  The changes are:
    //  dx = (is_internal ? dxf : dxw/sqrt(rl2))
    //  nzl = max( ( a_z_min ), int(dll/dx) + 1)
    //  nfil = max( a_r_min, int(cir/dx) + 1)
    //
    //
    // @param a_zleft : the z-value of left end of cylinder
    // @param a_zright : the z-value of right end of cylinder
    // @param a_radius : the radius of the cylinder
    // @param a_deps : the change in permitivity across boundary
    // @param a_z_min : the minimum number of tile in the z direction
    //       (Fortran ionch used 10 for inner and ?4? for outer
    // @param is_internal : whether the tiles face towards (true) the
    //       z-axis or away. 
    void add_line(double a_zleft, double a_zright, double a_radius, double a_epsin, double a_epsout, std::size_t a_z_min, std::size_t a_r_min, bool is_internal);

    //
    //
    // \param is_outer : is the media on the outer side of the
    //    arc or inner side. (For the four "corners" of the standard 
    //    toroid will be true.)
    void add_arc(double za0, double ra0, double ra, double ta1, double ta2, double a_epsin, double a_epsout, std::size_t a_z_min, std::size_t a_r_min, bool is_outer);

    //
    //
    //
    // \param is_to_positive : is the media on the more positive side of the wall?
    void add_wall(double a_z, double a_rl1, double a_rl2, double a_epsin, double a_epsout, std::size_t min_phi, std::size_t min_rad, bool is_to_positive);

    // Remove the tile definitions.
    void clear();

    //  After generating the A matrix the integrator objects are no-longer
    //  needed and, optionally, can be removed.
    void clear_integrators();

    // Integrate grid
    //
    // \pre not empty
    // \pre have_integrators
    void generate_matrix(boost::multi_array< double, 2 > & a_matrix);

    // Resize the allocated memory.
    void resize(std::size_t npatch);

    // Set the patch/tile delta to divide up the circumference
    // of circles normal to and centred on the Z-axis (only 
    // useful before generating A matrix)
    //
    // \pre empty
    void set_dxf(double val);
    // Set the patch/tile delta that divides lines and arcs that
    // are parallel or radial to the z-axis.  (only useful before 
    // generating A matrix) 
    //
    // \pre empty
    void set_dxw(double val);

    // Set the radial delta 
    //
    // \pre empty
    void set_nsub0(std::size_t val);


};

} // namespace evaluator
#endif
