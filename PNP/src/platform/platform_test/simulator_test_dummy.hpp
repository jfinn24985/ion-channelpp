#ifndef IONCH_PLATFORM_SIMULATOR_TEST_DUMMY_HPP
#define IONCH_PLATFORM_SIMULATOR_TEST_DUMMY_HPP

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

#include "geometry/grid.hpp"
#include "platform/simulator.hpp"
#include <cstddef>
#include "utility/unique_ptr.hpp"
#include <iostream>

namespace core { class input_delegater; } 
namespace geometry { class coordinate; } 
namespace geometry { class coordinate_set; } 
namespace core { class input_document; } 

namespace platform {

//  Minimal concrete simulator class for testing
class simulator_test_dummy : public simulator
 {
   private:
    // Register the input file meta classes for this simulation.
    virtual void build_reader(core::input_delegater & decoder) override {}

    //Compute the distances between given position and existing positions.
    //
    //\pre rij.size <= ens.size
    //\post rij[0:startindex] === 0
    //\post rij[ens.size:] undefined
    void do_compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, simulator::array_type & rij, std::size_t endindex, std::size_t startindex = 0) const override;

    // Pass ensemble to derived classes so they can generate
    // the initial set of particles.
    virtual std::unique_ptr< geometry::grid_generator > do_generate_simulation(std::ostream & oslog)
    {
       return std::unique_ptr< geometry::grid_generator >{};
    }
    

    //Implement in derived classes to write the name of 
    //the program and any citation information.
    void do_license(std::ostream & os) const {}

    //  The maximum input file version to be understood by this program
    virtual std::size_t get_max_input_version() const override
    {
      return 1ul;
    }


   public:
    // Is a particle at the given position and radius in a valid position
    // in the system?
    virtual bool is_valid_position(geometry::coordinate & coord, std::size_t ispec) const override
    {
      return true;
    }

    // Return the accessible volume of the given specie.
    virtual double volume(std::size_t ispec) const override
    {
      return 1.0;
    }


   private:
    // Write extra data of derived simulator object into the input
    // document object at the given section.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override {}


   public:
    simulator_test_dummy() = default;

    virtual ~simulator_test_dummy();

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_x() const override;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_y() const override;

    // Get the minimum and maximum dimension along the z coordinate
    virtual std::pair< double, double > range_z() const override;


};

} // namespace platform
#endif
