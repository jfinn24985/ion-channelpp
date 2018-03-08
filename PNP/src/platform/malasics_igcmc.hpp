#ifndef IONCH_PLATFORM_MALASICS_IGCMC_HPP
#define IONCH_PLATFORM_MALASICS_IGCMC_HPP

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

#include "platform/chemical_potential_update.hpp"
#include <iostream>
#include <string>
#include <cstddef>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace core { class constants; } 
namespace platform { class specie_monitor; } 
namespace core { class strngs; } 
namespace platform { class simulation; } 
namespace core { class input_document; } 
namespace platform { class simulation_definition; } 
namespace platform { class imc_update; } 
namespace core { class input_parameter_memo; } 

namespace platform {

class malasics_igcmc : public chemical_potential_update
 {
   public:
    //Log message descibing the observable and its parameters
    void description(std::ostream & os) const override;


   private:
    //Update the chemical potentials (in derived classes)
    virtual void do_update(simulation & sys, std::ostream & oslog) override;


   public:
    virtual ~malasics_igcmc();


   private:
    //Prepare the evaluator for use with the given simulator and
    //stepper.
    virtual void do_prepare(simulation & sim) override;


   public:
    std::string type_label() const;

    static std::string type_label_();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
      ar & boost::serialization::base_object< chemical_potential_update >(*this);
    }

    //Write an input file section.
    //
    //only throw possible should be from os.write() operation
    virtual void do_write_part_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Add updater specific options to the definition object and
    // return the "updater" type label for this class.
    static std::string add_to_definition(simulation_definition & defn);

    static boost::shared_ptr< imc_update > make_updater(const std::vector< core::input_parameter_memo >& params);


};

} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( platform::malasics_igcmc );
#endif
