#ifndef IONCH_PARTICLE_SPECIE_HPP
#define IONCH_PARTICLE_SPECIE_HPP

//Authors: Justin Finnerty
//
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


#include "geometry/centroid.hpp"
#include "geometry/coordinate.hpp"
#include <boost/serialization/map.hpp>
#include <string>
#include <boost/serialization/vector.hpp>
#include "core/input_parameter_memo.hpp"
#include <cstddef>

//--
#include "core/constants.hpp"
#include "utility/archive.hpp"

namespace core { class input_document; } 

namespace particle {

// A specie object contains all the position
// independent information for a particle in
// an ensemble.
//
// The specie contains general information
// required for all simulations as well as a
// dictionary of information containing data
// specific for certain types of evaluators.
// It is expected that evaluators will query
// specie specific information at the beginning of
// the simulation and cache the data of interest
// to them internally.  The dictionary contains
// key:value string pairs that each evaluator must
// convert to the required type.

class specie
 {
   public:
    enum specie_type
     {
      MOBILE = 0,//Localized ion in channel
      FLEXIBLE = 1,//Localized ion
      CHANNEL_ONLY = 2,//Free within channel
      SOLUTE = 3,//Anywhere in simulation
      INVALID = 4//Non-specie flag

    };


   private:
    //Iterator type for the parameter_set
    typedef std::map< std::string , std::string >::const_iterator const_iterator;

    //Vector of parameters
    std::vector< core::input_parameter_memo > parameter_set_;

    // Cached location of particles in the ensemble.  This is updated on reading an input
    // file and when writing an input file.  At all other times the correspondence between
    // this data and that in the ensemble is undefined.
    std::vector< geometry::coordinate > locations_;

    // (Optional) data for localiser evaluator.
    std::vector< geometry::centroid > localize_data_;

    // target salt concentrations (in SI Molar (mol/l)
    double concentration_;

    //Excess chemical potential value
    double excess_potential_;

    // specie code name
    std::string label_;

    //Particle radius (default 0.0)
    double radius_;

    //  The relative probability members of this specie participate in a MC trial.
    //
    //  Note that this the one probability value that is allowed to be zero.
    //
    // Default value is 1.
    //  
    double rate_;

    //particle charge (default 0.0)
    double valency_;

    specie_type type_;


   public:
    //chemical potential in units of kT : log([conc]) - mu_ex
    //
    //(mu_ex = excess_potential_)
    double chemical_potential() const;

    // Target concentration of a specie (in SI Molar (mol/l))
    //
    // The target concentration of specie is the sum of the partial
    // target concentrations from all salts it is a component of.
    double concentration() const
    {
       return this->concentration_;
    }

    //Write a human readable summary of the specie to 'out'
    void description(std::ostream & out) const;

    //chemical potential in units of kT : log([conc]) - mu_ex
    //
    //(mu_ex = excess_potential_)
    double excess_potential() const
    {
     return this->excess_potential_;
    }

    // specie code name/label
    //
    // \pre (is_valid and result.size == 2) or result.empty
    const std::string& label() const
    {
      return this->label_;
    }

    //Radius of specie
    //
    // Always well defined
    double radius() const
    {
      return this->radius_;
    }

    //Radius of specie
    //
    // Always well defined
    double rate() const
    {
      return this->rate_;
    }

    //Charge of specie
    //
    // Always well defined
    double valency() const
    {
      return this->valency_;
    }

    // Compare if two species have the same information
    bool equivalent(const specie & other) const;

    //Test if parameter is present in this specie
    bool has_parameter(std::string name) const;

    // Is this a structural channel-only specie?
    //
    // Particles of a channel-only specie are restricted to movement
    // anywhere within the filter region.  They can move in increments
    // from the current position or jump to anywhere in the filter.
    // They can not be added or removed, not can jump into or out of
    // the channel.
    //
    // chonly species are structural ions that are _not_ 'mobile' and
    // _not_ 'flexible'.
    //
    // Always well defined
    bool is_channel_only() const
    {
       return this->type_ == CHANNEL_ONLY;
    }

    // Is this a flexible structural specie type?
    //
    // Particles of a flexible specie do move within a sphere and can
    // not be added or deleted.  Unlike the other structural ions they
    // may exist outside zlimit.
    //
    // flexible species are structural ions that are _not_ 'mobile' and
    // _not_ 'chonly'
    //
    // Always well defined
    bool is_flexible() const
    {
      return this->type_ == FLEXIBLE;
    }

    // Is this a localized structural specie type?
    //
    // Particles of a localized specie move within a sphere, and can not
    // be added or deleted. Localized species are structural ions that are
    // either 'mobile' or 'flexible'
    //
    //  Always well defined
    bool is_localized() const
    {
      return this->type_ == MOBILE or this->type_ == FLEXIBLE;
    }

    // Is this a mobile structural ion specie type?
    //
    // Particles of a mobile specie are restricted in two way, firstly
    // they must remain within the filter region and secondly they must
    // each remain within a fixed radius of a defined point.  The only
    // movement possible is small displacement moves.  The can not be
    // added or deleted from the simulation.
    //
    // mobile species are structural ions that are _not_ 'chonly' and
    // _not_ 'flexible'
    //
    // Always well defined
    bool is_mobile() const
    {
      return this->type_ == MOBILE;
    }

    // Is this a non-structural (ie free) ion?
    //
    // Particles of a free ion specie may participate in any move
    // type, be a component of a salt and be added or deleted from
    // the system.  is_solute species are _not_ structural ions
    //
    // Always well defined
    bool is_solute() const
    {
      return this->type_ == SOLUTE;
    }

