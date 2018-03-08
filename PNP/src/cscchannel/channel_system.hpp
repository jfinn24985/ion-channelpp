#ifndef IONCH_ION_CHANNEL_CHANNEL_SYSTEM_HPP
#define IONCH_ION_CHANNEL_CHANNEL_SYSTEM_HPP

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

#include "platform/simulator.hpp"
#include <boost/serialization/vector.hpp>
#include <string>
#include <cstddef>
#include "utility/unique_ptr.hpp"
#include <iostream>
#include "geometry/cylinder_region.hpp"

namespace core { class input_delegater; } 
namespace geometry { class grid_generator; } 
namespace core { class input_document; } 
namespace geometry { class coordinate; } 
namespace geometry { class coordinate_set; } 

namespace ion_channel {

//Cell simulator with IONCH style pore geometry.
//
//NOTE: Regions as a concept are the domain of the geometry generator classes rather
//than this class. Initial version will follow the legacy settings.
class channel_system : public platform::simulator
 {
   public:
    enum
     {
      IZLIM = 0,// Region within user-defined subregion bounded by 'structural_length'. This may
      // be equal or smaller than IFILT region.
      IFILT = 1,// Region within the central cylinder of protein model.
      ICHAN = 2,// Region inside the protein model, including the central cylinder 
      // and entrances.
      IBULK = 3,// The entire simulation cell
      REGION_COUNT = 4// The number of defined regions

    };


   private:
    // The user-defined names of the regions
    std::vector<std::string> region_names_;

    //The total accessible volume for individual species
    std::vector<double> volumes_;

    // Half length of the cell (F90: ZL4)
    double cell_hlength_;

    // Radius of the simulation cell (F90 : RL5)
    double cell_radius_;

    // Radius of the arc at the protein/membrane interface (F90 RLCURV)
    double membrane_arc_radius_;

    //The half length of the pore cylinder. (F90 : ZL1)
    //
    //The total length of the SF pore is 2 x (pore_hlength + vestibule_radius)
    double pore_hlength_;

    // Radius of the pore (F90 : RL1)
    double pore_radius_;

    //Radius of the outer edge of the channel protein (F90 : RL4)
    double protein_radius_;

    //  Hard limit for structural ion movement along the z-axis. (F90 : ZLIMIT)
    //
    //  REQUIRE structural_hlength_ <= pore_hlength_ + vestibule_radius_
    double structural_hlength_;

    //Radius of the arc joining the channel ends to the outer surface of the protein. (F90 : RLVEST)
    double vestibule_arc_radius_;

    // Permittivity of the media, default media is water
    double epsw_;

    // Permittivity of the protein, default value is 10.0
    double epspr_;

    // Do we use old region boundary definition or new one?
    //
    // This parameter changes the move geometry limits used when
    // generating a trial move.  In the original code these limits lead
    // to slow convergence in particle number at the boundaries of the
    // channel.  For example the region from <zl(2) to >zl(2)+r was
    // considered in the 'ibulk' region so recieved far fewer trials
    // than the adjacent >zl(2).  When this is true the boundaries used
    // in Boda's version 16 code are followed.  When this is false, new
    // move geometry limits are used that overlap adjacent regions and
    // smooth out the number of samples per z-axis unit (meaning the
    // number of samples per unit volume significantly increases through
    // the channel!)
    // fortran equiv geom::useold
    bool use_boda_bounds_;

//
// Manage object lifetime
//
   public:
    channel_system();

    virtual ~channel_system();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< platform::simulator >(*this);
        ar & region_names_;
        ar & volumes_;
        ar & cell_hlength_;
        ar & cell_radius_;
        ar & membrane_arc_radius_;
        ar & pore_hlength_;
        ar & pore_radius_;
        ar & protein_radius_;
        ar & structural_hlength_;
        ar & vestibule_arc_radius_;
        ar & epsw_;
        ar & epspr_;
        ar & use_boda_bounds_;
      };

//
// Manage sub-regions within the cell
//
   public:
    //The number of regions
    std::size_t size() const
    {
      return REGION_COUNT;
    }

    // Get the name of a region at the given index
    //
    // \pre idx < size
    std::string region_name(std::size_t idx) const
    {
      return this->region_names_[idx];
    }

// SIMULATION INITIATION
//
// Coordinate the setting up of the simulation
// trial loop.
// 
    // Register the input file reader classes for this simulation.
    //
    // The base method registers a set of classes that should be
    // simulation cell independent. Derived classes should therefore
    // call this method as well as adding simulation cell dependent
    // parts.
    virtual void build_reader(core::input_delegater & decoder);

    //Generate an initial ensemble to run a simulation.
    //
    //NOTE: region object that accepts 'set_volume' or 
    //a region factory that takes a volume parameter 
    //could be passed as an argument.
    virtual std::unique_ptr< geometry::grid_generator > do_generate_simulation(std::ostream & os);

