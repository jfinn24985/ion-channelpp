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
//

// BEGIN EXTRA INCS
#include "utility/config.hpp"
#include "utility/require.hpp"
#include "application/application.hpp"
#include "application/simulator.hpp"
#include "core/input_delegater.hpp"
#include "application/application_meta.hpp"
#include "particle/specie_meta.hpp"
#include "application/simulator_meta.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "observable/sampler_meta.hpp"
#include "observable/base_observable.hpp"
#include "trial/choice.hpp"
#include "trial/chooser.hpp"
#include "trial/choice_meta.hpp"

#include <boost/bind.hpp>
#include <fstream>
#include <iostream>
#include <limits>
#include "core/constants.hpp"
#include "utility/machine.hpp"
//END
static const std::size_t max_input_ver (1);

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
    boost::shared_ptr< core::input_base_meta > delegate (new platform::simulator_meta(
          boost::bind(&platform::application::add_simulator, appl.get(), _1)));
    decoder.add_input_delegate (delegate);
  }
  {
    // Add input decoder class to handle the energy evaluator options.  This
    // handles instantiation of the requested evaluator. 
    boost::shared_ptr< core::input_base_meta > delegate (new evaluator::evaluator_meta(appl));
    decoder.add_input_delegate (delegate);
  }
  {
    // Add input decoder class for the observable types. This handles the instantiation
    // of the requested observables
    boost::shared_ptr< core::input_base_meta > delegate (new observable::sampler_meta(
          boost::bind(&platform::application::add_observable, appl.get(), _1)));
    decoder.add_input_delegate (delegate);
  }
  {
    // Add input decoder class for defining the different choices to use in the
    // simulation
    boost::shared_ptr< core::input_base_meta > delegate (new trial::choice_meta (
          boost::bind(&platform::application::add_choice, appl.get(), _1)));
    decoder.add_input_delegate (delegate);
  }
    // NOTE: The input decoders are not required to check that the final system is 
    // valid.
  if (argc > 1)
  {
    std::ifstream store (argv[1]);
    boost::archive::text_iarchive ia(store);
    ia >> *(appl.get());
    if (not utility::fp_env::env_.no_except())
    {
      std::cout << " Floating point exception during deserialization ignored : "
                << utility::fp_env::env_.error_message () << "\n";
      utility::fp_env::env_.reset();
    }
  }
  else
  {
    // initialize system
    std::ifstream ifs ("input.dat");
    if (not ifs)
    {
      std::cout << "No input file 'input.dat'\n";
      return 1;
    }
    std::unique_ptr< platform::bulk_simulator > sim (new platform::bulk_simulator(0,0.0));
    // The readat method currently performs what will be done by the delegator
    // when instantiating the system.
    sim->readat(ifs, *appl);
    appl->add_simulator (std::unique_ptr< platform::base_simulator >(sim.release()));
  }
  
  appl->set_uuid ();
  appl->description (std::cout);
  appl->main();
  return 0;
}
