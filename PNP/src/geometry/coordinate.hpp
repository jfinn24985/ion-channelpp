#ifndef IONCH_GEOMETRY_COORDINATE_HPP
#define IONCH_GEOMETRY_COORDINATE_HPP

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

// Default inc
#include "utility/utility.hpp"
#include "utility/archive.hpp"
// End
#include "utility/archive.hpp"

#include <cstddef>
#include <iostream>
#include <string>

namespace core { class input_base_reader; } 
namespace core { class base_input_parameter; } 

namespace geometry {

class coordinate
 {
   public:
    double x;

    double y;

    double z;

    //C++ default constructor
    coordinate(): x(0.0), y(0.0), z(0.0) {}
    //3 Arg C++ constructor
    coordinate(double a, double b, double c)
    : x(a)
    , y(b)
    , z(c)
    {}

    ~coordinate() = default;
    coordinate(const coordinate & source)
    : x(source.x)
    , y(source.y)
    , z(source.z)
    {}

    coordinate(coordinate && source)
    : x( std::move( source.x ) )
    , y( std::move( source.y ) )
    , z( std::move( source.z ) )
    {}

    coordinate & operator=(coordinate source)
    {
      this->swap (source);
      return *this;
    }


  friend class boost::serialization::access;    //Read/write object to an archive.
    template< class Archive > void serialize(Archive & ar, std::size_t a_ver)
    {
      ar & x; ar & y; ar & z;
    }

    void swap(coordinate & source)
    {
      std::swap (x, source.x);
      std::swap (y, source.y);
      std::swap (z, source.z);
    }

    // Read from stream.
    //
    // Not robust enough for interpreting user input.
    //
    
    void read(std::istream & is)
    {
      is >> this->x >> this->y >> this->z;
    }
    //Equality test
    //
    //(is __eq__ for python)
    bool equivalent(coordinate const& rhs) const
    {
      return (this == &rhs) or (x == rhs.x and y == rhs.y and z == rhs.z);
    }

    // Write to stream
    //
    // (is __str__ for python)
    //
    // Not robust enough for interpreting user input.
    void write(std::ostream & os) const
    {
      os << this->x << " " << this->y << " " << this->z;
    }
    

    //Get x, y or z by index; 0, 1 or 2
    //respectively.
    double operator[](std::size_t idx)
    {
      switch(idx)
      {
        case 0: return this->x; 
        case 1: return this->y; 
        case 2: return this->z;
        default: UTILITY_ALWAYS(idx < 3, "Coordinate only has three index positions");
          return 0.0; // to stop complaint about no return value
      }
    }

    // Attempt to read an input value as a floating point number
    //
    // \param name : input parameter name
    // \param value : value in input file
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    // \parma above_zero : whether value should be positive
    // \param equal_zero : if above_zero is true, allows value to also equal zero 
    
    static void coordinate_input(std::string name, std::string value, std::string title, std::string section_name, coordinate & target, const core::input_base_reader * reader);

    // Attempt to read an input value as a coordinate (x,y,z)
    //
    // \param param : input parameter
    // \param title : text giving class context for error, should start with capital.
    // \param section_name : section context for error
    // \param target : where to put the result, unchanged on error
    
    static void coordinate_input(const core::base_input_parameter & param, std::string title, std::string section_name, coordinate & target);

    // Indicate that a coordinate consists of three numeric values.
    static std::string set_of_three_error()
    {
      return std::string( "Ensure coordinate data has exactly three numeric values." );
    }
    friend bool operator==(const coordinate & lhs, const coordinate & rhs)
    {
      return lhs.equivalent( rhs );
    }
     
    friend bool operator!=(const coordinate & lhs, const coordinate & rhs)
    {
      return not lhs.equivalent( rhs );
    }
     
    friend std::ostream& operator<<(std::ostream & os, const coordinate & rhs)
    {
      rhs.write( os );
      return os;
    }

    friend std::istream& operator>>(std::istream & is, coordinate & rhs)
    {
      rhs.read( is );
      return is;
    }


};

} // namespace geometry
#endif
