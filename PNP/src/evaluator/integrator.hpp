#ifndef IONCH_EVALUATOR_INTEGRATOR_HPP
#define IONCH_EVALUATOR_INTEGRATOR_HPP

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

#include <cstddef>

namespace evaluator {

// ICC surface integrator interface
class integrator
 {
   private:
    // The index of the patch for this integrator
    std::size_t index_;

    // The number of subdivisions to use for integrating other patches.
    std::size_t nsub_other_;

    // The number of subdivisions to use for integrating self patch.
    std::size_t nsub_self_;


   protected:
    integrator();


   public:
    // The single nsub constructor (sets nsub_other = a_nsub and nsub_self = 5 * nsub_other)
    integrator(std::size_t a_nsub, std::size_t a_index)
    : index_(a_index)
    , nsub_other_(a_nsub)
    , nsub_self_(a_nsub*6.0)
    {}

    // The only constructor
    integrator(std::size_t a_nsubO, std::size_t a_nsubS, std::size_t a_index);
    

    virtual ~integrator() {};

    // Virtual method for performing the integration.
    virtual double operator ()(std::size_t other, double pxi, double pyi, double pzi, double pux, double puy, double puz) const = 0;


   private:
    integrator(const integrator & source) = delete;

    integrator(integrator && source) = delete;

    integrator & operator=(const integrator & source) = delete;



  friend class boost::serialization::access;    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
       ar & index_;
       ar & nsub_other_;
       ar & nsub_self_;
    }


   public:
    std::size_t index() const
    {
      return this->index_;
    }

    // Number of sub-tiles in each dimension when integrating between
    // two separate tiles
    std::size_t nsub_other() const
    {
       return this->nsub_other_;
    }

