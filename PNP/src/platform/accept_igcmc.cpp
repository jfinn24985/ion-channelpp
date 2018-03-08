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

#ifndef DEBUG
#define DEBUG 0
#endif

#include "platform/accept_igcmc.hpp"
#include "utility/archive.hpp"

#include "core/constants.hpp"
#include "core/strngs.hpp"
#include "platform/simulator.hpp"
#include "core/input_document.hpp"
#include "observable/base_observable.hpp"

// Manuals
//#include <boost/bind.hpp>
#include "cscpbc/alternater_choice.hpp"
#include "cscpbc/alternater_sampler.hpp"
#include "cscpbc/periodic_system.hpp"
#include "observable/acceptance_observable.hpp"
#include "particle/ensemble.hpp"
// -
namespace platform {

//Log message descibing the observable and its parameters
void accept_igcmc::description(std::ostream & os) const 
{
  os << "[sampler] " << this->type_label_() << "\n";
  os << " Super-observable: used to perform iterative GCMC simulations.\n";
  os << " Use IGCMC to estimate the excess chemical potentials required\n";
  os << " to maintain the target concentrations of solute species.\n";
  this->igcmc::description(os);

}

//Update the chemical potentials (in derived classes)
void accept_igcmc::do_on_super_loop(simulator & sys) 
{
  // Get mean concentration from specie_count sampler.
  //
  // NOTE: we could use alternater_sampler::type_label_()
  boost::any choices_any = sys.get_reporter().get_sample("alternater-sampler")->get_value();
  std::vector< alternater_sampler::probability_datum > const& choices = *(boost::any_cast< std::vector< alternater_sampler::probability_datum >* >(choices_any));
  
  for (std::size_t ispec = 0; ispec != sys.specie_count(); ++ispec)
  {
    alternater_sampler::probability_datum const* specie_datum = nullptr;
    for (auto const& datum : choices)
    {
      if (datum.specie_key == ispec)
      {
        specie_datum = &datum;
        break;
      }
    }
    UTILITY_CHECK(nullptr != specie_datum, "Could not find specie datum for a specie.");
  
    if (nullptr == specie_datum)
    {
      continue;
    }
    auto & spcobj = sys.get_specie(ispec);
    // only solute species in PBC simulations
    const double p_add
    {
      specie_datum->add.mean()
    };
    const double p_del
    {
      specie_datum->remove.mean()
    };
    // Initialise chempi to the previous value.
    double chempi (spcobj.excess_potential());
    const double num_of_particles(spcobj.concentration()*sys.volume(0)/core::constants::to_SI());
  
    switch(this->variant_)
    {
    case 0:
    case 1:
      chempi = spcobj.excess_potential() + std::log(p_del/p_add)/2.0;
      break;
    case 5:
      //      ac6nt=ctargi(ispec)*volblk()/tosi
      //     ac6n=floor(ac6nt)
      //      if (ac6nt.gt.1.D0) then
      //        tmp=(ac6nt*ac6nt)/(ac6n*ac6n+ac6n)
      //      else
      //        tmp=6.D0*ac6nt
      //      endif
      //      write(*,*)"Multiply factor : N_t=",ac6nt," N=",ac6n," factor=",tmp
      //      call chexi_set (ispec, chexi(ispec) + dlog((p_d/p_a) * tmp)/2.D0 + chisp)
      //      write(unit=fidlog,fmt=*)"##### ln((P_d/P_a)*(N_t**2/N(N+1)))/2 #####"
    {
      double ac6n=std::floor(num_of_particles);
      double tmp=(num_of_particles > 1 ? (num_of_particles*num_of_particles)/(ac6n*ac6n+ac6n) : (6*num_of_particles));
      chempi = spcobj.excess_potential() + std::log(tmp*p_del/p_add)/2.0;
    }
    break;
    case 13:
      //      write(unit=fidlog,fmt=*)"#########       Variant 13      ##########"
      //       ac6nt=ctargi(ispec)*volblk()/tosi
      //       tmp=(1.D0-erf((0.5D0-ac6nt)/sqrt(2.D0*ac6nt)))/2.D0
      //       write(*,*)"Multiply factor : N_t=",ac6nt," N=",ac6n," factor=",tmp
      //       call chexi_set (ispec, chexi(ispec) + dlog(tmp*p_d/p_a)/2.D0 + chisp)
      //       write(unit=fidlog,fmt=*)"##### ln(1-erf(1/2-N_t)/sqrt(2*N_t)) #####"
    {
      double tmp=(1-std::erf((0.5-num_of_particles)/std::sqrt(2*num_of_particles)))/2;
      chempi = spcobj.excess_potential() + std::log(tmp*p_del/p_add)/2.0;
    }
    break;
    default:
    {
      std::size_t known_variant = 0;
      UTILITY_INPUT(this->variant_ == known_variant, "Accept IGCMC variant is unknown", core::strngs::sampler_label(), nullptr);
    }
    break;
    }
    spcobj.set_excess_potential(chempi);
  }
  

}

//Prepare for a main simulation loop
void accept_igcmc::do_prepare(simulator & sim) 
{}

//Write an input file section.
//
//only throw possible should be from os.write() operation
void accept_igcmc::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );
  wr[ ix ].add_entry( "variant", this->variant_ );

}

//Create a new widom sampler using the given parameter set.
void accept_igcmc::make_super_looper(std::map< std::string, std::string > const& params, simulator & sim)
{
  bool required_tags_step = false;
  bool required_tags_variant = false;
  std::unique_ptr< accept_igcmc > smplr( new accept_igcmc );
  // Check parameters for usable values
  for (auto const &item:params)
  {
    if (item.first == core::strngs::fsnstp ())
    {
      smplr->set_loop_size (boost::lexical_cast < std::size_t > (item.second));
      required_tags_step = true;
    }
    else if (item.first == "variant")
    {
      smplr->variant_ = (boost::lexical_cast < std::size_t > (item.second));
      required_tags_variant = true;
    }
    else
    {
      const std::string known_key;
      UTILITY_INPUT(item.first == known_key,
                     "Parameter [" + item.first +
                     "] unknown for sampler type [" +
                     accept_igcmc::type_label_ () + "]",
                     core::strngs::sampler_label (), nullptr);
    }
  }
  UTILITY_INPUT(required_tags_step,
                 "Required number of steps tag was not present.",
                 core::strngs::sampler_label (), nullptr);
  if (not required_tags_variant)
  {
    // default variant is '0'
    smplr->variant_ = 0;
  }
  boost::shared_ptr< periodic_cube::igcmc > result( smplr.release() );
  dynamic_cast< periodic_cube::periodic_system& >( sim ).set_super_looper( result );
  if (not sim.get_reporter().has_tracked(observable::acceptance_observable::type_label_()))
  {
    std::map< std::string, std::string > dummy;
    observable::acceptance_observable::make_sampler(dummy, sim);
  }
  if (not sim.get_reporter().has_sample("alternater-sampler"))
  {
    boost::shared_ptr< observable::sampled_observable > alt_sampler(new alternater_sampler);
    sim.get_reporter().add_sample(alt_sampler);
  }
  

}

std::string accept_igcmc::type_label() const 
{
  return type_label_();
}

std::string accept_igcmc::type_label_()

{
  return std::string("accept");
}


} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(periodic_cube::accept_igcmc, "periodic_cube::accept_igcmc");