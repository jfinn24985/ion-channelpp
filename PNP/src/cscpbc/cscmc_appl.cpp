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

// BEGIN EXTRA INCS
#include "utility/config.hpp"
#include "utility/require.hpp"

// Input file reader header
#include "core/input_delegater.hpp"

// Application headers
#include "platform/application.hpp"
#include "platform/application_meta.hpp"

// Simulator related headers
#include "platform/simulator.hpp"
#include "platform/simulator_meta.hpp"
#include "platform/periodic_system.hpp"

// Specie reader header
#include "particle/specie_meta.hpp"

// Evaluator related headers
#include "evaluator/evaluator_meta.hpp"
#include "evaluator/lennard_jones.hpp"
#include "evaluator/coulomb.hpp"

// Data collector headers
#include "observable/sampler_meta.hpp"
#include "observable/base_observable.hpp"
#include "observable/d3_distribution.hpp"
#include "observable/metropolis_sampler.hpp"
#include "observable/rdf_sampler.hpp"
#include "observable/specie_count.hpp"
#include "observable/widom.hpp"
// Super loop observables
#include "observable/malasics_igcmc.hpp"
#include "observable/lamperski_igcmc.hpp"
#include "observable/accept_igcmc.hpp"

// Trial type headers
#include "trial/choice.hpp"
#include "trial/add_choice.hpp"
#include "trial/chooser.hpp"
#include "trial/choice_meta.hpp"
#include "trial/accept_choice.hpp"

// Utility headers
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <limits>
#include "core/constants.hpp"
#include "utility/machine.hpp"
#include "core/strngs.hpp"
//END
static const std::size_t max_input_ver (1);

// Random seed value
#ifdef HAVE_NO_RANDOM_DEVICE
// This is only the case when boost::random::random_device is not defined for
// your environment. This should never be the case!
namespace
{
  unsigned int random_seed_value () 
  {
    return 859091u;
  }
}
#else
#include <boost/random/random_device.hpp>
namespace
{
  // Get random seed value from random device
  unsigned int random_seed_value()
  {
    boost::random::random_device seedgenr;
    return seedgenr() - seedgenr.min();
  }
}
#endif

