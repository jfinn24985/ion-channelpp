#ifndef IONCH_PLATFORM_STORAGE_MANAGER_TEST_HPP
#define IONCH_PLATFORM_STORAGE_MANAGER_TEST_HPP

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
#include <cstddef>
#include <iostream>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace platform { class serial_storage; } 
namespace platform { class simulation; } 
namespace platform { class simulation_definition; } 
namespace platform { class simulation_manager; } 
namespace platform { class simulator_meta; } 
namespace platform { class standard_simulation; } 
namespace platform { class storage_meta; } 
namespace core { class input_document; } 
namespace observable { class base_sink; } 
namespace core { class input_base_reader; } 
namespace core { class input_parameter_memo; } 

namespace platform {

// Unit test for the storage_manager and derived classes.
//
// Includes: storage_manager, serial_storage, storage_definition, storage_meta
class storage_manager_test
 {
   public:
    // Simple derived class version of storage manager. 
    //
    // NOTE: Abstract virtual functions are declared but not
    // defined (so can not be called!).
    class simple_storage_manager : public storage_manager
     {
       private:
        std::string param_;


       public:
        // Computer the output directory for the current simulator object
        // from the output_dir_fmt template.
        std::string compute_output_dir() const override;

        // Make a unique seed for a simulation based on the given seed value.
        //
        // For serial simulations this method should be a no-op. For 
        // parallel simulations it should produce a unique and
        // reproducable seed value for each simulation.
        virtual std::size_t compute_seed(std::size_t seed) const
        {
          return seed;
        }


       private:
        // Details about the current storage object.
        void do_description(std::ostream & os) const override;

        // Write extra data of derived simulator object into the input
        // document object at the given section.
        void do_write_document(core::input_document & wr, std::size_t ix) const override;


       public:
        // A word used to idenitfy the storage manager type in the input file.
        std::string get_label() const override
        {
          return std::string( "simple" );
        }

        std::string get_value() const
        {
          return this->param_;
        }

        virtual bool main(int argc, char ** argv, simulation & sim) override;

        // Create/open the simulation's output sink.
        virtual boost::shared_ptr< observable::base_sink > open_output() override;

        // Create/open the simulation's input stream.
        virtual boost::shared_ptr< core::input_base_reader > open_input() override;

        // Create/open the simulation's checkpoint stream.
        boost::shared_ptr< std::ostream > open_checkpoint() override;


       private:
        // Create the system's log output stream.
        virtual boost::shared_ptr< std::ostream > open_log() override;


       public:
        // Set optional parameters (from the input file).
        void set_parameters(const std::vector< core::input_parameter_memo >& params) override;

        simple_storage_manager()
        : storage_manager()
        , param_()
        {}

        virtual ~simple_storage_manager();

    
  friend class boost::serialization::access;
       private:
        template<class Archive>
          void serialize(Archive & ar, const unsigned int version)
          {
            ar & boost::serialization::base_object< platform::storage_manager >( *this );
            ar & param_;
          }

       public:
        static void add_definition(storage_meta & meta);

        static std::string type_label_();


    };
    
    // Test input file content.
    static std::string canon_input();


};

} // namespace platform
#endif

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(platform::storage_manager_test::simple_storage_manager, "storage_manager_test::simple_storage_manager" );