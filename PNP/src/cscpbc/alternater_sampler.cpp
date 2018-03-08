

#ifndef DEBUG
#define DEBUG 0
#endif

#include "cscpbc/alternater_sampler.hpp"
#include "core/input_document.hpp"
#include "platform/simulator.hpp"
#include "observable/report_manager.hpp"

// Manuals
#include "observable/acceptance_observable.hpp"
// -

namespace periodic_cube {

alternater_sampler::~alternater_sampler()
{}

//Log message descibing the observable and its parameters
void alternater_sampler::description(std::ostream & out) const
{
  out << "SAMPLER [" << this->get_label() << "]\n";
  out << " Collect data about alternater choice add/remove probabilities\n";
  out << " (Is automatically added when needed and can not be accessed via\n";
  out << " the input file.)\n";

}

// This sampler is not intended to be generated from an input file, so this 
// method must delete entry from output doc.
void alternater_sampler::do_write_document(core::input_document & wr, std::size_t ix) const
{
  // remove entry.
  wr.remove_section(ix);

}

//Retrieve the current value of the variable.
//
///return type vector< probability_datum >
boost::any alternater_sampler::get_value() const
{
  boost::any result = this->probabilities_;
  return result;

}

// Does nothing as this object does not store the observations
// it samples.
void alternater_sampler::prepare(platform::simulator & sim)
{}

//Broadcast add trial success probabilities. Does not output
//data anywhere.
void alternater_sampler::on_sample(const platform::simulator & sim)
{
  if (this->probabilities_.empty())
  {
    this->probabilities_.resize(sim.specie_count());
    for (std::size_t idx = 0; idx != this->probabilities_.size(); ++idx)
    {
      this->probabilities_[idx].specie_key = idx;
    }
  }
  // Get acceptance data from "trial-success" sampler
  if (not sim.get_reporter().has_tracked(observable::acceptance_observable::type_label_()))
  {
    std::vector< observable::acceptance_observable::acceptance_datum > result
       = boost::any_cast< std::vector< observable::acceptance_observable::acceptance_datum > >(sim.get_reporter().get_tracked(observable::acceptance_observable::type_label_())->get_value());
    for(auto const& item : result)
    {
      if ((item.label.find("Add") != std::string::npos) or (item.label.find("add") != std::string::npos))
      {
        this->probabilities_[item.specie_key].add.append(item.probability());
      }
      else if ((item.label.find("Del") != std::string::npos) or (item.label.find("del") != std::string::npos))
      {
        this->probabilities_[item.specie_key].remove.append(item.probability());
      }
    }
  }
  
  

}

//Does nothing.
void alternater_sampler::on_report(const platform::simulator & sim, std::ostream & out, observable::report_manager & reporter)
{}

std::string alternater_sampler::type_label_()
{
  return std::string("alternater-sampler");
}


} // namespace periodic_cube


#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(periodic_cube::alternater_sampler, "periodic_cube::alternater_sampler");