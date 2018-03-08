#ifndef IONCH_EVALUATOR_ICC_FORTRAN_HPP
#define IONCH_EVALUATOR_ICC_FORTRAN_HPP

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

#include "evaluator/base_evaluator.hpp"
#include <boost/serialization/vector.hpp>
#include "utility/multi_array.hpp"
#include <string>
#include <cstddef>
#include "utility/unique_ptr.hpp"
#include <iostream>

namespace evaluator { class evaluator_meta; } 
namespace core { class input_parameter_memo; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace particle { class change_set; } 
namespace evaluator { class evaluator_manager; } 
namespace core { class input_document; } 
namespace geometry { class membrane_cylinder_region; } 
namespace geometry { class coordinate; } 

namespace evaluator {

// Induced Charge Computation evaluator (fortran).
//
// Evaluate the interaction between dielectric boundary and ensemble of
// charges. This is done by calculating the surface charge on the
// dielectric surface from the external electric field and then calculating
// the Coulomb interactions between the surface charge and the ensemble
// of charges.
// 
// NOTES
// * We assume constant permittivity in solvent.
// * We assume the dielectric boundary has a fixed geometry during
//   the simulation.
class icc_fortran : public base_evaluator
 {
   private:
    //The c vector
    std::vector<double> c_;

    //The changed C vector from latest trial
    mutable std::vector<double> cnew_;

    //The h vector
    std::vector<double> h_;

    //The changed h vector from the latest trial
    mutable std::vector<double> hnew_;

    //The stored particle-patch distances
    boost::multi_array<double, 2> rip_;

    //The trial particle-patch distances
    mutable std::vector<std::vector< double >> ripnew_;

    // filename for storing the A matrix (default : amx.XXX)
    //
    // NOTE: boost text archive format may exist with filename (amx.XXX.ar)
    //
    // file format:
    // 1 # file version
    // npchsz # dimension -> ( npchsz x npchsz )
    // i1 i2 value # indices and entry
    // ... 
    std::string a_matrix_filename_;

    // filename for storing the tile definition (patch grid) (default : patch.XXX)
    //
    // NOTE: boost text archive format may exist with filename (patch.XXX.ar)
    //
    // file format:
    // 1 { 2 }  # format version
    // npchsz # number of patches
    // prx pry prz area pux puy puz deps { c1x c1y c1z c2x c2y c2z c3x c3y c3z c4x c4y c4z } 
    // ...
    std::string patch_filename_;

    // Scale factor = kT . echg / (4 * pi * eps0 * angstrom * epsw )
    //
    //NOTE : We assume constant permittivity in solvent.
    double alfa_;

    // Solvent permittivity 
    double epsw_;

    //Value of potential for the last accepted configuration.
    double old_potential_;

    //Value of potential for current test configuration.
    mutable double new_potential_;

    // PARAMETER: The permittivity of the protein
    //
    // DEFAULT : 10
    double epspr_;

    // PARAMETER: The outer radius of the protein 
    //
    // PR > rgn.channel_radius + rgn.arc_radius + membrane_arc_radius and PR < rgn.radius
    //
    // DEFAULT:  rgn.channel_radius + rgn.arc_radius + membrane_arc_radius
    double protein_radius_;

    // PARAMETER: arc radius of toroid inside the membrane
    //
    // DEFAULT: rgn.arc_radius
    double membrane_arc_radius_;

    //  The desired tile size for lines parallel or radial to the z-axis.
    double dx_zaxis_;

    //  The desired tile size to split up the circumferences of circles that are 
    //  centred on the z-axis.
    double dx_radial_;

    // Number of sub-tiles in each dimension when integrating between
    // two separate tiles
    std::size_t nsub0_;


   public:
    icc_fortran();


   private:
    //no copy
    icc_fortran(const icc_fortran & source) = delete;

    //no move
    icc_fortran(icc_fortran && source) = delete;


   public:
    virtual ~icc_fortran()
    {}


   private:
    //no assignment
    icc_fortran & operator=(const icc_fortran & source) = delete;


   public:
    // Create and add a evaluator_definition to the meta object.
    static void add_definition(evaluator_meta & meta);

    //Set up the evaluator using the given map of name/value pairs
    //
    //Valid only for channel_system simtypes. Accepts no input parameters.
    static std::unique_ptr< base_evaluator > make_evaluator(const std::vector< core::input_parameter_memo >& param_set);



  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< base_evaluator >(*this);
       ar & c_;
       ar & cnew_;
       ar & h_;
       ar & hnew_;
       ar & a_matrix_filename_;
       ar & patch_filename_;
       ar & alfa_;
       ar & epsw_;
       ar & old_potential_;
       ar & new_potential_;
       ar & epspr_;
       ar & protein_radius_;
       ar & membrane_arc_radius_;
       ar & dx_zaxis_;
       ar & dx_radial_;
       ar & nsub0_;
    }


   public:
    // UNIFIED ICC ENERGY
    // 
    // This method calculates the change in coulombic potential energy between
    // the particles and the surface patches.
    
    void compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const;

    //Calculate the total potential energy.
    virtual double compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const override;

