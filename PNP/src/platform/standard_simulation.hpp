#ifndef IONCH_PLATFORM_STANDARD_SIMULATION_HPP
#define IONCH_PLATFORM_STANDARD_SIMULATION_HPP

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

#include "platform/simulation_manager.hpp"
#include "utility/archive.hpp"

#include <iostream>
#include <string>
#include <cstddef>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace core { class input_document; } 
namespace platform { class simulator_meta; } 
namespace platform { class simulation; } 
namespace core { class input_parameter_memo; } 

namespace platform {

// The style of simulation that is running.  This manages the super-loop variants
// and can be selected from the input file at runtime.
class standard_simulation : public simulation_manager
 {
   public:
    standard_simulation()
    : simulation_manager()
    {}

    virtual ~standard_simulation();


   private:
    // No move
    standard_simulation(standard_simulation && source);

    // No copy
    standard_simulation(const standard_simulation & source);

    // No assignment
    standard_simulation & operator=(const standard_simulation & source);


  friend class boost::serialization::access;    template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< platform::simulation_manager >( *this );
    }
    // Details about the current storage object.
    virtual void do_description(std::ostream & os) const override;


   public:
    //Implement in derived classes to write the name of 
    //the program and any citation information.
    virtual void license(std::ostream & os) const override;

    // Label that identifies this simulation manager subtype in
    // the input file (virtual access).
    virtual std::string type_label() const override;

    // Label that identifies this simulation manager subtype in
    // the input file.
    static std::string type_label_();


   private:
    // Write extra data of derived simulator object into the input
    // document object at the given section.
    virtual void do_write_part_document(core::input_document & wr, std::size_t ix) const override;


   public:
    static void add_definition(simulator_meta & meta);

    // Perform the simulation. This class does:
    //
    // sim.prepare + sim.run_loop(equil_int)
    // sim.prepare + sim.run_loop(prod_int)
    
    bool run(simulation & sim, std::ostream & oslog) override;

    static boost::shared_ptr< simulation_manager > make_simulation_manager(const std::vector< core::input_parameter_memo >& params);


};

} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( platform::standard_simulation );
#endif
