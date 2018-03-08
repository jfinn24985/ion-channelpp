#ifndef IONCH_PLATFORM_IMC_UPDATE_HPP
#define IONCH_PLATFORM_IMC_UPDATE_HPP


#include <iostream>
#include <cstddef>
#include "utility/archive.hpp"


namespace core { class input_document; } 
namespace platform { class simulation; } 

namespace platform {

class imc_update
 {
   public:
    // Log message descibing the itearation update and its parameters
    virtual void description(std::ostream & os) const = 0;

    // Add content of update into IMC section of an input file section.
    //
    //only throw possible should be from os.write() operation
    virtual void do_write_part_document(core::input_document & wr, std::size_t ix) const = 0;

    //Prepare the updater before running a series of simulation steps
    virtual void prepare(simulation & sim) = 0;

    // Perform update after each IMC cycle.
    virtual void update(simulation & sys, std::ostream & oslog) = 0;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {}


};

} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( platform::imc_update );
#endif
