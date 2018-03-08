#ifndef IONCH_OBSERVABLE_FIELD_FORMAT_HPP
#define IONCH_OBSERVABLE_FIELD_FORMAT_HPP

//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or (at
//your option) any later version.
//
//This program is distributed in the hope that it will be useful, but
//WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------

#include "observable/output_file.hpp"
#include <boost/serialization/vector.hpp>
#include <string>
#include <iostream>
#include <cstddef>
#include "utility/digitizer.hpp"
#include "geometry/digitizer_3d.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "utility/unique_ptr.hpp"

namespace utility { class estimate_array; } 

namespace observable {

// Output a floating point value from a fixed array by index
class element_output : public output_field
 {
   private:
    // Set of values to output.
    std::vector<double> keys_;

    // Which index variable to use in op(), idx or rank.
    bool use_rank_;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
      ar & keys_;
      ar & use_rank_;
    }

    // Serialization only
    element_output()
    : output_field()
    , keys_()
    , use_rank_()
    {}


   public:
    // Main ctor, use_rank choses rank or index as key selector in write.
    element_output(std::string name, std::string unit, bool use_rank);

    virtual ~element_output() {}


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


   public:
    // Get key at given position, check index.
    double key(std::size_t idx) const
    {
      return this->keys_.at( idx );
    }
    // Number of keys
    std::size_t size() const
    {
      return this->keys_.size();
    }

    // Test if any keys.
    bool empty() const
    {
      return this->keys_.empty();
    }

    // Append element to list
    void push_back(double key)
    {
      this->keys_.push_back( key );
    }

