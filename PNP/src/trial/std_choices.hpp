#ifndef IONCH_TRIAL_STD_CHOICES_HPP
#define IONCH_TRIAL_STD_CHOICES_HPP

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

#include "trial/choice.hpp"
#include <cstddef>
#include "utility/unique_ptr.hpp"
#include <boost/serialization/vector.hpp>

namespace particle { class change_set; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace utility { class random_distribution; } 
namespace trial { class choice_meta; } 
namespace core { class input_parameter_memo; } 
namespace particle { class specie; } 

namespace trial {

//Objects of this class define an MC move that involves a
//displacement within a cube. The displacement is defined by a
//displacement of max_displacement by a random number in [0,1)
//for each coordinate.
//
//During the non-production phase of the MC simulation the 
//object adjusts the max displacement so that only about
//50% of trials will be successful.
//
//In the MC literature, the optimisation to 50% success rate is
//thought to maximise efficiency of sampling the phase space.
//(To many successes mean that the particles remain independent
//and to few successes mean that a successful trial is added
//many times.)
//
//change_hash subtype is "0", so hash pattern is ( X, 1, 1, 0 )
class move_choice : public base_choice
 {
   private:
    //Maximum displacement
    double delta_;

    //  Default delta value if none is given in the input.
    static const double default_delta;


   public:
    move_choice(std::size_t ispec);


   private:
    //Default ctor for serialization only
    move_choice()
    : base_choice()
    , delta_()
    {}


   public:
    virtual ~move_choice();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
        ar & delta_;
      };


   public:
    std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr);

    double max_displacement() const
    {
      return this->delta_;
    }

    // Change the maximum displacement step size.
    void set_max_displacement(double val)
    {
      this->delta_ = val;
    }

    // Add a definition object for this choice class to the
    // meta object.
    static void add_definition(choice_meta & meta);

    // The default displacement value.
    static double default_displacement()
    {
      return move_choice::default_delta;
    }

    // Get delta from params and then create a new object.
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature:
    //   static unique_ptr< base_choice > make_choice( size_t, geometry_manager const&, map< string, string > const& )
    // 
    
