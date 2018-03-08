

#ifndef DEBUG
#define DEBUG 0
#endif

#include "platform/chemical_potential_update.hpp"
#include "platform/specie_monitor.hpp"
#include "platform/simulation.hpp"

// manual includes
#include "core/strngs.hpp"
#include "observable/base_sink.hpp"
#include "observable/report_manager.hpp"
#include "observable/output_buffer_datum.hpp"
#include "observable/output_text.hpp"
#include "particle/particle_manager.hpp"
#include "platform/imc_simulation.hpp"
// -
namespace platform {

// Get our specie monitor observer
const specie_monitor& chemical_potential_update::counts() const
{
  if( not this->counts_ )
  {
    this->counts_.reset( new specie_monitor );
  }
  return *(this->counts_);

}

//Prepare the evaluator for use with the given simulator and
//stepper.
//
//- Add self to super loop observable list
void chemical_potential_update::prepare(simulation & sim) 
{
  if( not this->counts_ )
  {
    this->counts_.reset( new specie_monitor );
  }
  this->chemical_potentials_.clear();
  this->chemical_potentials_.resize( sim.particles().specie_count() );
  if( not sim.report().has_sample( this->counts_->get_label() ) )
  {
    sim.get_report_manager().add_sample( this->counts_ );
  }
  this->do_prepare( sim );
  

}

//Update the chemical potentials.
//
//* Set do_repeat to true if loop count less than loop size.
//* Update chemical potential by calling (defined in derived classes)
//do_on_super_loop.
//* Report chemical potentials.
void chemical_potential_update::update(simulation & sys, std::ostream & oslog) 
{
  // Save existing chemical potentials.
  auto const& pman = sys.particles();
  std::vector< double > old_cpx_list( pman.specie_count() );
  std::vector< double > new_cpx_list( pman.specie_count() );
  for(std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
  {
    old_cpx_list[ ispec ] = pman.get_specie( ispec ).excess_potential();
  }
  
  // Call derived class to update excess potentials
  this->do_update( sys, oslog );
  
  // update statistics
  for(std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
  {
    new_cpx_list[ ispec ] = pman.get_specie( ispec ).excess_potential();
  }
  this->chemical_potentials_.append( new_cpx_list );
  
  this->write_output( sys );
  
  // Report chemical potentials
  bool first = true;
  boost::format fmt_head (" %3s %8s %8s %8s\n");
  boost::format fmt_row (" %3s %8.4f %8.4f %8.4f\n");
  for (std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec)
  {
    auto const& spcobj = pman.get_specie(ispec);
    // only solute species have interesting chemical potentials
    if (spcobj.is_solute())
    {
      if (first)
      {
        first = false;
        oslog << core::strngs::horizontal_bar() << "\n";
        oslog << " CHEMICAL POTENTIAL ESTIMATE \n";
        oslog << boost::format(fmt_head) % "SPC" % "CHEM EX" % "DLTA_CPX" % "<CPX>";
        oslog << core::strngs::horizontal_bar() << "\n";
      }
      oslog << boost::format(fmt_row) % spcobj.label() %  new_cpx_list[ispec]
          % (new_cpx_list[ispec] - old_cpx_list[ispec]) % this->chemical_potentials_.mean( ispec );
    }
  }
  // Output horizontal bar only if at least
  // one chemical potential was changed.
  if (not first)
  {
    oslog << core::strngs::horizontal_bar() << "\n";
  }

}

// Write output on chemical potential estimates to data sink. This is 
// called in update and should not be called manually except for testing.
void chemical_potential_update::write_output(const simulation & sim) const
{
  const std::string label { "igcmc.dat" };
  
  const std::size_t loopindex { dynamic_cast< const imc_simulation& >( sim.manager() ).count() + 1 };
  const particle::particle_manager& pman = sim.particles();
  auto & sink = sim.report().get_sink();
  
  if( not sink.has_dataset( label ) )
  {
    std::unique_ptr< observable::output_text >definition{ new observable::output_text( label, true ) };
    definition->set_title( "Series of chemical potential estimates." );
    std::stringstream lbls, unts;
    lbls << "INDEX ";
    unts << "ORDINAL ";
    for (std::size_t ispec = 0; ispec != this->chemical_potentials_.size(); ++ispec)
    {
     const std::string slabel = pman.get_specie( ispec ).label();
     lbls << "SPECIE " << slabel << "_CURRENT " << slabel << "_MEAN " << slabel << "_VAR ";
     unts << "LABEL ENERGY ENERGY ENERGY ";
    }
    std::string labels( lbls.str() );
    std::string units( unts.str() );
    labels.pop_back();
    units.pop_back();
    definition->set_field_labels( labels );
    definition->set_units( units );
  
    sink.add_dataset( std::move( definition ) );
  }
  std::stringstream os;
  os << loopindex << " ";
  for (std::size_t ispec = 0; ispec != this->chemical_potentials_.size(); ++ispec)
  {
     const double mu_ex = pman.get_specie( ispec ).excess_potential();
     const double mean = this->chemical_potentials_.mean( ispec );
     const double var = this->chemical_potentials_.variance( ispec );
     os << pman.get_specie( ispec ).label() << " " << mu_ex << " " << mean << " " << var << " ";
  }
  os << "\n";
  std::unique_ptr< observable::output_datum > dtm( new observable::output_buffer_datum( os.str() ) );
  sink.receive_data( label, std::move( dtm ) );

}

//Factor for calculating the chemical potential correction 
//from non-zero charge of system. Final correction for
//chemical potential of specie ispec is
//
//-(ispec.valency * average_charge * result)
//

double chemical_potential_update::charge_correction_factor(const simulation & sim) const 
{
  //namespace cc = core::constants;
  //static const double csloth { ( 6*std::log( 2 + std::sqrt(3.0)) - cc::pi() )
  //  / (16 * cc::pi() * cc::eps0() * cc.dsi() ) };
  return 0.0;
  // csloth * sim.beta() * std::cbrt(sim.volume());
  //    chisp=-xz(ispec)*avchg*chcons

}


} // namespace platform
