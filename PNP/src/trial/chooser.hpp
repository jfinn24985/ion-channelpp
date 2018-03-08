#ifndef IONCH_TRIAL_CHOOSER_HPP
#define IONCH_TRIAL_CHOOSER_HPP

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

#include "trial/base_chooser.hpp"
#include <boost/serialization/vector.hpp>
#include "core/input_parameter_memo.hpp"
#include <string>
#include <cstddef>
#include "particle/specie.hpp"
#include "geometry/geometry_manager.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "trial/choice.hpp"
#include "utility/unique_ptr.hpp"

namespace trial {

// Chooser object for a single type of trial.
template<class Choice>
class chooser : public base_chooser
 {
   protected:
    chooser() = default;


   public:
    chooser(std::vector< core::input_parameter_memo > const& params, std::string type, const core::input_parameter_memo & specie_list, double rate)
    : base_chooser( params, type, specie_list, rate )
    {}

    virtual ~chooser();


   private:
    chooser(const chooser<Choice> & source) = delete;
    chooser(chooser<Choice> && source) = delete;

   public:
    chooser<Choice> & operator=(chooser<Choice> source) = delete;


  friend class boost::serialization::access;    //Read/write object to an archive.
    template< class Archive > void serialize(Archive & ar, std::size_t a_ver)
    {
      ar & boost::serialization::base_object< trial::base_chooser >(*this);
    }


   private:
    // Can this specie be used to create a trial of the current type?
    virtual bool is_permitted(const particle::specie & spec) const override
    {
      return Choice::permitted( spec );
    }

    // Generate a choice object.
    virtual void add_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params, boost::ptr_vector< base_choice >& choice_list) const override
    {
      choice_list.push_back( Choice::make_choice( ispec, gman, params ).release() );
    }


   public:
    //  Function to be used to instantiate chooser objects from input file.
    static std::unique_ptr< base_chooser > make_chooser(const std::vector< core::input_parameter_memo >& params, std::string type, const core::input_parameter_memo & specie_list, double rate)
    {
      return std::unique_ptr< base_chooser >( new chooser< Choice >( params, type, specie_list, rate ) );
    }


};
template<class Choice>
chooser<Choice>::~chooser() = default;


} // namespace trial
#endif