    static std::unique_ptr< move_choice > make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params);

    // All mobile specie types can have this choice?
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature
    //   static bool make_choice( specie const& )
    static bool permitted(const particle::specie & spec)
    {
      return true;
    }


};
// Objects of this class define an MC move that involves a
// jump from anywhere to anywhere within the simulation.
//
//change_hash subtype is "1", so hash pattern is ( X, 1, 1, 1 )
class jump_choice : public base_choice
 {
   private:
    // default ctor for deserialization only
    jump_choice()
    : base_choice()
    {}


   public:
    jump_choice(std::size_t ispec)
    : base_choice( particle::change_hash( ispec, 1, 1, 1 ) )
    {}

    virtual ~jump_choice();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
      }


   public:
    std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr);

    // Add a definition object for this choice class to the
    // meta object.
    static void add_definition(choice_meta & meta);

    // Get delta from params and then create a new object.
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature:
    //   static unique_ptr< base_choice > make_choice( size_t, geometry_manager, map< string, string > const& )
    // 
    
    static std::unique_ptr< jump_choice > make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params);

    // Only solute species can have this trial type?
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature
    //   static bool make_choice( specie const& )
    static bool permitted(const particle::specie & spec);


};
// Objects of this class define an MC move that involves a
// jump from anywhere to a specific region within the simulation.
//
//change_hash subtype is "2", so hash pattern is ( X, 1, 1, 2 )
class jump_in : public base_choice
 {
   private:
    // The region index. 
    std::size_t region_index_;

    // default ctor for deserialization only
    jump_in()
    : base_choice()
    {}


   public:
    jump_in(std::size_t ispec, std::size_t rgx)
    : base_choice( particle::change_hash( ispec, 1, 1, 2 ) )
    , region_index_( rgx )
    {}
    

    virtual ~jump_in();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
        ar & region_index_;
      }


   public:
    std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr);

    // The region name for this choice.
    std::size_t region_index() const
    {
      return this->region_index_;
    }

    // Add a definition object for this choice class to the
    // meta object.
    static void add_definition(choice_meta & meta);

    // Get delta from params and then create a new object.
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature:
    //   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
    // 
    
    static std::unique_ptr< jump_in > make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params);

    // Only solute species can have this trial type?
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature
    //   static bool make_choice( specie const& )
    static bool permitted(const particle::specie & spec);


};
// Objects of this class define an MC move that involves a
// jump from anywhere to a specific region within the simulation.
//
// change_hash subtype is "3", so hash pattern is ( X, 1, 1, 3 )
class jump_out : public base_choice
 {
   private:
    // The region index. 
    std::size_t region_index_;

    // default ctor for deserialization only
    jump_out()
    : base_choice()
    {}


   public:
    jump_out(std::size_t ispec, std::size_t rgx)
    : base_choice( particle::change_hash( ispec, 1, 1, 3 ) )
    , region_index_( rgx )
    {}

    virtual ~jump_out();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
        ar & region_index_;
      }


   public:
    std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr);

    // The region name for this choice.
    std::size_t region_index() const
    {
      return this->region_index_;
    }

    // Add a definition object for this choice class to the
    // meta object.
    static void add_definition(choice_meta & meta);

    // Get delta from params and then create a new object.
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature:
    //   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
    // 
    
    static std::unique_ptr< jump_out > make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params);

    // Only solute species can have this trial type?
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature
    //   static bool make_choice( specie const& )
    static bool permitted(const particle::specie & spec);


};
// Objects of this class define an MC move that involves a
// jump from anywhere to anywhere in a specific region. It
// is an exception if any particles of the given species are 
// not in the stated region. This trial is for channel-only
// species.
//
//change_hash subtype is "4", so hash pattern is ( X, 1, 1, 4 )
class jump_around : public base_choice
 {
   private:
    // The region index. 
    std::size_t region_index_;

    // default ctor for deserialization only
    jump_around()
    : base_choice()
    {}


   public:
    jump_around(std::size_t ispec, std::size_t rgx)
    : base_choice( particle::change_hash( ispec, 1, 1, 4 ) )
    , region_index_( rgx )
    {}

    virtual ~jump_around();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
        ar & region_index_;
      }


   public:
    std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr);

    // The region name for this choice.
    std::size_t region_index() const
    {
      return this->region_index_;
    }

    // Add a definition object for this choice class to the
    // meta object.
    static void add_definition(choice_meta & meta);

    // Get delta from params and then create a new object.
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature:
    //   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
    // 
    
    static std::unique_ptr< jump_around > make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params);

    // Only solute species can have this trial type?
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature
    //   static bool make_choice( specie const& )
    static bool permitted(const particle::specie & spec);


};
// Add particle move generator for a single specie
//
// (instantiated from input file via gc_choice objects)
//
// Choice hash pattern is { X, 1, 0, 0 }
class remove_specie : public base_choice
 {
   public:
    // Add/remove given specie
    //
    
    remove_specie(std::size_t ispec)
    : base_choice ( particle::change_hash( ispec, 1, 0, 0 ) )
    {}


   protected:
    // Default add/remove given specie (only for serialization)
    //
    
    remove_specie()
    : base_choice ()
    {}


   public:
    virtual ~remove_specie();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
      }


   public:
    // Create a particle remove trial
    virtual std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr);

    // Get delta from params and then create a new object.
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature:
    //   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
    // 
    
    static std::unique_ptr< remove_specie > make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params);

    // Only solute species can have this trial type?
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature
    //   static bool make_choice( specie const& )
    static bool permitted(const particle::specie & spec);


};
// Add particle move generator for a single specie
//
// (instantiated from input file via gc_choice objects)
//
// Choice hash pattern is { X, 0, 1, 0 }
class add_specie : public base_choice
 {
   public:
    // Add/remove given specie within a cube
    //
    // Choice hash püattern is { X, 0, 1, 0 }
    
    add_specie(std::size_t ispec)
    : base_choice ( particle::change_hash( ispec, 0, 1, 0 ) )
    {}


   protected:
    // Default add/remove given specie (only for serialization)
    //
    
    add_specie() = default;

   public:
    virtual ~add_specie();



  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::base_object< base_choice >(*this);
          };


   public:
    // Create a particle addition trial
    virtual std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr);

    // Add a definition object for this choice class to the
    // meta object.
    //
    // NOTE: For add/remove paired type add_definition is
    // only defined in add half of pair.
    static void add_definition(choice_meta & meta);

    // Get delta from params and then create a new object.
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature:
    //   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
    // 
    
    static std::unique_ptr< add_specie > make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params);

    // Only solute species can have this trial type?
    //
    // Needs to be defined as static method in derived classes that
    // use chooser<>.
    //
    // signature
    //   static bool make_choice( specie const& )
    static bool permitted(const particle::specie & spec);


};

} // namespace trial

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT_KEY(trial::move_choice);
BOOST_CLASS_EXPORT_KEY(trial::jump_choice);
BOOST_CLASS_EXPORT_KEY(trial::jump_in);
BOOST_CLASS_EXPORT_KEY(trial::jump_out);
BOOST_CLASS_EXPORT_KEY(trial::jump_around);
BOOST_CLASS_EXPORT_KEY(trial::add_specie);
BOOST_CLASS_EXPORT_KEY(trial::remove_specie);

#include "trial/chooser_pair.hpp"
#include "trial/chooser.hpp"
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::move_choice >);
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::jump_choice >);
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::jump_in >);
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::jump_out >);
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::jump_around >);
BOOST_CLASS_EXPORT_KEY2(trial::chooser_pair< trial::add_specie BOOST_PP_COMMA() trial::remove_specie >, "trial::chooser_pair< trial::add_specie, trial::remove_specie >");
#endif
