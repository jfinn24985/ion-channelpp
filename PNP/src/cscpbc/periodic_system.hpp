#ifndef IONCH_PERIODIC_CUBE_PERIODIC_SYSTEM_HPP
#define IONCH_PERIODIC_CUBE_PERIODIC_SYSTEM_HPP

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
#include <boost/serialization/shared_ptr.hpp>
#include <iostream>
#include <cstddef>
#include "utility/unique_ptr.hpp"

namespace core { class strngs; } 
namespace platform { class imc_simulation; } 
namespace core { class input_delegater; } 
namespace core { class input_document; } 
namespace geometry { class grid_generator; } 
namespace geometry { class coordinate; } 
namespace geometry { class coordinate_set; } 

namespace periodic_cube {

// Manage a simulation in a periodic cube cell.

class periodic_system : public platform::simulator
 {
   private:
    // Optional object managing iterative MC simulations
    boost::shared_ptr< platform::imc_simulation > imc_;

    //The simulation box length
    double length_;

    // Permittivity of the media, default media is water
    double epsw_;

//
// Manage object lifetime
//

   public:
    //Default ctor for deserialization only
    periodic_system() = default;

    virtual ~periodic_system();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< platform::simulator >(*this);
        ar & imc_;
        ar & length_;
        ar & epsw_;
      };

// SIMULATION INITIATION
//
// Coordinate the setting up of the simulation
// trial loop.
// 

   public:
    // Register the input file reader classes for this simulation.
    //
    // The base method registers a set of classes that should be
    // simulation cell independent. Derived classes should therefore
    // call this method as well as adding simulation cell dependent
    // parts.
    virtual void build_reader(core::input_delegater & decoder);

    // (how to output IMC)
    // Details about the current geometry to be written to the
    // log at the start of the simulation.
    virtual void do_description(std::ostream & os) const override;


   private:
    //Implement in derived classes to write the name of 
    //the program and any citation information.
    void do_license(std::ostream & os) const;

    // (todo: how to output IMC)
    // Write an input file section.
    //
    // only throw possible should be from os.write() operation
    void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    //Generate an initial ensemble to run a simulation.
    //
    //NOTE: region object that accepts 'set_volume' or 
    //a region factory that takes a volume parameter 
    //could be passed as an argument.
    virtual std::unique_ptr< geometry::grid_generator > do_generate_simulation(std::ostream & os);


   private:
    // Derived class implementation of run method.
    virtual void do_run(std::ostream & oslog) override;


   public:
    //  The maximum input file version to be understood by this program
    virtual std::size_t get_max_input_version() const override;

    void set_super_looper(boost::shared_ptr< platform::imc_simulation >& imc)
    {
      this->imc_ = imc;
    }

// SIMULATION OWNERSHIP
//
// Manage the set of objects required to run a
// simulation.
//
//
// Manage distances between points
//
//
// Check if an object is obstructed by some fixed geoemtric
// object.
//
    //Is a particle at the given position and radius in a valid position
    //in the system?
    //
    //For PBC system, the coordinate is adjusted within the cube and true
    //always returned.
    virtual bool is_valid_position(geometry::coordinate & coord, std::size_t ispec) const override;

// SIMULATION INITIATION
//
// Coordinate the setting up of the simulation
// trial loop.
// 
    // Compute square of distance between two coordinates in 
    // a periodic box.
    double distance_sqr(const geometry::coordinate & lhs, const geometry::coordinate & rhs) const;


   private:
    //Compute the distances between any new position and existing positions.
    //
    //\post rij.size == ens.size
    //\post rij[0:start_index] == 0.0
    
    void do_compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, platform::simulator::array_type & rij, std::size_t endindex, std::size_t startindex = 0) const override;

//
// Manage sub-regions within the cell
//

   public:
    //The length dimension of the periodic cube
    double length() const
    {
       return this->length_;
    }

    // Return the current concentration of the given specie.
    //
    // For a periodic system, the volume is independent of specie, so ispec
    // is always ignored (ie a valid ispec is not checked)
    virtual double volume(std::size_t ispec) const override;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_x() const override;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_y() const override;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_z() const override;

    //Get the permittivity for the region.
    //
    //Note that nested regions may have different permittivity. The
    //permittivity of the inner-most region that contains a particular
    //particle is the value to use for that particle.
    double permittivity() const
    {
      return this->epsw_;
    }

    //Set permitivity for whole geometry
    void set_permittivity(double val)
    {
      this->epsw_ = val;
    }


};

} // namespace periodic_cube

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY(periodic_cube::periodic_system);

#endif
