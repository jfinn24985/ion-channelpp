#ifndef IONCH__OUTPUT_TEST_SUITE_HPP
#define IONCH__OUTPUT_TEST_SUITE_HPP

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

#include "observable/output_datum.hpp"
#include <string>
#include "observable/output_rows.hpp"
#include <iostream>
#include "utility/unique_ptr.hpp"

namespace observable { class archive_file; } 
namespace observable { class combined_output; } 
namespace observable { class gdbm_sink; } 
namespace observable { class memory_sink; } 
namespace observable { struct output_buffer_datum; } 
namespace observable { class output_dataset; } 
namespace observable { class output_field; } 
namespace observable { struct output_row_datum; } 
namespace observable { class output_series; } 
namespace observable { struct output_series_datum; } 
namespace observable { class output_text; } 
namespace observable { class widom_row; } 
namespace observable { class output_rows; } 
namespace observable { class base_sink; } 

class output_test_suite
 {
   public:
    class output_test_datum : public observable::output_datum
     {
       public:
        // serialization only
        output_test_datum()
        : output_datum()
        {}

        virtual ~output_test_datum() {}

        // A generic buffer for transferring information.
        static std::string buffer;

    
  friend class boost::serialization::access;
       private:
        template<class Archive>
          inline void serialize(Archive & ar, const unsigned int version)
        {
          ar & boost::serialization::base_object< output_datum >(*this);
        }


       public:
        virtual void visit(observable::output_series & sink) override;

        virtual void visit(observable::output_text & sink) override;

        virtual void visit(observable::output_rows & sink) override;


    };
    
    class output_test_row : public observable::output_row
     {
       public:
        // serialization only
        output_test_row()
        : output_row()
        , buffer()
        {}

        virtual ~output_test_row() {}

        // A generic buffer for transferring information.
        std::string buffer;

    
  friend class boost::serialization::access;
       private:
        template<class Archive>
          inline void serialize(Archive & ar, const unsigned int version)
        {
          ar & boost::serialization::base_object< output_row >(*this);
          ar & buffer;
        }


       public:
        // Output a space separated list of the field labels.
        virtual void labels(std::ostream & os) const override;

        // Output a space separated list of the field units.
        virtual void units(std::ostream & os) const override;

        // Output a space separated list of the field entries.
        virtual void row(std::ostream & os) const override;

        // Merge the given row into this row.
        virtual void merge(std::unique_ptr< observable::output_row > source) override;


    };
    
    static void base_output_field_test(std::unique_ptr< observable::output_field > fld);























    //Test output_row base class features.
    static void base_output_row_test(std::unique_ptr< observable::output_row > data);





    //Test output_datum base class features.
    static void base_output_datum_test(std::unique_ptr< observable::output_datum > data, int16_t usewith);









    //Test output_dataset base class features.
    static void base_output_dataset_test(std::unique_ptr< observable::output_dataset > data);













    //Test generic operations on a sink object
    static void test_base_sink_method(observable::base_sink & sink);


};
#endif
