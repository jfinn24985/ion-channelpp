#ifndef IONCH_PLATFORM_IMC_SIMULATION_HPP
#define IONCH_PLATFORM_IMC_SIMULATION_HPP

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
#include <boost/serialization/shared_ptr.hpp>
#include <cstddef>
#include <iostream>
#include <boost/serialization/vector.hpp>
#include <string>

namespace core { class strngs; } 
namespace platform { class lamperski_igcmc; } 
namespace platform { class malasics_igcmc; } 
namespace platform { class imc_update; } 
namespace platform { class simulator_meta; } 
namespace core { class input_parameter_memo; } 
namespace platform { class simulation; } 
namespace core { class input_document; } 

namespace platform {

// Simulation manager class for Iterative Monte Carlo simulations,
// for example to estimate the excess chemical potential.
class imc_simulation : public simulation_manager
 {
   private:
    // The updater functor used between each simulation iteration.
    boost::shared_ptr< imc_update > updater_;

    //The number of super loop iterations to perform.
    std::size_t super_loop_size_;

    //The current loop count
    std::size_t super_loop_count_;


   public:
    static void add_definition(simulator_meta & meta);

    //The current loop count
    std::size_t count() const
    {
      return this->super_loop_count_;
    }


   private:
    // Log message descibing the simulation and its parameters
    virtual void do_description(std::ostream & os) const override;


   public:
    imc_simulation(boost::shared_ptr< imc_update > up);


   private:
    // For serialization.
    imc_simulation();


   public:
    virtual ~imc_simulation();

    //Implement in derived classes to write the name of 
    //the program and any citation information.
    virtual void license(std::ostream & os) const override;

    //Set the super-loop size
    std::size_t loop_size() const
    {
      return this->super_loop_size_;
    }

    static boost::shared_ptr< simulation_manager > make_simulation_manager(const std::vector< core::input_parameter_memo >& params);

    //Update the chemical potentials.
    //
    //* Set do_repeat to true if loop count less than loop size.
    //* Update chemical potential by calling (defined in derived classes)
    //do_on_super_loop.
    //* Report chemical potentials.
    bool run(simulation & sim, std::ostream & oslog) override;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
      ar & boost::serialization::base_object< simulation_manager >(*this);
      ar & updater_;
      ar & super_loop_size_;
      ar & super_loop_count_;
    }


   public:
    //Set the super-loop size
    void set_loop_size(std::size_t sz)
    {
      this->super_loop_size_ = sz;
    }

    // Label that identifies this simulation manager subtype in
    // the input file (virtual access).
    std::string type_label() const override
    {
      return imc_simulation::type_label_();
    }

    // Label that identifies this simulation manager subtype in
    // the input file (virtual access).
    static std::string type_label_();


   private:
    // Write extra data of derived simulator object into the input
    // document object at the given section.
    virtual void do_write_part_document(core::input_document & wr, std::size_t ix) const override;


};

} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( platform::imc_simulation );
#endif