int main (int argc, char** argv)
{
  boost::shared_ptr< platform::application > appl (new platform::application);
  core::input_delegater decoder (max_input_ver);
  {
    // Add input decoder class to handle the base application options
    boost::shared_ptr< core::input_base_meta > delegate (new platform::application_meta(appl));
    decoder.add_input_delegate (delegate);
  }
  {
    // Add input decoder class to handle the specie definition options
    boost::shared_ptr< core::input_base_meta > delegate (new particle::specie_meta(appl->get_specie_manager ()));
    decoder.add_input_delegate (delegate);
  }
  {
    // Add input decode class to handle the simulation type options ???
    std::unique_ptr< platform::simulator_meta > tmp_(new platform::simulator_meta);
    // Add the simulator types that this application should handle
    tmp_->type_to_object_[core::strngs::bulk_label()] = &platform::periodic_system::make_geometry;
    boost::shared_ptr< core::input_base_meta > delegate (tmp_.release());
    decoder.add_input_delegate (delegate);
  }
  {
    // Add input decoder class to handle the energy evaluator options.  This
    // handles instantiation of the requested evaluator.
    std::unique_ptr< evaluator::evaluator_meta > tmp_ (new evaluator::evaluator_meta);
    // Add the evaluator types tha t we want this application to handle
    tmp_->type_to_object_[evaluator::lennard_jones::type_label_()] = &evaluator::lennard_jones::make_evaluator;
    tmp_->type_to_object_[evaluator::coulomb::type_label_()] = &evaluator::coulomb::make_evaluator;
    boost::shared_ptr< core::input_base_meta > delegate (tmp_.release());
    decoder.add_input_delegate (delegate);
  }
  {
    // Add input decoder class for the observable types. This handles the instantiation
    // of the requested observables
    std::unique_ptr< observable::sampler_meta > tmp_ (new observable::sampler_meta);
    tmp_->type_to_object_[observable::d3_distribution::type_label_()]= &observable::d3_distribution::make_sampler;
    tmp_->type_to_object_[observable::metropolis_sampler::type_label_()]= &observable::metropolis_sampler::make_sampler;
    tmp_->type_to_object_[observable::rdf_sampler::type_label_()]= &observable::rdf_sampler::make_sampler;
    tmp_->type_to_object_[observable::specie_count::type_label_()]= &observable::specie_count::make_sampler;
    tmp_->type_to_object_[observable::widom::type_label_()]= &observable::widom::make_sampler;
    tmp_->type_to_object_[observable::malasics_igcmc::type_label_()]= &observable::malasics_igcmc::make_sampler;
    tmp_->type_to_object_[observable::lamperski_igcmc::type_label_()]= &observable::lamperski_igcmc::make_sampler;
    tmp_->type_to_object_[observable::accept_igcmc::type_label_()]= &observable::accept_igcmc::make_sampler;
    boost::shared_ptr< core::input_base_meta > delegate (tmp_.release());
    decoder.add_input_delegate (delegate);
  }
  {
    // Add input decoder class for defining the different choices to use in the
    // simulation
    std::unique_ptr< trial::choice_meta > tmp_ (new trial::choice_meta);
    tmp_->type_to_object_["move"] = &trial::move_choice::make_choice;
    tmp_->type_to_object_["gc"] = &trial::add_choice::make_choice;
    tmp_->type_to_object_["jump"] = &trial::jump_choice::make_choice;
    tmp_->type_to_object_["accept"] = &trial::accept_choice::make_choice;
    boost::shared_ptr< core::input_base_meta > delegate (tmp_.release());
    decoder.add_input_delegate (delegate);
  }
  // NOTE: The input decoders are not required to check that the final system is
  // valid.
  if (argc > 1)
  {
    namespace po = boost::program_options;
    po::options_description cmdln_opts("Allowed Options");
    cmdln_opts.add_options()
      ("run", po::value<unsigned int>(), "Simulation run number.")
      (core::strngs::inputpattern_label().c_str(), po::value<std::string>(), "Plain or printf style format string for generating input filename")
      ("seed", po::value<unsigned int>(), "Simulation random seed value.")
      ("restart", po::value<std::string>(), "Check point file for simulation restart")
      ("help", "Produce help message");
    po::variables_map vm;
    try
    {
      po::store(po::parse_command_line(argc, argv, cmdln_opts), vm);
      po::notify(vm);
    }
    catch (std::exception &err)
    {
      std::cout << "Error : " << err.what() << "\n";
      std::cout << cmdln_opts << "\n";
      return 1;
    }

    if (vm.count("help"))
    {
      std::cout << cmdln_opts << "\n";
      return 1;
    }

    if (vm.count("restart"))
    {
      if (vm.count("seed"))
      {
        std::cout << "Random seed value on command line is ignored with restart.\n";
      }
      if (vm.count(core::strngs::inputpattern_label()))
      {
        std::cout << "Input filename on command line is ignored with restart.\n";
      }
      if (vm.count("run"))
      {
        std::cout << "Run number on command line is ignored with restart.\n";
      }

      std::ifstream store (vm["restart"].as<std::string>());
      boost::archive::text_iarchive ia(store);
      ia >> *(appl.get());
      if (not utility::fp_env::env_.no_except())
      {
        std::cout << " Floating point exception during deserialization ignored : "
                << utility::fp_env::env_.error_message () << "\n";
        utility::fp_env::env_.reset();
      }
      appl->description (std::cout);
    }
    else
    {
      if (vm.count(core::strngs::inputpattern_label()))
      {
        appl->set_filename_base (vm[core::strngs::inputpattern_label()].as<std::string>());
      }
      if (vm.count("run"))
      {
        appl->set_run_number (vm["run"].as<unsigned int>());
      }

      // initialize system having got a run number
      appl->set_uuid ();
      appl->description (std::cout);
      if (vm.count("seed"))
      {
        unsigned int seedval (vm["seed"].as<unsigned int>());
        std::cout << "Random seed value : " << seedval << ".\n";
        appl->set_random_seed (seedval);
      }
      else
      {
        unsigned int seedval (random_seed_value ());
        std::cout << "Random seed value : " << seedval << ".\n";
        appl->set_random_seed (seedval);
      }
      std::string fn (appl->find_input_filename());
      std::cout << core::strngs::horizontal_bar () << "\n";
      std::cout << "Reading input file '"<<fn<<"'\n";
      decoder.read_input_file (*appl, fn);
      std::cout << core::strngs::horizontal_bar () << "\n";
      decoder.write_input_file(std::cout);
      std::cout << core::strngs::horizontal_bar () << "\n";
    }
  }
  else
  {
    // initialize system
    appl->set_uuid ();
    appl->description (std::cout);
    {
      unsigned int seedval (random_seed_value ());
      std::cout << "Random seed value : " << seedval << ".\n";
      appl->set_random_seed (seedval);
    }
    std::string fn (appl->find_input_filename());
    std::cout << core::strngs::horizontal_bar () << "\n";
    std::cout << "Reading input file '"<<fn<<"'\n";
    decoder.read_input_file (*appl, fn);
    std::cout << core::strngs::horizontal_bar () << "\n";
    decoder.write_input_file(std::cout);
    std::cout << core::strngs::horizontal_bar () << "\n";
  }

  appl->main();
  return 0;
}