    //Details about the current geometry to be written to the
    //log at the start of the simulation.
    virtual void do_description(std::ostream & os) const override;


   private:
    //Implement in derived classes to write the name of 
    //the program and any citation information.
    void do_license(std::ostream & os) const;


   public:
    void do_write_document(core::input_document & wr, std::size_t ix) const override;

    //  The maximum input file version to be understood by this program
    virtual std::size_t get_max_input_version() const override;

    //Calculate the accessible volume for a particle of the given specie.
    double volume(std::size_t ispec) const override
    {
      return this->volumes_[ispec];
    }

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_x() const override;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_y() const override;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_z() const override;

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

//
// Manage distances between points
//
   private:
    //Compute the distances between given position and existing positions.
    //
    //\pre rij.size <= ens.size
    //\post rij[0:startindex] === 0
    //\post rij[ens.size:] undefined
    void do_compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, platform::simulator::array_type & rij, std::size_t endindex, std::size_t startindex) const override;


   public:
    //Compute distance between two coordinates.
    double distance_sqr(const geometry::coordinate & lhs, const geometry::coordinate & rhs) const;

//
// Check if an object is obstructed by some fixed geoemtric
// object.
//
    //Is a particle at the given position and radius in a valid position
    //in the system?
    virtual bool is_valid_position(geometry::coordinate & coord, std::size_t ispec) const;

    //Get the permittivity for the region.
    //
    //Note that nested regions may have different permittivity. The
    //permittivity of the inner-most region that contains a particular
    //particle is the value to use for that particle.
    double permittivity() const
    {
      return this->epsw_;
    }

    //Get the permittivity for the region.
    //
    //Note that nested regions may have different permittivity. The
    //permittivity of the inner-most region that contains a particular
    //particle is the value to use for that particle.
    double protein_permittivity() const
    {
      return this->epspr_;
    }

    //Set permitivity for whole geometry
    void set_permittivity(double val)
    {
      this->epsw_ = val;
    }

    //Set permitivity for whole geometry
    void set_protein_permittivity(double val)
    {
      this->epspr_ = val;
    }

    // Get the half length of the total cell
    double cell_hlength() const
    {
      return this->cell_hlength_;
    }

    // Get the radius of the total cell
    double cell_radius() const
    {
      return this->cell_radius_;
    }

    // Get the arc radius of the torus at the corner near the 
    // membrane
    double membrane_arc_radius() const
    {
      return this->membrane_arc_radius_;
    }

    // Get the half length of the pore internal cylinder
    double pore_hlength() const
    {
      return this->pore_hlength_;
    }

    // Get the radius of the pore internal cylinder
    double pore_radius() const
    {
      return this->pore_radius_;
    }

    // Get the radius of the torus from the centre of rotation to
    // the furthest extent 
    double protein_radius() const
    {
      return this->protein_radius_;
    }

    // Get the hlength of the limit of structural ions
    double structural_hlength() const
    {
      return this->structural_hlength_;
    }

    // Test whether to use Dezso's original region
    // bounds or new bounds.
    double use_boda_bounds() const
    {
      return this->use_boda_bounds_;
    }

    // Get the arc radius of the torus at the corner at the 
    // pore
    double vestibule_arc_radius() const
    {
      return this->vestibule_arc_radius_;
    }

    // Get the half length of the total cell
    void set_cell_hlength(double val)
    {
      this->cell_hlength_ = val;
    }

    // Get the half length of the total cell
    void set_cell_radius(double val)
    {
      this->cell_radius_ = val;
    }

    // Get the half length of the total cell
    void set_membrane_arc_radius(double val)
    {
      this->membrane_arc_radius_ = val;
    }

    // Get the half length of the total cell
    void set_pore_hlength(double val)
    {
      this->pore_hlength_ = val;
    }

    // Get the half length of the total cell
    void set_pore_radius(double val)
    {
      this->pore_radius_ = val;
    }

    // Get the half length of the total cell
    void set_protein_radius(double val)
    {
      this->protein_radius_ = val;
    }

    // Get the half length of the total cell
    void set_structural_hlength(double val)
    {
      this->structural_hlength_ = val;
    }

    // Get the half length of the total cell
    void set_boda_bounds(double val)
    {
      this->use_boda_bounds_ = val;
    }

    // Get the half length of the total cell
    void set_vestibule_arc_radius(double val)
    {
      this->vestibule_arc_radius_ = val;
    }

    // Generate a cylinder object containing the allowable sphere
    // centrepoint region for a given specie.
    geometry::cylinder_region region_geometry(std::size_t subregion, std::size_t ispec) const;


};

} // namespace ion_channel
#endif
