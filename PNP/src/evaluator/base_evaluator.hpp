#ifndef IONCH_EVALUATOR_BASE_EVALUATOR_HPP
#define IONCH_EVALUATOR_BASE_EVALUATOR_HPP

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

#include <boost/noncopyable.hpp>

#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"
#include "utility/archive.hpp"

#include <iostream>
#include <string>
#include <cstddef>

namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace particle { class change_set; } 
namespace evaluator { class evaluator_manager; } 
namespace core { class input_document; } 

namespace evaluator {

//Calculate the value/cost of the transformation on the ensemble of a
//given change_set.
class base_evaluator : public boost::noncopyable
 {
   public:
    virtual ~base_evaluator() {}


  friend class boost::serialization::access;
   private:
    //Serialize all but the observable set.
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
      };


   public:
    //  Compute the change in potential energy.
    virtual void compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const = 0;

    // Calculate the total potential energy. In general this is the sum
    virtual double compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const = 0;

    //Log message descibing the evaluator and its parameters
    void description(std::ostream & os) const;


   private:
    // Log message descibing the derived evaluator and its parameters
    virtual void do_description(std::ostream & os) const = 0;


   public:
    //  Called after the trial is complete. (default do nothing)
    virtual void on_conclude_trial(const particle::change_set & changes) {}

    //Prepare the evaluator for use with the given simulator and
    //stepper.
    virtual void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman) = 0;

    virtual std::string type_label() const = 0;

    // Add an input file section.
    //
    // only throw possible should be from os.write() operation
    //
    // The output of this factory method is made up like
    //
    // evaluator
    // type <type_label()>
    // <call do_write_input_section>
    // end
    
    void write_document(core::input_document & wr) const;


   private:
    // Add derived content to input file section wr[ix]. 
    //
    // only throw possible should be from os.write() operation
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const = 0;


};

} // namespace evaluator
#endif
