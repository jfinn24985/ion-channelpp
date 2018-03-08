#ifndef IONCH_GEOMETRY_DISTANCE_CALCULATOR_HPP
#define IONCH_GEOMETRY_DISTANCE_CALCULATOR_HPP

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

#include <boost/noncopyable.hpp>

#include <boost/serialization/vector.hpp>
#include <cstddef>

// manual decl
#include "utility/utility.hpp"
//-
namespace geometry { class coordinate; } 
namespace geometry { class coordinate_set; } 

namespace geometry {

// Abstract class to allow friend relation between geometry_manager and
// distance calculation methods.
class distance_calculator : public boost::noncopyable
 {
  friend class geometry_manager;

  friend class boost::serialization::access;
   private:
    //Write the grid parameters to an archive
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {}


   protected:
    // Calculate distance between the given position and
    // all positions in the given coordinate set. The
    // result vector will contain the distances in the
    // same sequence as the coordinate set.
    //
    // NOTE: This is hidden because it should
    // only be called throw a geometry_manager
    // object.
    //
    // \pre startindex < endindex
    // \pre endindex <= others.size
    // \post result.size == end_index
    // \post result[0:start_index] == 0.0
    void calculate_distances(const coordinate & pos, const coordinate_set & others, std::vector< double > & result, std::size_t startindex, std::size_t endindex) const;
    // Calculate distance between the given position and
    // all positions in the given coordinate set. The
    // result vector will contain the distances in the
    // same sequence as the coordinate set.
    //
    // NOTE: This is hidden because it should
    // only be called throw a geometry_manager
    // object.
    //
    // \pre startindex < endindex
    // \pre endindex <= others.size
    // \post result.size == end_index
    // \post result[0:start_index] == 0.0
    void calculate_distances_sq(const coordinate & pos, const coordinate_set & others, std::vector< double > & result, std::size_t startindex, std::size_t endindex) const;

   private:
    // Calculate distance between the given position and
    // all positions in the given coordinate set. The
    // result vector will contain the distances in the
    // same sequence as the coordinate set.
    //
    // \post result.size == end_index
    // \post result[0:start_index] == 0.0
    virtual void do_calculate_distances_sq(const coordinate & pos, const coordinate_set & others, std::vector< double > & result, std::size_t startindex, std::size_t endindex) const = 0;


   protected:
    // Calculate square of distance between two points.
    //
    // NOTE: This is hidden because it should
    // only be called throw a geometry_manager
    // object.
    virtual double calculate_distance_squared(const coordinate & pos1, const coordinate & pos2) const = 0;

    // Increase or decrease the system dimensions to match the new
    // volume. Derived classes each have rules to decide how the
    // region's dimensions change to accomodate the new volume. For
    // example a cube simply adjusts one length while a cylinder must
    // distribute the change over the radial and linear dimensions.
    //
    // \pre rad >= 0 
    // \pre vol >= 0 
    // \post volume(rad) == vol ** may be unchecked **
    //
    // \pre AFTER VOLUME CHANGE fits(rad) : It is considered an
    // argument error if after changing the volume the system can
    // not fit an object of the given radius. ** should only checked 
    // where it may fail **
    void change_volume(double vol, double rad)
    {
      UTILITY_CHECK( not (vol < 0), "A negative volume is invalid." );
      UTILITY_CHECK( not (rad < 0), "A negative radius is invalid." );
      this->do_change_volume( vol, rad );
    }

   private:
    // Increase or decrease the system dimensions to match the new
    // volume. Derived classes each have rules to decide how the
    // region's dimensions change to accomodate the new volume. For
    // example a cube simply adjusts one length while a cylinder must
    // distribute the change over the radial and linear dimensions.
    //
    // * NOTE : This is expected to be used only during system
    // initialization, ie before checking particle positions or
    // asking for new particle positions. Whether any check or
    // particle postion calculated before calling this method is
    // still valid is undefined.
    
    virtual void do_change_volume(double vol, double rad) = 0;


};

} // namespace geometry
#endif
