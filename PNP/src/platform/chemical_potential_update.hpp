#ifndef IONCH_PLATFORM_CHEMICAL_POTENTIAL_UPDATE_HPP
#define IONCH_PLATFORM_CHEMICAL_POTENTIAL_UPDATE_HPP


#include "platform/imc_update.hpp"
#include "utility/estimate_array.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include <iostream>

namespace platform { class specie_monitor; } 
namespace platform { class simulation; } 

namespace platform {

// Base class for IMC updaters that iteratively change the chemical potential.
class chemical_potential_update : public imc_update
 {
   private:
    utility::estimate_array chemical_potentials_;

    // Monitor specie count.
    mutable boost::shared_ptr< specie_monitor > counts_;


   public:
    // Get our specie monitor observer
    const specie_monitor& counts() const;

    //Prepare the evaluator for use with the given simulator and
    //stepper.
    //
    //- Add self to super loop observable list
    void prepare(simulation & sim);


   private:
    virtual void do_prepare(simulation & sim) = 0;


   public:
    //Update the chemical potentials.
    //
    //* Set do_repeat to true if loop count less than loop size.
    //* Update chemical potential by calling (defined in derived classes)
    //do_on_super_loop.
    //* Report chemical potentials.
    void update(simulation & sys, std::ostream & oslog) override;

    // Perform update after each IMC cycle.
    virtual void do_update(simulation & sys, std::ostream & oslog) = 0;

    // Write output on chemical potential estimates to data sink. This is 
    // called in update and should not be called manually except for testing.
    void write_output(const simulation & sim) const;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
      ar & boost::serialization::base_object< imc_update >(*this);
      ar & this->counts_;
    }


   public:
    //Factor for calculating the chemical potential correction 
    //from non-zero charge of system. Final correction for
    //chemical potential of specie ispec is
    //
    //-(ispec.valency * average_charge * result)
    //
    
    double charge_correction_factor(const simulation & sim) const;


};

} // namespace platform
#endif
