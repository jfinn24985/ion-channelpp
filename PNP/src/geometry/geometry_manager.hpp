#ifndef IONCH_GEOMETRY_GEOMETRY_MANAGER_HPP
#define IONCH_GEOMETRY_GEOMETRY_MANAGER_HPP

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
#include <boost/serialization/shared_ptr.hpp>
#include "geometry/base_region.hpp"
#include <cstddef>
#include <string>

namespace geometry { class coordinate; } 
namespace geometry { class coordinate_set; } 
namespace core { class input_document; } 
namespace core { class input_delegater; } 

namespace geometry {

// Manager facets of the systems geometry.
//
// The geometry of the system can be arbitrary.  It might support
// periodic boundaries in one or more dimensions or solid edges.
// The simulation cell could be any shape. Furthermore the 
// space could be divided into an arbitrary number of sub-spaces
// or _regions_. To this end the geometry manager is responsible for:
//
// * Calculation of volume(s).
//
// * Determining which region a particle is in.
//
// * Calculating distances between two points (accounting
//   for periodicity).
//
// * Managing access to regions and region nesting
class geometry_manager : public boost::noncopyable
 {
   private:
    // Regions in the simulation. The order implies no inter-region
    // relationships. The first entry will always encompass the entire
    // simulation (ie is the "system" region.) The vector will always
    // have at least this system region.
    std::vector< boost::shared_ptr< base_region > > regions_;

    // For serialization only.
    geometry_manager() = default;


   public:
    geometry_manager(boost::shared_ptr< base_region > system_region);

    ~geometry_manager();


  friend class boost::serialization::access;
   private:
    //Write the grid parameters to an archive
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
      ar & this->regions_;
    }


   public:
    // Add region to manager. If a region with the same label
    // exists in the manager, the new object replaces the old.
    void add_region(boost::shared_ptr< base_region > regn);

    // Calculate distance between the given position and
    // all positions in the given coordinate set. The
    // result vector will contain the distances in the
    // same sequence as the coordinate set.
    //
    // \pre startindex < endindex
    // \pre endindex <= others.size
    // \post result.size == end_index
    // \post result[0:start_index] == 0.0
    void calculate_distances(const coordinate & pos, const coordinate_set & others, std::vector< double > & result, std::size_t startindex, std::size_t endindex) const
    {
      this->regions_[0]->calculate_distances( pos, others, result, startindex, endindex );
    }
    // Calculate distance between the given position and
    // all positions in the given coordinate set. The
    // result vector will contain the distances in the
    // same sequence as the coordinate set.
    //
    // \pre startindex < endindex
    // \pre endindex <= others.size
    // \post result.size == end_index
    // \post result[0:start_index] == 0.0
    void calculate_distances_sq(const coordinate & pos, const coordinate_set & others, std::vector< double > & result, std::size_t startindex, std::size_t endindex) const
    {
      this->regions_[0]->calculate_distances_sq( pos, others, result, startindex, endindex );
    }
    // Calculate square of distance between two points.
    double calculate_distance_squared(const coordinate & pos1, const coordinate & pos2) const
    {
      return this->regions_[0]->calculate_distance_squared( pos1, pos2 );
    }

    // Increase or decrease the system dimensions to match the new
    // volume. Derived classes each have rules to decide how the
    // region's dimensions change to accomodate the new volume. For
    // example a cube simply adjusts one length while a cylinder must
    // distribute the change over the radial and linear dimensions.
    //
    // Pre/post-conditions from region.change_volume(...)
    //
    // \pre rad >= 0
    //
    // \pre vol >= 0
    //
    // \post volume(rad) == vol
    //
    // \pre AFTER VOLUME CHANGE fits(rad)
    //
    // * NOTE : This is expected to be used only during system
    // initialization, ie before checking particle positions or
    // asking for new particle positions. Whether any check or
    // particle postion calculated before calling this method is
    // still valid is undefined.
    
    void change_volume(double vol, double rad);

    // Get a region by index.
    //
    // \pre idx < region_count : (throws same error as vector::at)
    base_region const& get_region(std::size_t idx) const
    {
      return *(this->regions_.at(idx));
    }
    // Puts the set of regions the given point is in into
    // regns. The set may be empty.
    void locate_region(const coordinate & pos, double rad, std::vector< boost::shared_ptr< base_region > >& regns) const;

    // The number of regions
    std::size_t region_count() const
    {
      return this->regions_.size();
    }

    // Find a regions key from its label/name
    //
    // \return index if found or region_count if not found.
    std::size_t region_key(std::string label) const;

    // The number of regions defined.
    //
    // \invariant >0
    std::size_t size() const
    {
      return this->regions_.size();
    }

    // Get the region representing the entire system.
    base_region const& system_region() const
    {
      return *(this->regions_[0]);
    }

    // Get all samplers to write their input information.
    void write_document(core::input_document & wr) const;

    // Adds factory methods and input parsers for all the types that can be instantiated from 
    // the input file.
    static void build_input_delegater(boost::shared_ptr< geometry_manager > gman, core::input_delegater & delegate);


};

} // namespace geometry
#endif
