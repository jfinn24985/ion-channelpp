#ifndef IONCH__PYTHON_BINDING_HPP
#define IONCH__PYTHON_BINDING_HPP

//Python wrappings for particle namespace
//
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

#include "../src/python-binding/python_wrappers.hpp"
#include "particle/specie.hpp"
#include "geometry/coordinate.hpp"
#include "particle/ensemble.hpp"
#include "platform/simulator.hpp"
#include "evaluator/coulomb.hpp"
#include "observable/base_observable.hpp"
#include "trial/change_set.hpp"
#include <boost/python.hpp>
#include <sstream>
using namespace boost::python;
using namespace particle;
using namespace platform;
using namespace evaluator;
using namespace trial;
using namespace observable;
using namespace utility;

particle::specie& (simulator::*get_specie_const)(size_t) = &simulator::get_specie;

namespace {

void specie_description(specie const& sp, object &f)
{ 
  std::stringstream os;
  sp.description(os);
  f.attr("write")(os.str());
}

void ensemble_description(ensemble const& ens, object &f)
{ 
  std::stringstream os;
  ens.description(os);
  f.attr("write")(os.str());
}
}

BOOST_PYTHON_MODULE(csc)
{
class_< coordinate_py >("coordinate_py")
    .def_readwrite("x",&coordinate::x)

    .def_readwrite("y",&coordinate::y)

    .def_readwrite("z",&coordinate::z)

    .def("__getitem__", &coordinate::operator[])

    .def(init<double, double, double>())

;

//Python wrapper of trial::change_atom
class_< change_atom_py >("change_atom_py")
    .def_readwrite("do_new",&change_atom::do_new)

    .def_readwrite("do_old",&change_atom::do_old)

    .def_readwrite("energy_old",&change_atom::energy_old)

    .def_readwrite("energy_new",&change_atom::energy_new)

    .def_readwrite("key",&change_atom::key)

    .def_readwrite("index",&change_atom::index)

    //.def("__getitem__", &coordinate::operator[])

;

//Python wrapper of trial::change_atom
class_< change_set_py, boost::noncopyable >("change_set_py")
    .add_property("accept",&change_set::accept,&change_set::set_accept)

    .add_property("exponential_factor", &change_set::exponential_factor)

    .add_property("fail", &change_set::fail)

    .add_property("probability_factor",&change_set::probability_factor)

    .def("__getitem__", &change_set::operator[], return_internal_reference<>())

    // .def("choice", &change_set::choice, return_internal_reference<>())

    .def("energy", &change_set::energy)

    .def("set_fail", &change_set::set_fail)

    .def("__len__", &change_set::size)

    .def("update_exponential_factor", &change_set::update_exponential_factor)

    .def("update_probability_factor", &change_set::update_probability_factor)

;

class_< specie_py >("specie_py")
    .def("chemical_potential", &specie::chemical_potential)

    .def("increment", &specie::increment)

    .def("decrement", &specie::decrement)

    //Expose excess potential as a property
    .add_property("concentration", &specie::concentration, &specie::set_concentration)

    //Expose count as a property
    .add_property("count", &specie::count, &specie::set_count)

    //Expose excess potential as a property
    .add_property("excess_potential", &specie::excess_potential, &specie::set_excess_potential)

    .def("label", &specie::get_label)

    .def("set_label", &specie::set_label)

    //Expose excess potential as a property
    .add_property("radius", &specie::radius, &specie::set_radius)

    //Expose excess potential as a property
    .add_property("valency", &specie::valency, &specie::set_valency)

    .def("sub_type", &specie::sub_type)

    .def("set_type", &specie::set_type)

    .def("description", &specie_description)

;

//Python reflection of evalutor::base_evaluator class
class_< evaluator_wrapper, boost::noncopyable>("base_evaluator")
    .def("compute_potential", pure_virtual(&base_evaluator::compute_potential))

    //.def("description", pure_virtual(&base_evaluator::description))

    .def("prepare", pure_virtual(&base_evaluator::prepare))

    .def("type_label", pure_virtual(&base_evaluator::type_label))

;

//Python reflection of evalutor::coulomb class
class_< coulomb_py >("coulomb_py", no_init)
    .def("compute_potential", &coulomb::compute_potential)

    //.def("description", &coulomb::description)

    .def("prepare", &coulomb::prepare)

    .def("type_label", &coulomb::type_label)

;

//Python reflection of platform::simulator class
class_< simulator_py, boost::noncopyable >("simulator_py", no_init)
    //.def("description", &simulator_description)

    .def("do_enrol", &simulator::do_enrol)

    .def("get_ensemble", &simulator::get_ensemble, return_internal_reference<>())

    .def("get_specie", get_specie_const, return_internal_reference<>())

    .def("get_specie_key", &simulator::get_specie_key)

    .def("has_signal", &simulator::has_signal)

    .def("has_specie", &simulator::has_specie)

    .def("initialize", &simulator::initialize)

    .def("ionic_strength", &simulator::ionic_strength)

    .def("compute_output_dir", &simulator::compute_output_dir)

    .def("run", &simulator::run)

    // .def("run_imc", &simulator::run_imc)

    .def("set_delta", &simulator::set_delta)

    .def("set_equilibration_interval", &simulator::set_equilibration_interval)

    .def("set_inner_loop_size", &simulator::set_inner_loop_size)

    .def("set_production_interval", &simulator::set_production_interval)

    .def("set_target_particles", &simulator::set_target_particles)

    .def("set_temperature", &simulator::set_temperature)

    .def("specie_count", &simulator::specie_count)

    .def("target_number_of_particles", &simulator::target_number_of_particles)

    .def("temperature", &simulator::get_temperature)

;

//Python wrapper of trial::ensemble class
class_< ensemble_py, boost::noncopyable >("ensemble_py", no_init)
    .def("charge", &ensemble::charge)

    .def("commit", &ensemble::commit)

    .def("count", &ensemble::count)

    .def("description", &ensemble_description)

    //.def("generate_ensemble", &ensemble::generate_ensemble)

    .def("key", &ensemble::key)

    .def("max_size", &ensemble::max_size)

    .def("nth_index", &ensemble::nth_index)

    .def("nth_specie_index", &ensemble::nth_specie_index)

    .def("position", &ensemble::position)

    .def("x", &ensemble::x)

    .def("y", &ensemble::y)

    .def("z", &ensemble::z)

    .def("__len__", &ensemble::size)

;

//Python reflection of observable::base_observable class
class_< sampler_wrapper, boost::noncopyable>("sampled_observable")
    //.def("description", pure_virtual(&base_evaluator::description))

    .def("prepare", pure_virtual(&base_observable::prepare))

    .def("enrol", &base_observable::enrol, &sampler_wrapper::default_enrol)

;

//Python wrapper of utility::random_distribution
class_< random_distribution_py, boost::noncopyable >("random_distribution_py")
    .def("uniform", &random_distribution::uniform)

    .def("split_uniform", &random_distribution::split_uniform)

    .def("randint", &random_distribution::randint)

;

}
#endif