    // Prepare the evaluator for running a simulation loop set.
    virtual void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman) override;

    static std::string amx_file_label()
    {
      return "amx_file";
    }


   private:
    // Log message descibing the evaluator and its parameters
    virtual void do_description(std::ostream & os) const override;

    // Write evaluator specific content of input file section.
    //
    // only throw possible should be from os.write() operation
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    bool empty() const
    {
       return this->h_.empty();
    }

    //Field experienced normal to a given surface element
    double field(std::size_t idx) const
    {
       return this->c_[ idx ];
    }

    // Filename to save A matrix data. If empty no data is saved separately.
    const std::string get_a_matrix_filename() const;

    // Get the patch/tile delta to divide up the circumference
    // of circles normal to and centred on the Z-axis
    double get_alfa() const
    {
       return this->alfa_;
    }

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

    // Filename to save patch data. If empty no data is saved separately.
    const std::string get_patch_filename() const;

    // Get the relative permittivity of the solvent media
    double get_permittivity() const
    {
       return this->epsw_;
    }

    // Get the relative permittivity of the protein media
    double get_protein_permittivity() const
    {
      return this->epspr_;
    }

    // Get the radius of the outer part of the protein. It should be less than the cell radius but
    // greater than the sum of the channel radius and the vestibule and membrane
    // arc radii.
    double get_protein_radius() const
    {
      return this->protein_radius_;
    }

    // Get the arc radius of the protein toroid within the membrane.
    double get_membrane_arc_radius() const
    {
      return this->membrane_arc_radius_;
    }

    static std::string patch_file_label()
    {
      return "patch_file";
    }

    //Induced charge on a given surface element
    double surface_charge(std::size_t idx) const;

    //Area of a given surface element
    double surface_area(std::size_t idx) const;

    // Total area of all surface elements
    double surface_area() const;

    // H array value on a given surface element
    double surface_value(std::size_t idx) const
    {
       return this->h_[ idx ];
    }

    std::size_t size() const
    {
       return this->h_.size();
    }

    virtual std::string type_label() const override
    {
      return icc_fortran::type_label_();
    }
    static std::string type_label_();

    // Write surface tile and A matrix data to files
    void write_data() const;

    //Compute the initial C and H matrices
    void compute_initial_c_h(const particle::particle_manager & pman);

    //Compute the total surface area and charge
    void compute_total_surface_charge(double & charget, double & areat) const;

    // Create the  matrix, this involves defining the patch grid,
    // integrating the grid to generate the A' matrix then inverting
    // this matrix to get the A matrix.
    //
    // Use the input geometry to define the information that the
    // 'patch' module will use to generate a set of 'patches' that
    // cover the protein surface.
    //
    // fortran equiv patch::defgrd + patch::matrix
    //  see ionch::iccgrid::add_XXXX for patch::goXXXX
    //  see ionch::XXXX_integrator types for patch::intXXXX
    void create_amx(const geometry::membrane_cylinder_region & rgn);

    // Create the  matrix, this involves defining the patch grid,
    // integrating the grid to generate the A' matrix then inverting
    // this matrix to get the A matrix.
    //
    // Use the input geometry to define the information that the
    // 'patch' module will use to generate a set of 'patches' that
    // cover the protein surface.
    //
    // fortran equiv patch::defgrd + patch::matrix
    //  see ionch::iccgrid::add_XXXX for patch::goXXXX
    //  see ionch::XXXX_integrator types for patch::intXXXX
    void create_amx(double zl1, double rl1, double rlvest);

    //  Called after the result of the trial is known.
    void on_conclude_trial(const particle::change_set & changes) override;

    // Set filename to save A matrix information. If not set then no data is saved.
    void set_a_matrix_filename(std::string value);

    //  Set the alfa scaling factor:
    //  alfa = k{temperature} . echg / (4 * pi * eps0 * angstrom * {permittivity} )
    //
    // \param temperature : in kelvin
    // \param permittivity : relative permittivity.
    void set_alfa(double temperature);

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

    // Set the arc radius of the protein toroid within the membrane.
    void set_membrane_arc_radius(double val)
    {
      this->membrane_arc_radius_ = val;
    }

    // Set the radial delta 
    //
    // \pre empty
    void set_nsub0(std::size_t val);

    // Set filename to save patch information. If not set then no patch
    // data is saved.
    void set_patch_filename(std::string value);

    // Set the relative permittivity of the solvent media
    void set_permittivity(double val)
    {
      this->epsw_ = val;
    }

    // Set the relative permittivity of the protein media
    void set_protein_permittivity(double val)
    {
      this->epspr_ = val;
    }

    // Set the radius of the outer part of the protein. It should be less than the cell radius but
    // greater than the sum of the channel radius and the vestibule and membrane
    // arc radii.
    void set_protein_radius(double val)
    {
      this->protein_radius_ = val;
    }

    //  Dump data from current object
    void dump(std::string fname) const;

    // Get the patch centre point and normal
    void get_patch_coordinate(std::size_t idx, geometry::coordinate & pos, geometry::coordinate & norm);

    double gaussbox() const;


};

} // namespace evaluator
#endif