    template< class IterType > void assign(IterType begin, IterType end)
    {
      this->keys_.assign( begin, end );
    }


};
// Output the current index.
class index_output : public output_field
 {

  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
    }

    // Serialization only
    index_output(): output_field() {}


   public:
    // Main ctor.
    index_output(std::string name, std::string unit);
    

    virtual ~index_output() {}


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


};
// Output a string value from a fixed array by index
class key_output : public output_field
 {
   private:
    // Set of values to output.
    std::vector<std::string> keys_;

    // Which index variable to use in op(), idx or rank.
    bool use_rank_;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
      ar & keys_;
      ar & use_rank_;
    }

    // Serialization only
    key_output()
    : output_field()
    , keys_()
    , use_rank_()
    {}
    


   public:
    // Main ctor, use_rank choses rank or index as key selector in write.
    key_output(std::string name, std::string unit, bool use_rank);

    virtual ~key_output() {}


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


   public:
    // Get key at given position, check index.
    std::string key(std::size_t idx) const
    {
      return this->keys_.at( idx );
    }
    // Number of keys
    std::size_t size() const
    {
      return this->keys_.size();
    }

    // Test if any keys.
    bool empty() const
    {
      return this->keys_.empty();
    }

    // Append key to list
    void push_back(std::string key)
    {
      this->keys_.push_back( key );
    }

    template< class IterType > void assign(IterType begin, IterType end)
    {
      this->keys_.assign( begin, end );
    }


};
// Output the mean value at the given index
class mean_output : public output_field
 {

  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
    }

    // Serialization only
    mean_output(): output_field() {}


   public:
    // Main ctor.
    mean_output(std::string name, std::string unit);
    

    virtual ~mean_output() {}

    // Does using this field consume an array element?
    // Default value is false.
    virtual bool consume_value() const override
    {
      return true;
    }


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


};
// Output the mean and variance values at the given index
class mean_variance_output : public output_field
 {

  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
    }

    // Serialization only
    mean_variance_output(): output_field() {}


   public:
    // Main ctor.
    mean_variance_output(std::string name, std::string unit);
    

    virtual ~mean_variance_output() {}

    // Does using this field consume an array element?
    // Default value is false.
    virtual bool consume_value() const override
    {
      return true;
    }


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


};
// Output the rank parameter
class rank_output : public output_field
 {

  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
    }

    // Serialization only
    rank_output(): output_field() {}


   public:
    // Main ctor.
    rank_output(std::string name, std::string unit);
    

    virtual ~rank_output() {}


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


};
// Output the sample count of the estimate array.
class sample_count_output : public output_field
 {

  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
    }

    // Serialization only
    sample_count_output(): output_field() {}


   public:
    // Main ctor.
    sample_count_output(std::string name, std::string unit);
    

    virtual ~sample_count_output() {}


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


};
// Output the variance value at the given index
class variance_output : public output_field
 {

  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
    }

    // Serialization only
    variance_output(): output_field() {}


   public:
    // Main ctor.
    variance_output(std::string name, std::string unit);
    

    virtual ~variance_output() {}

    // Does using this field consume an array element?
    // Default value is false.
    virtual bool consume_value() const override
    {
      return true;
    }


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


};
// Output bin min/mid/max values of a digitzer at the given index
class digitizer_output : public output_field
 {
   public:
    enum
     {
      USE_MIN = 1,
      USE_MID = 2,
      USE_MAX = 4,
      USE_ALL = 7

    };


   private:
    // Axis to output.
    utility::digitizer axis_;

    // Which bin values to output (as mask)
    //
    // 001 = min
    // 010 = mid
    // 100 = max
    int16_t parts_;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
      ar & axis_;
      ar & parts_;
    }

    // Serialization only
    digitizer_output()
    : output_field()
    , axis_()
    , parts_()
    {}
    


   public:
    digitizer_output(std::string name, std::string unit, utility::digitizer axis, int16_t use_which)
    : output_field( name, unit )
    , axis_( std::move( axis ) )
    , parts_( use_which )
    {}
    

    virtual ~digitizer_output() {}


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


   public:
    const utility::digitizer & axis() const
    {
      return this->axis_;
    }

    // Should we output bin minimum value
    bool use_min() const
    {
      return 0 != (this->parts_ & USE_MIN);
    }

    // Should we output bin midpoint value
    bool use_mid() const
    {
      return 0 != (this->parts_ & USE_MID);
    }

    // Should we output bin maximum value
    bool use_max() const
    {
      return 0 != (this->parts_ & USE_MAX);
    }


};
// Output bin min/mid/max values of a digitzer at the given index
class digitizer_3d_output : public output_field
 {
   public:
    enum
     {
      USE_MIN = 1,
      USE_MID = 2,
      USE_MAX = 4,
      USE_ALL = 7

    };


   private:
    geometry::digitizer_3d axis_;

    // Which bin values to output (as mask)
    //
    // 001 = min
    // 010 = mid
    // 100 = max
    int16_t parts_;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
      ar & axis_;
      ar & parts_;
    }

    // Serialization only
    digitizer_3d_output()
    : output_field()
    , axis_()
    , parts_()
    {}
    


   public:
    digitizer_3d_output(std::string name, std::string unit, geometry::digitizer_3d axis, int16_t use_which)
    : output_field( name, unit )
    , axis_( std::move( axis ) )
    , parts_( use_which )
    {}

    virtual ~digitizer_3d_output() {}


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


   public:
    const geometry::digitizer_3d & axis() const
    {
      return this->axis_;
    }

    // Should we output bin minimum value
    bool use_min() const
    {
      return 0 != (this->parts_ & USE_MIN);
    }

    // Should we output bin midpoint value
    bool use_mid() const
    {
      return 0 != (this->parts_ & USE_MID);
    }

    // Should we output bin maximum value
    bool use_max() const
    {
      return 0 != (this->parts_ & USE_MAX);
    }


};
// Combine several fields into one apparent field.
class combined_output : public output_field
 {
   private:
    // Sub fields.
    boost::ptr_vector< output_field > children_;


   public:
    // Simple default ctor
    combined_output(): output_field() {}

    ~combined_output();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_field >(*this);
      ar & children_;
    }


   public:
    // Add an output field to the data set row
    void push_back_field(std::unique_ptr< output_field > field);

    // Does using this field consume an array element?
    // Default value is false.
    virtual bool consume_value() const override;


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const override;


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::combined_output );
BOOST_CLASS_EXPORT_KEY( observable::element_output );
BOOST_CLASS_EXPORT_KEY( observable::index_output );
BOOST_CLASS_EXPORT_KEY( observable::key_output );
BOOST_CLASS_EXPORT_KEY( observable::mean_output );
BOOST_CLASS_EXPORT_KEY( observable::mean_variance_output );
BOOST_CLASS_EXPORT_KEY( observable::rank_output );
BOOST_CLASS_EXPORT_KEY( observable::sample_count_output );
BOOST_CLASS_EXPORT_KEY( observable::variance_output );
BOOST_CLASS_EXPORT_KEY( observable::digitizer_output );
BOOST_CLASS_EXPORT_KEY( observable::digitizer_3d_output );
#endif
