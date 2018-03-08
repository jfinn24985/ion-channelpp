#ifndef IONCH_PLATFORM_SPECIE_MONITOR_HPP
#define IONCH_PLATFORM_SPECIE_MONITOR_HPP

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

#include "observable/specie_count.hpp"
#include <cstddef>
#include <iostream>
#include <string>

namespace core { class strngs; } 
namespace observable { class base_sink; } 
namespace core { class input_document; } 

namespace platform {

// Observe the average number of particles from periodic sampling for
// the chemical potential updater types. This can not be constructed 
// from the input file.

class specie_monitor : public observable::specie_count
 {
   public:
    // Main constructor
    specie_monitor()
    : specie_count()
    {}

    virtual ~specie_monitor();

  friend class boost::serialization::access;
    template< class Archive > void serialize(Archive & ar, std::size_t version)
    {
      ar & boost::serialization::base_object< specie_count >(*this);
    }

    //Log message descibing the observable and its parameters
    void description(std::ostream & out) const override;

    std::string get_label() const override
    {
      return std::string("specie-monitor");;
    }

    // Produces no report.
    void on_report(std::ostream & out, observable::base_sink & sink) override;


   private:
    // Remove input file section for this observer.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


};

} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( platform::specie_monitor );
#endif
