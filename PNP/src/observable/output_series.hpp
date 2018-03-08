#ifndef IONCH_OBSERVABLE_OUTPUT_SERIES_HPP
#define IONCH_OBSERVABLE_OUTPUT_SERIES_HPP

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

#include "observable/output_file.hpp"
#include "utility/estimate_array.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <string>
#include <iostream>
#include <cstddef>
#include "utility/unique_ptr.hpp"

namespace core { class strngs; } 
namespace observable { class output_field; } 
namespace observable { class base_sink; } 
namespace observable { struct output_datum; } 

namespace observable {

// Manage data that can be manipulated as a set of estimate_array objects.
// Many observations involve sampling a distribution that can be digitized
// in some way. The main condition that has to be met for such a set of
// observations is that all data are sampled the same number of times. It
// can be organised into an array or a series of arrays. A single array is 
// output vertically. A series of arrays is output as a matrix. A series can
// be a time-series, incrementing each time data is received or of a fixed
// size. New data can be merged with existing data.
class output_series : public output_dataset
 {
   public:
    typedef boost::ptr_vector< output_field >::const_iterator const_iterator;


   private:
    // The sampled results.
    boost::ptr_vector< utility::estimate_array > data_;

    boost::ptr_vector< output_field > entries_;

    // If true then the data set is a series of estimate_arrays,
    // one for each report cycle.
    bool is_serial_;

    // Serialization constructor
    output_series()
    : output_dataset()
    , data_()
    , entries_()
    , is_serial_()
    {}


   public:
    // Main constructor
    output_series(std::string label, bool serial)
    : output_dataset( label )
    , data_()
    , entries_()
    , is_serial_( serial )
    {}

    // Simple dtor
    virtual ~output_series();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_dataset >(*this);
      ar & data_;
      ar & entries_;
      ar & is_serial_;
    }


   public:
    // Iterator to first field
    const_iterator begin() const
    {
      return this->entries_.begin();
    }

    // Iterator to terminate loop over fields
    const_iterator end() const
    {
      return this->entries_.end();
    }

    // Write the data section of the output file
    //
    // \pre not empty
    virtual void do_array_body(std::ostream & os) const;

    // Write the data section of the output file
    //
    // \pre not empty
    virtual void do_series_body(std::ostream & os) const;

    // Write the header of the data file
    //
    // \pre not empty
    void do_header(std::ostream & os) const;

    // Check is there are any data fields defined.
    bool empty() const
    {
      return this->entries_.empty();
    }

    // Get the title of the dataset
    bool is_serial() const
    {
      return this->is_serial_;
    }

    // Check is there are any data fields defined.
    std::size_t size() const
    {
      return this->entries_.size();
    }

    // Create file if it doesn't exist and write the header and body. If
    // the file exists append new data.
    //
    // \pre not empty
    void write(base_sink & sink) const;

    // Attempt to accept a datum.
    void accept(std::unique_ptr< output_datum > datum) override;

    // Capture data.
    //
    // \pre arr /= nul
    void receive_data(std::size_t count, std::unique_ptr< utility::estimate_array > arr);

    // Add an output field to the data set row
    void push_back_field(std::unique_ptr< output_field > field)
    {
      this->entries_.push_back( field.release() );
    }


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::output_series );
#endif
