#ifndef IONCH_PARTICLE_CHANGE_HASH_HPP
#define IONCH_PARTICLE_CHANGE_HASH_HPP

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

#include <cstddef>
#include "utility/archive.hpp"

#include <iostream>

namespace particle {

// Hash for use with choice objects.
//
// Code should not care about the sort order of the hash. The
// purpose of the hash is to allow hash_tables, sets or maps
// and search methods to be used.

class change_hash
 {
   private:
    // Empty first byte or specie key
    std::size_t key_;

    // The number of particles in change set at start of trial.
    std::size_t start_;

    // The number of particles in the change set at end of trial.
    std::size_t finish_;

    // Trial subtype indicator. Value is user defined.
    std::size_t subtype_;


   public:
    change_hash(): key_(0), start_(0), finish_(0), subtype_(0) {}

    ~change_hash() {}

    change_hash(change_hash && source)
    : key_( std::move( source.key_ ) )
    , start_( std::move( source.start_ ) )
    , finish_( std::move( source.finish_ ) )
    , subtype_( std::move( source.subtype_ ) )
    {}

    change_hash(const change_hash & source)
    : key_( source.key_ )
    , start_( source.start_ )
    , finish_( source.finish_ )
    , subtype_( source.subtype_ )
    {}

    change_hash & operator=(change_hash source)
    {
      this->swap( source );
      return *this;
    }

    void swap(change_hash & source)
    {
      std::swap( this->key_, source.key_ );
      std::swap( this->start_, source.start_ );
      std::swap( this->finish_, source.finish_ );
      std::swap( this->subtype_, source.subtype_ );
    }

    change_hash(std::size_t speciekey, std::size_t start_count, std::size_t finish_count, std::size_t choice_subtype)
    : key_( speciekey )
    , start_( start_count )
    , finish_( finish_count )
    , subtype_( choice_subtype )
    {}



  friend class boost::serialization::access;


   private:
    //We serialize the object using the struct members not the
    //hash. This should ensure the object has the same logical 
    //state regardless of endianness.
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & key_;
        ar & start_;
        ar & finish_;
        ar & subtype_;
      }


   public:
    bool equivalent(const change_hash & other) const
    {
      return this->key_ == other.key_
    	&& this->start_ == other.start_
    	&& this->finish_ == other.finish_
    	&& this->subtype_ == other.subtype_;
    }

    // Get the hash's finish count
    std::size_t finish() const
    {
      return this->finish_;
    }

    // Set the hash's finish count
    void finish(std::size_t val)
    {
      this->finish_ = val;
    }

    // Get the hash's specie key.
    std::size_t key() const
    {
      return this->key_;
    }

    // Set the hash's specie key.
    void key(std::size_t val)
    {
      this->key_ = val;
    }

    bool less_than(const change_hash & other) const
    {
      return this->key_ < other.key_
    	|| (this->key_ == other.key_ && (this->start_ < other.start_
    		|| (this->start_ == other.start_ && (this->finish_ < other.finish_
    			|| (this->finish_ == other.finish_ && this->subtype_ < other.subtype_)))));
    }

    // Two change_hash objects match if they are equivalent except for the specie key,
    // which may or may not be equivalent.
    bool match(const change_hash & other) const
    {
      return this->start_ == other.start_
    	&& this->finish_ == other.finish_
    	&& this->subtype_ == other.subtype_;
    }

friend inline bool operator==(change_hash const& lhs, change_hash const&rhs)
{
  return lhs.equivalent(rhs);
}
friend inline bool operator!=(change_hash const& lhs, change_hash const&rhs)
{
  return not lhs.equivalent(rhs);
}
friend inline bool operator<(change_hash const& lhs, change_hash const&rhs)
{
  return lhs.less_than(rhs);
}
friend inline bool operator>=(change_hash const& lhs, change_hash const&rhs)
{
  return not lhs.less_than(rhs);
}
friend inline bool operator>(change_hash const& lhs, change_hash const&rhs)
{
  return rhs.less_than(lhs);
}
friend inline bool operator<=(change_hash const& lhs, change_hash const&rhs)
{
  return not rhs.less_than(lhs);
}
    friend std::ostream& operator<<(std::ostream& os, const change_hash & rhs)
    {
      os << "(" << rhs.key_ << ", " << rhs.start_ << ", " << rhs.finish_ << ", " << rhs.subtype_ << ")";
      return os;
    }

    // Get the hash's start count.
    std::size_t start() const
    {
      return this->start_;
    }

    // Set the hash's start count.
    void start(std::size_t val)
    {
      this->start_ = val;
    }

    // Get the hash's trial sub-type ID.
    std::size_t subtype() const
    {
      return this->subtype_;
    }

    // Set the hash's trial sub-type ID.
    void subtype(std::size_t val)
    {
      this->subtype_ = val;
    }


};

} // namespace particle

// As hash is a union we need to tell the serialization system that
// it is serializable.
//BOOST_CLASS_IMPLEMENTATION(particle::change_hash,boost::serialization::object_serializable)
#endif
