#ifndef IONCH_PLATFORM_ACCEPT_IGCMC_HPP
#define IONCH_PLATFORM_ACCEPT_IGCMC_HPP

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
#include "platform/imc_simulation.hpp"
#include <cstddef>
#include <iostream>
#include <boost/serialization/map.hpp>
#include <string>

namespace core { class constants; } 
namespace core { class strngs; } 
namespace platform { class simulator; } 
namespace core { class input_document; } 
namespace observable { class base_observable; } 

namespace platform {

class accept_igcmc : public imc_simulation
 {
   private:
    //this represents the number of the methods
    std::size_t variant_;


   public:
    //Log message descibing the observable and its parameters
    void description(std::ostream & os) const;

    //Update the chemical potentials (in derived classes)
    virtual void do_on_super_loop(simulator & sys) override;


   private:
    //Prepare for a main simulation loop
    virtual void do_prepare(simulator & sim) override;

    //Write an input file section.
    //
    //only throw possible should be from os.write() operation
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    //Create a new widom sampler using the given parameter set.
    static void make_super_looper(std::map< std::string, std::string > const& params, simulator & sim);

    std::string type_label() const;

    static std::string type_label_();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
      ar & boost::serialization::base_object< igcmc >(*this);
      ar & variant_;
    }


};

} // namespace platform
#endif
