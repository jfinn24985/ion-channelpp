#ifndef IONCH_PLATFORM_SERIAL_STORAGE_HPP
#define IONCH_PLATFORM_SERIAL_STORAGE_HPP

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

#include "platform/storage_manager.hpp"
#include <string>
#include <iostream>
#include <cstddef>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace core { class input_document; } 
namespace core { class input_parameter_memo; } 
namespace platform { class simulation; } 
namespace observable { class base_sink; } 
namespace core { class input_base_reader; } 
namespace platform { class storage_meta; } 

namespace platform {

// Manage access to data sources and sinks in a serial
// context. This uses no special handling of input and
// output data and access the local filesystem.

class serial_storage : public storage_manager
 {
   public:
    serial_storage();

    virtual ~serial_storage();


   private:
    // no move
    serial_storage(serial_storage && source);

    // no copy
    serial_storage(const serial_storage & source);

    // no assignment
    serial_storage & operator=(const serial_storage & source);


  friend class boost::serialization::access;    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
      {
        ar & boost::serialization::base_object< platform::storage_manager >( *this );
      }

   public:
    // A word used to idenitfy the storage manager type in the input file.
    std::string get_label() const override
    {
      return serial_storage::type_label_();
    }


   private:
    // Details about the current storage object.
    void do_description(std::ostream & os) const override;

    // Write extra data of derived simulator object into the input
    // document object at the given section.
    void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Throw an exception as this manager takes no extra
    // parameters.
    void set_parameters(const std::vector< core::input_parameter_memo >& params) override;

    // Make a unique seed for a simulation based on the given seed value.
    //
    // For serial simulations this method should be a no-op. For 
    // parallel simulations it should produce a unique and
    // reproducable seed value for each simulation.
    virtual std::size_t compute_seed(std::size_t seed) const
    {
      return seed;
    }

    // Computer the output directory for the current simulator object
    // from the output_dir_fmt template.
    std::string compute_output_dir() const override;

    virtual bool main(int argc, char ** argv, simulation & sim) override;

    // Create/open the simulation's output sink.
    //
    // If extension is not given then a sink of type gdbm_sink
    // is returned.
    boost::shared_ptr< observable::base_sink > open_output() override;

    // Create/open the simulation's input stream.
    //
    // This uses find_input_filename to identify the
    // input file then opens it using a input_reader
    // type reader object.
    //
    // \throw as per find_input_filename
    boost::shared_ptr< core::input_base_reader > open_input() override;

    // Create/open the simulation's checkpoint stream.
    boost::shared_ptr< std::ostream > open_checkpoint() override;


   private:
    // Create the system's log output stream. In this case
    // the log file is to standard out.
    boost::shared_ptr< std::ostream > open_log() override;


   public:
    static void add_definition(storage_meta & meta);

    // The string "local" used to identify this storage
    // manager in the input file.
    static std::string type_label_();


};

} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( platform::serial_storage );
#endif
