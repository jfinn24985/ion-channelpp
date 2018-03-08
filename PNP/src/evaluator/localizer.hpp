#ifndef IONCH_EVALUATOR_LOCALIZER_HPP
#define IONCH_EVALUATOR_LOCALIZER_HPP

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
#include "utility/archive.hpp"

#include "geometry/centroid.hpp"
#include <string>
#include "utility/unique_ptr.hpp"
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <cstddef>

namespace evaluator { class evaluator_meta; } 
namespace core { class input_parameter_memo; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace particle { class change_set; } 
namespace core { class input_document; } 
namespace evaluator { class evaluator_manager; } 

namespace evaluator {

//  Evaluate the mobile structural ion localisation penalty.
//
//
//  This computes the potential energy change of moving a particle.
//  For a harmonic oscillator the potential stores in a particle is
//
//     U = 1/2 k r^2
//
//  Where k is the spring constant and r is the distance from the centre
//  of the oscillation.
//
//  To approximate a Uniform distribution the potential is.
//
//     U  = k_bT/2 ( r^2 )/( sigma^2 )  -- eqn 1
//
//  We use units of k_bT and so what we calculate is
//
//     U = C_mob ( r^2 )/( R_i^2 ) -- eqn 2
//
//  Where kmob is a simulation wide constant and R_i is the per-particle
//  is the cut-off distance.
//
//  To get eqn 2 to model a Uniform distribution our values of the spring factor
//  (C_mob, parameter "mobk") and R_i must satisfy the relations.
//
//     C_mob = 1/2 n^2 and R_i = n * sigma -- eqn 3
//
//  where n is any factor. Conversely, given kmob and R_i the distribution
//  models a Uniform distribution with 
//
//     sigma^2 = R_i^2 / (2 * C_mob)
//     
//  DEFAULT SETTINGS:
//
//  In a Uniform distribution, greater that 99.5% of the distribution
//  is within 3 standard deviations of the mean position. We take this as
//  the default by defining (n) in eqn 3 as 3 giving. 
// 
//     C_mob = 4.5
//     R_i  = 3 * sigma
//
//  If we have B factors from an X-ray we can determine R_i from
//
//     R_i = 3 * sigma = 3 { 1/(2 pi) sqrt( B_i/2 ) }
//
//   
//
//
//

class localizer : public base_evaluator
 {
   private:
    // Constant multiplier for mobile ion constraint. Note this
    // is the maximum value that the penalty energy can contribute
    // to the potential energy as we scale the displacement by the 
    // maximum displacement (rsr^2) when calculating Hooke's law
    // potential energy.
    //
    // fortran equiv k_mobl
    double spring_factor_;

// lifetime methods


   public:
    localizer();

    virtual ~localizer();



  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< base_evaluator >(*this);
      ar & this->spring_factor_;
    }

// access methods


   public:
    // The default spring factor (4.5) when paired with R_max
    // values 2 x X-Ray B-factor or atomic simulation RMSDs should
    // give comparable atom distributions to the X-Ray or atomic
    // simulations.
    
    static constexpr double default_spring_factor()
    {
      return 4.5;
    }

    double spring_factor() const
    {
      return this->spring_factor_;
    }

    //  Set the global spring factor.
    void spring_factor(double a_factor)
    {
      this->spring_factor_ = a_factor;
    }

    virtual std::string type_label() const override
    {
      return localizer::type_label_();
    }
    // Create and add a evaluator_definition to the meta object.
    static void add_definition(evaluator_meta & meta);

    // Set up the evaluator using the given map of name/value pairs
    //
    // Valid for all simtypes. Optional spring factor constant "mobk" parameter
    static std::unique_ptr< base_evaluator > make_evaluator(const std::vector< core::input_parameter_memo > & param_set);

    static std::string type_label_();

// action methods

    // Calculate change in localization energy if one of the atoms
    // in the change set has moved.
    virtual void compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const override;

    //Calculate the total potential energy.
    virtual double compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const override;


   private:
    // Log message descibing the evaluator and its parameters
    virtual void do_description(std::ostream & os) const;

    // Write evaluator specific content of input file section.
    //
    // only throw possible should be from os.write() operation
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const;


   public:
    // This is a no-op; a localizer objects does not need any
    // preparation before a simulation.
    virtual void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman) override;


};

} // namespace evaluator

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( evaluator::localizer );

#endif
