#ifndef IONCH_OBSERVABLE_OUTPUT_ROWS_HPP
#define IONCH_OBSERVABLE_OUTPUT_ROWS_HPP

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

#include <iostream>
#include "utility/unique_ptr.hpp"
#include "observable/output_file.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <string>
#include <cstddef>

namespace observable { class base_sink; } 
namespace observable { struct output_datum; } 

namespace observable {

// Base class for a row in a output file.
class output_row
 {
   public:
    output_row() = default;

    virtual ~output_row() {}


   private:
    // no move
    output_row(output_row && source) = delete;

    // No copy
    output_row(const output_row & source) = delete;

    // no assignment
    output_row & operator=(const output_row & source) = delete;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {}


   public:
    // Output a space separated list of the field labels.
    virtual void labels(std::ostream & os) const = 0;

    // Output a space separated list of the field units.
    virtual void units(std::ostream & os) const = 0;

    // Output a space separated list of the field entries.
    virtual void row(std::ostream & os) const = 0;

    // Merge the given row into this row.
    virtual void merge(std::unique_ptr< output_row > source) = 0;


};
// Manage data in a user-controlled manner. When the data does not meet
// the requirements for the output_series type and the user wants to be able
// to merge data you need to use this type. To do this you need to define a
// subclass of the output_row object that performs the desired merge
// operation, as well as supplying the formatted output lines.
class output_rows : public output_dataset
 {
   private:
    // The rows of data.
    boost::ptr_vector< output_row > rows_;

    // If true then the data set is a series of estimate_arrays,
    // one for each report cycle.
    bool is_serial_;

    // Serialization constructor
    output_rows()
    : output_dataset()
    , rows_()
    , is_serial_()
    {}


   public:
    // Main constructor
    output_rows(std::string label, bool serial)
    : output_dataset( label )
    , rows_()
    , is_serial_( serial )
    {}

    // Simple dtor
    ~output_rows();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_dataset >(*this);
      ar & rows_;
      ar & is_serial_;
    }


   public:
    // Whether to replace or append the buffer to the file
    bool is_serial() const
    {
      return this->is_serial_;
    }

    // Create file if it doesn't exist and write the header and body. If
    // the file exists append new data.
    void write(base_sink & sink) const;

    // Attempt to accept a datum.
    void accept(std::unique_ptr< output_datum > datum) override;

    // Capture data.
    //
    // \pre arr /= nul
    void receive_data(std::size_t count, std::unique_ptr< output_row > arr);


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::output_rows );
#endif
