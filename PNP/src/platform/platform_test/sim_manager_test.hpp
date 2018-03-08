#ifndef IONCH_PLATFORM_SIM_MANAGER_TEST_HPP
#define IONCH_PLATFORM_SIM_MANAGER_TEST_HPP

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

#include "platform/imc_update.hpp"
#include <iostream>
#include <cstddef>
#include <string>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace platform { class imc_simulation; } 
namespace platform { class simulation; } 
namespace platform { class simulation_definition; } 
namespace platform { class simulation_manager; } 
namespace platform { class simulator_meta; } 
namespace platform { class standard_simulation; } 
namespace core { class input_document; } 

namespace platform {

// Unit tests for the simulation_manager and derived classes
//
// Includes: simulation_manager, standard_simulation, imc_simulation,
//   imc_update, chemical_potential_update, malasics_igcmc, lamperski_igcmc
class sim_manager_test
 {
   public:
    class simple_updater : public imc_update
     {
       public:
        // Log message descibing the itearation update and its parameters
        virtual void description(std::ostream & os) const override;

        // Add content of update into IMC section of an input file section.
        //
        //only throw possible should be from os.write() operation
        virtual void do_write_part_document(core::input_document & wr, std::size_t ix) const override;

        //Prepare the updater before running a series of simulation steps
        virtual void prepare(simulation & sim) override;

        // Perform update after each IMC cycle.
        virtual void update(simulation & sys, std::ostream & oslog) override;

    
  friend class boost::serialization::access;
       private:
        template<class Archive>
          inline void serialize(Archive & ar, const unsigned int version) {
          ar & boost::serialization::base_object< imc_update >(*this);
        }


    };
    
    // Test input file content.
    static std::string canon_input();

    // (using standard_simulation)
    // Methods tested
    //  * equilibration_interval
    //  * production_interval
    //  * set_equilibration_interval
    //  * set_production_interval
    //  * serialize
    //  * description
    //  run
    //  * license
    //  write_part_document
    //  * type_label
    
    static void simulation_manager_methods_test(boost::shared_ptr< simulation_manager > sman, std::string label, std::string lic_text, std::vector< std::string > entries);


};

} // namespace platform
#endif
