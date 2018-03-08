#ifndef IONCH_GEOMETRY_MEMBRANE_CELL_INSERTER_HPP
#define IONCH_GEOMETRY_MEMBRANE_CELL_INSERTER_HPP

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

#include "geometry/coordinate.hpp"

// manual includes
#include "core/constants.hpp"
// -
namespace utility { class random_distribution; } 

namespace geometry {

class membrane_cell_inserter
 {
   private:
    // The radius this data object is for.
    double radius_;

    // Cell radius less specie radius. ( R - r )
    double reduced_cell_radius_;

    // Volume factor for outer compartments
    //
    // = ( R - r )^2.pi.(L/2 - 2r)
    double compartment_factor_;

    // The offset of all Z positions within the cell compartment
    //
    //  Lc + r
    double cell_offset_;

    //The channel radius reduced by sphere radius
    //
    // Rc - r
    double reduced_channel_radius_;

    // Volume factor for channel
    //
    // (Rc - r)^2.pi.(Lc/2 - A)
    double inner_channel_factor_;

    // Volume factor for vestibule
    //
    // volume_of_rotation( Rc + A, A + r )
    double vestibule_factor_;

    //The radius of the centre point of the vestibule quarter toroid
    //
    // Rc + A
    double vestibule_outer_radius_;

    //The radius of the vestibule toroid including the object 
    //radius.
    //
    // A + r
    double vestibule_radius_;

    //The offset of the first position within the vestibule
    //
    // Lc - A
    double vestibule_offset_;


   public:
    // For serialization
    membrane_cell_inserter() = default;

    membrane_cell_inserter(double radius, double cell_radius, double cell_hlength, double channel_radius, double channel_hlength, double arc_radius);

    ~membrane_cell_inserter() = default;

    membrane_cell_inserter(membrane_cell_inserter && source) = default;

    membrane_cell_inserter(const membrane_cell_inserter & source) = default;

    membrane_cell_inserter & operator=(membrane_cell_inserter source)
    {
      this->swap( source );
      return *this;
    }

    void swap(membrane_cell_inserter & source);


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & radius_;
      ar & reduced_cell_radius_;
      ar & compartment_factor_;
      ar & cell_offset_;
      ar & reduced_channel_radius_;
      ar & inner_channel_factor_;
      ar & vestibule_factor_;
      ar & vestibule_outer_radius_;
      ar & vestibule_radius_;
      ar & vestibule_offset_;
    }


   public:
    bool compare_equal(const membrane_cell_inserter & source) const;

    bool compare_equal(double radius) const
    {
      return radius == this->radius_;
    }

    friend bool operator<(const membrane_cell_inserter & lhs, const membrane_cell_inserter & rhs)
    {
      return lhs.radius_ < rhs.radius_;
    }

    friend bool operator<(const membrane_cell_inserter & lhs, double rhs)
    {
      return lhs.radius_ < rhs;
    }

    friend bool operator<(double lhs, const membrane_cell_inserter & rhs)
    {
      return lhs < rhs.radius_;
    }

    friend bool operator==(const membrane_cell_inserter & lhs, const membrane_cell_inserter & rhs)
    {
      return lhs.compare_equal( rhs );
    }

    friend bool operator==(double lhs, const membrane_cell_inserter & rhs)
    {
      return rhs.compare_equal( lhs );
    }

    friend bool operator==(const membrane_cell_inserter & lhs, double rhs)
    {
      return lhs.compare_equal( rhs );
    }

    // Generate a random position uniformly in this space.
    coordinate generate_position(utility::random_distribution & rgnr);

    // Calculate the volume of rotation _underneath_ an arc between z_0
    // and z_1.
    //
    // This calculates the volume of rotation underneath an arc whose
    // centrepoint is (axial_disp,radial_disp) and has the given arc_radius.
    // The volume is the slice between the z_0 and z_1 .
    //
    // \pre radial_disp > arc_radius
    
    static double volume_of_rotation(double axial_disp, double radial_disp, double arc_radius, double z_0, double z_1);

    // Calculate the volume of rotation _underneath_ an arc whose
    // centrepoint is (0.0,radial_disp) and has the given arc_radius.
    // The volume is the slice between the 0.0 and arc_radius. In
    // other words it is the volume under the half arc.
    //
    // \pre radial_disp > arc_radius
    
    static double volume_of_rotation(double radial_disp, double arc_radius)
    {
      return volume_of_rotation( 0.0, radial_disp, arc_radius, 0.0, arc_radius );
    }

    // Get the radius this is for
    double radius() const
    {
      return this->radius_;
    }

    // Get the radius this is for
    double reduced_cell_radius() const
    {
      return this->reduced_cell_radius_;
    }

    // Get the radius this is for
    double compartment_factor() const
    {
      return this->compartment_factor_;
    }

    // Get the radius this is for
    double cell_offset() const
    {
      return this->cell_offset_;
    }

    // Get the radius this is for
    double reduced_channel_radius() const
    {
      return this->reduced_channel_radius_;
    }

    // Get the radius this is for
    double inner_channel_factor() const
    {
      return this->inner_channel_factor_;
    }

    // Get the radius this is for
    double vestibule_factor() const
    {
      return this->vestibule_factor_;
    }

    // Get the radius this is for
    double vestibule_outer_radius() const
    {
      return this->vestibule_outer_radius_;
    }

    // Get the radius this is for
    double vestibule_radius() const
    {
      return this->vestibule_radius_;
    }

    // Get the radius this is for
    double vestibule_offset() const
    {
      return this->vestibule_offset_;
    }


};

} // namespace geometry
#endif