    // Is this a 'valid' specie type?
    //
    // Default constructed objects are not valid
    //
    // Always well defined
    bool is_valid() const
    {
      return this->type_ != INVALID;
    }

    // Define a partial ordering of species based on type.
    //
    // The ordering is MOBILE < FLEXIBLE < CHANNEL < SOLUTE
    //
    // \pre type != INVALID 
    bool operator <(const specie & other) const;
    // Compare two species for equivalence
    friend bool operator==(const specie & lhs, const specie & rhs)
    {
       return lhs.equivalent( rhs );
    }

    // Compare two species for equivalence
    friend bool operator!=(const specie & lhs, const specie & rhs)
    {
       return not lhs.equivalent( rhs );
    }

    //Get a parameter value
    //
    //\pre has_parameter (name)
    const core::input_parameter_memo& parameter(std::string name) const;

    //Get the specie type
    //
    // Always well defined
    std::size_t sub_type() const
    {
      return this->type_;
    }

    //Convert specie type value into a string
    //
    //\pre typeval in { MOBILE, FLEXIBLE, CHANNEL_ONLY, SOLUTE or INVALID }
    //( INVALID returns non-dictionary string "invalid" which is not recognised by
    // the string_to_specie_type companion method )
    static std::string type_label(std::size_t typeval);

    // LIFETIME METHODS
    // base constructor
    specie();

    specie(const specie & other);

    specie(specie && other);

    specie & operator=(specie source)
    {
      this->swap (source);
      return *this;
    }

  friend class boost::serialization::access;

   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & parameter_set_;
        ar & locations_;
        ar & localize_data_;
        ar & concentration_;
        ar & excess_potential_;
        ar & label_;
        ar & radius_;
        ar & rate_;
        ar & valency_;
        ar & type_;
      }


   public:
    // unbuild object
    ~specie() = default;

    void swap(specie & other);

    // Target concentration of a specie (in SI Molar (mol/l))
    //
    // The target concentration of specie is the sum of the partial
    // target concentrations from all salts it is a component of.
    void set_concentration(double conc)
    {
      this->concentration_ = conc;
    }

    //Set the value of the excess chemical potential.
    void set_excess_potential(double val)
    {
      this->excess_potential_ = val;
    }

    void set_label(std::string val)
    {
      this->label_ = val;
    }

    //Set a parameter value
    //
    //\pre not has_parameter( name )
    //\post has_parameter( name )
    void set_parameter(core::input_parameter_memo param);

    void set_radius(double val)
    {
      this->radius_ = val;
    }

    void set_rate(double val)
    {
      this->rate_ = val;
    }

    void set_valency(double val)
    {
      this->valency_ = val;
    }

    void set_type(std::size_t val)
    {
      this->type_ = specie_type(val);
    }

    // Convert a text representation of a specie type (eg from input file) into 
    // the specie type enum value. Users should "dequote" and "trim" values
    // before calling this method.
    //
    // Sets 'set' to true if valid value is found, otherwise INVALID value is returned.
    static specie_type string_to_specie_type(std::string val, bool & set);

    // Specie type keys as list in same order as enum values.
    static std::vector< std::string > specie_type_list();

    // Write representation of this specie object in the input file format.
    //
    // Adds standard data and parameter list. Adds x,y,z coordinates
    // of cached particles. The position data must have been updated 
    // (using update_position) from the ensemble if the generated file 
    // is to have latest position information.
    void write_document(core::input_document & wr) const;

    // Update the cached position of a particular particle. The index 'lidx' refers
    // to the index of the particle within the species. This method automatically
    // updates the size of the array, such resizing can be avoided by first calling
    // update_position_size. Such resizing is only allowed for species that can  
    // have particles added/removed from the simulation.
    void update_position(std::size_t lidx, const geometry::coordinate & pos);

    // Update the number cached positions. This is only allowed for types that support
    // addition/deletion.
    void update_position_size(std::size_t count);

    // Add a particle to the specie from the input file when centroid information
    // is available. This implicitly sets specie type to MOBILE if it is currently
    // set to INVALID, otherwise the specie type must support localisation (e.g.
    // MOBILE or FLEXIBLE).
    //
    // This method should only be called while the simulation is being 
    // instantiated (specifically before simulator::generate_simulation is
    // called). Calling this method after the simulation has begun is
    // undefined.
    //
    // \require is_localized or sub_type == INVALID
    void append_position(const geometry::coordinate & pos, const geometry::centroid & cntr);

    // Add a particle to the specie from the input file when no centroid information
    // is available. This does not implicitly set the specie type.
    //
    // This method should only be called while the simulation is being 
    // instantiated (specifically before simulator::generate_simulation is
    // called). Calling this method after the simulation has begun is
    // undefined.
    //
    // \require not is_localized
    void append_position(const geometry::coordinate & pos);

    // Get the cached position of a particular particle. The index 'lidx' refers
    // to the index of the particle within the species, and is in the range 0
    // to get_position_size().
    //
    // \pre lidx < get_position_size()
    //
    // This data is the position data read from the input file. 
    const geometry::coordinate& get_position(std::size_t lidx) const;

    // Get the number of cached position.object.
    std::size_t get_position_size() const
    {
      return this->locations_.size();
    }

    // Get the localization data of a particular particle. The index 'lidx' refers
    // to the index of the particle within the species, and therefore fills the range
    // 0 to get_position_size() for localized species.
    //
    // \pre is_localized and ldx < get_postion_size()
    const geometry::centroid& get_localization_data(std::size_t lidx) const;

    // Get the number of defined localization data.
    std::size_t get_localization_size() const
    {
      return this->localize_data_.size();
    }

};

} // namespace particle
#endif
