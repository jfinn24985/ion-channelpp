#ifndef IONCH_EVALUATOR_COULOMB_HPP
#define IONCH_EVALUATOR_COULOMB_HPP

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
#include <iostream>
#include <string>
#include <cstddef>
#include "utility/unique_ptr.hpp"
#include <boost/serialization/vector.hpp>

namespace particle { class ensemble; } 
namespace core { class constants; } 
namespace core { class strngs; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace particle { class change_set; } 
namespace evaluator { class evaluator_manager; } 
namespace core { class input_document; } 
namespace evaluator { class evaluator_meta; } 
namespace core { class input_parameter_memo; } 

namespace evaluator {

//Compute change in Coulomb electrostatic potential between
//the new/old position and existing positions. (Also checks
//for particle-particle overlap)
//
//    \begin{alignat}{3} V_{LJ}& = 4 \varepsilon &\left[
//    \left( \frac {\sigma} {r} \right)^{12} - \left(
//    \frac {\sigma} {r} \right)^6 \right]\\ & = \varepsilon
//    &\left[\left( \frac {r_m} {r} \right)^{12} - 2 \left(
//    \frac {r_m} {r} \right)^6 \right] \end{alignat}
//
//where
//  \varepsilon is the depth of the potential well, \sigma is
//  the finite distance at which the inter-particle potential
//  is zero, r is the distance between the particles,
//  and r_m is the distance at which the potential reaches
//  its minimum.
//
//At r_m, the potential function has the value -\varepsilon.
//The distances are related as r_m = 2^{1/6}\sigma.
//These parameters can be fitted to reproduce experimental
//data or accurate quantum chemistry calculations.  Due to
//its computational simplicity, the Lennard-Jones potential
//is used extensively in computer simulations even though
//more accurate potentials exist.
//

class coulomb : public base_evaluator
 {
   private:
    //The simulation specific energy scale converting
    //charges and Angstrom distances into units of
    //kT
    double scalar_;


   public:
    virtual ~coulomb();

    // Default ctor, Should be called only by serialize and make_evaluator method
    coulomb();



  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< base_evaluator >(*this);
      ar & this->scalar_;
    }


   public:
    //Calculate the change in Coulomb Electrostatic
    //energy on the ensemble in 'sys' caused by changes in
    //'changes'.
    void compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const;

    // Calculate the total in Coulomb Electrostatic
    // energy on the ensemble.
    double compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const;


   private:
    // Log message descibing this evaluator subclass and its parameters
    void do_description(std::ostream & os) const override;


   public:
    // Return the internal to external energy scaling factor. This is undefined before first call to prepare().
    //
    // = (electron_charge^2) / (4.pi.epsilon_0.boltzmann_constant.angstrom . permittivity . temperature)
    double factor() const
    {
      return this->scalar_;
    }
    // Return the invariant part of the energy scaling factor
    //
    // = (electron_charge^2) / (4.pi.epsilon_0.boltzmann_constant.angstrom)
    static double invariant_factor();
    //Prepare the evaluator for use with the given simulator and
    //stepper.
    //
    //Base specie type defines all parameters necessary for the
    //coulomb evaluator, so no species are remove from specs.
    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman);

    std::string type_label() const override
    {
      return coulomb::type_label_();
    }

   private:
    // Write evaluator specific content of input file section.
    //
    // only throw possible should be from os.write() operation
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const;


   public:
    // Create and add a evaluator_definition to the meta object.
    static void add_definition(evaluator_meta & meta);

    static std::string type_label_();

    // Set up the evaluator using the given map of name/value pairs.
    static std::unique_ptr< base_evaluator > make_evaluator(const std::vector< core::input_parameter_memo > & param_set);


};

} // namespace evaluator

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( evaluator::coulomb );

#endif