    // Number of sub-tiles in each dimension when integrating a tile
    // with itself
    std::size_t nsub_self() const
    {
       return this->nsub_self_;
    }


};
// ICC surface integrator for surface arcs
class arc_integrator : public integrator
 {
   private:
    // Z coord of arc centre
    double za0_;

    // Radial from z-axis of arc centre
    double ra0_;

    // Radius of arc
    double radius_;

    // Tile edges
    double theta1_;

    double theta2_;

    double phi1_;

    double phi2_;


   public:
    //  Should only be used for serialization
    explicit arc_integrator();

    // The only constructor
    arc_integrator(std::size_t a_nsub, std::size_t a_index, double a_zcentre, double a_rcentre, double a_rad, double ta1, double ta2, double fi1, double fi2)
    : integrator(a_nsub, a_index)
          , za0_ (a_zcentre)
          , ra0_ (a_rcentre)
          , radius_(a_rad)
          , theta1_(ta1)
          , theta2_(ta2)
          , phi1_(fi1)
          , phi2_(fi2)
        {};

    // The only constructor
    arc_integrator(std::size_t a_nsubO, std::size_t a_nsubS, std::size_t a_index, double a_zcentre, double a_rcentre, double a_rad, double ta1, double ta2, double fi1, double fi2)
    : integrator( a_nsubO, a_nsubS, a_index)
          , za0_ (a_zcentre)
          , ra0_ (a_rcentre)
          , radius_(a_rad)
          , theta1_(ta1)
          , theta2_(ta2)
          , phi1_(fi1)
          , phi2_(fi2)
        {}
    

    virtual ~arc_integrator() {};



  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
       ar & boost::serialization::base_object< integrator >(*this);
       ar & za0_;
       ar & ra0_;
       ar & radius_;
       ar & theta1_;
       ar & theta2_;
       ar & phi1_;
       ar & phi2_;
    }


   public:
    // Integrate over arc tiles.
    double operator ()(std::size_t pchx, double pxi, double pyi, double pzi, double pux, double puy, double puz) const;

    // Centre of arc circle
    double za0() const
    {
       return this->za0_;
    }

    // Radial distance from axis of rotation of the centre of
    // the arc
    double ra0() const
    {
       return this->ra0_;
    }

    // Radius of the arc
    double radius() const
    {
       return this->radius_;
    }

    // Start angle in axial direction
    double theta1() const
    {
       return this->theta1_;
    }

    // Start angle in rotation direction
    double phi1() const
    {
       return this->phi1_;
    }

    // End angle in axial direction
    double theta2() const
    {
       return this->theta2_;
    }

    // End angle in rotation direction
    double phi2() const
    {
       return this->phi2_;
    }


};
// ICC surface integrator cylindrical surfaces
class cylinder_integrator : public integrator
 {
   private:
    // Radius of cylinder
    double radius_;

    // Tile edges
    double zmin_;

    double zmax_;

    double phimin_;

    double phimax_;


   public:
    // Should only be used for serialization and testing
    explicit cylinder_integrator();

    // The only constructor
    cylinder_integrator(std::size_t a_nsub, int a_index, double a_rad, double z1, double z2, double fi1, double fi2)
    : integrator( a_nsub, a_index)
    , radius_(a_rad)
    , zmin_(z1)
    , zmax_(z2)
    , phimin_(fi1)
    , phimax_(fi2)
    {}

    // The only constructor
    cylinder_integrator(std::size_t a_nsubO, std::size_t a_nsubS, int a_index, double a_rad, double z1, double z2, double fi1, double fi2)
    : integrator( a_nsubO, a_nsubS, a_index)
    , radius_(a_rad)
    , zmin_(z1)
    , zmax_(z2)
    , phimin_(fi1)
    , phimax_(fi2)
    {}
    

    virtual ~cylinder_integrator() {};



  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
       ar & boost::serialization::base_object< integrator >(*this);
       ar & radius_;
       ar & zmin_;
       ar & zmax_;
       ar & phimin_;
       ar & phimax_;
    }


   public:
    // Integrate over cylinder tiles.
    double operator ()(std::size_t pchx, double pxi, double pyi, double pzi, double pux, double puy, double puz) const;

    // Radial distance from axis of rotation
    double radius() const
    {
       return this->radius_;
    }

    // Lower edge in rotation axis direction
    double zmin() const
    {
       return this->zmin_;
    }

    // Other edge in rotation axis direction
    double zmax() const
    {
       return this->zmax_;
    }

    // Lower edge in direction around of rotation axis
    double phimin() const
    {
       return this->phimin_;
    }

    // Other edge in direction around rotation axis
    double phimax() const
    {
       return this->phimax_;
    }


};
// ICC surface integrator radial surfaces
class wall_integrator : public integrator
 {
   private:
    //Z coordinate of wall
    double z_;

    // Inner radius of disk
    double ra0_;

    // Outer radius of disk
    double ra1_;

    // Tile centre radius
    double radius_;

    // Tile edges
    double phi1_;

    double phi2_;


   public:
    explicit wall_integrator();

    // The only constructor
    wall_integrator(std::size_t a_nsub, std::size_t a_index, double a_z, double a_ra0, double a_ra1, double a_rad, double fi1, double fi2)
    : integrator(a_nsub, a_index)
          , z_ (a_z)
          , ra0_ (a_ra0)
          , ra1_ (a_ra1)
          , radius_(a_rad)
          , phi1_(fi1)
          , phi2_(fi2)
        {}
    

    // The only constructor
    wall_integrator(std::size_t a_nsubO, std::size_t a_nsubS, std::size_t a_index, double a_z, double a_ra0, double a_ra1, double a_rad, double fi1, double fi2)
    : integrator(a_nsubO, a_nsubS, a_index)
          , z_ (a_z)
          , ra0_ (a_ra0)
          , ra1_ (a_ra1)
          , radius_(a_rad)
          , phi1_(fi1)
          , phi2_(fi2)
        {}
    

    virtual ~wall_integrator() {};



  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
       ar & boost::serialization::base_object< integrator >(*this);
       ar & z_;
       ar & ra0_;
       ar & ra1_;
       ar & radius_;
       ar & phi1_;
       ar & phi2_;
    }


   public:
    // Virtual method for performing the integration.
    double operator ()(std::size_t pchx, double pxi, double pyi, double pzi, double pux, double puy, double puz) const;

    // Lower radial distance from axis of rotation
    double z() const
    {
       return this->z_;
    }

    // Lower radial distance from axis of rotation
    double ra0() const
    {
       return this->ra0_;
    }

    // Upper radial distance from axis of rotation
    double ra1() const
    {
       return this->ra1_;
    }

    // Mean radial distance from axis of rotation
    double radius() const
    {
       return this->radius_;
    }

    // Start angle in rotation direction
    double phi1() const
    {
       return this->phi1_;
    }

    // End angle in rotation direction
    double phi2() const
    {
       return this->phi2_;
    }


};

} // namespace evaluator

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( evaluator::arc_integrator );
BOOST_CLASS_EXPORT_KEY( evaluator::cylinder_integrator );
BOOST_CLASS_EXPORT_KEY( evaluator::wall_integrator );
#endif
