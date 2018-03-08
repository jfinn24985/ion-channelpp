#ifndef IONCH_GEOMETRY_CENTROID_HPP
#define IONCH_GEOMETRY_CENTROID_HPP

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

#include "utility/archive.hpp"

#include <iostream>
#include "geometry/coordinate.hpp"

namespace geometry {

// Information about the centrepoints of localised atoms
//
// (x,y,z coordinates and spring constant/max displacement parameter r)
class centroid
 {
   public:
    // Radial factor of localisation potential (not that the fortran version
    // stores this value as the square of the value in the input to avoid
    // squaring the value when required. This is not done here and the value
    // must be squared when used.)
    double r;

    //X-coord of localisation centre point
    double x;

    //Y-coord of localisation centre point
    double y;

    //Z-coord of localisation centre point
    double z;

    // Write to stream.
    //
    // Not robust enough for interpreting user input.
    void write(std::ostream & os) const
    {
      os << this->r << " " << this->x << " " << this->y << " " << this->z;
    }

    // Read from stream (consumes four real numbers).
    //
    // Not robust enough for interpreting user input.
    void read(std::istream & is)
    {
      is >> this->r >> this->x >> this->y >> this->z;
    }


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & this->x;
      ar & this->y;
      ar & this->z;
      ar & this->r;
    }


   public:
    centroid(double ar, double ax, double ay, double az)
    : r(ar)
    , x(ax)
    , y(ay)
    , z(az)
    {}

    centroid()
    : r(0.0)
    , x(0.0)
    , y(0.0)
    , z(0.0)
    {}

    ~centroid() = default;

    centroid(const centroid & source) = default;

    centroid(centroid && source) = default;

    centroid & operator=(centroid source)
    {
      this->swap( source );
      return *this;
    }

    void swap(centroid & other)
    {
      std::swap( this->r, other.r );
      std::swap( this->x, other.x );
      std::swap( this->y, other.y );
      std::swap( this->z, other.z );
    }

    // Get centroid position as a coordinate
    coordinate position() const
    {
      return coordinate( this->x, this->y, this->z );
    }

    bool equivalent(const centroid & other) const
    {
      return this->r == other.r and this->x == other.x and this->y == other.y and this->z == other.z;
    }


};

static inline bool operator==(centroid const& a, centroid const& b)
{
  return a.equivalent(b);
}

static inline bool operator!=(centroid const& a, centroid const& b)
{
  return not a.equivalent(b);
}

static inline std::ostream &operator<<(std::ostream &os, centroid const& a)
{
  a.write( os );
  return os;
}

static inline std::istream &operator>>(std::istream &is, centroid & a)
{
  a.read( is );
  return is;
}


} // namespace geometry
#endif
