#ifndef IONCH_GEOMETRY_REGION_META_HPP
#define IONCH_GEOMETRY_REGION_META_HPP

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

#include "core/input_definition.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include <string>
#include <boost/serialization/vector.hpp>
#include "core/input_base_meta.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "utility/unique_ptr.hpp"
#include <cstddef>

namespace geometry { class base_region; } 
namespace core { class input_parameter_memo; } 
namespace geometry { class geometry_manager; } 
namespace core { class input_help; } 
namespace core { class input_base_reader; } 

namespace geometry {

// Information about a region class's input data.
class region_definition : private core::input_definition
 {
   public:
    // Function signature for creating new region.
    typedef boost::shared_ptr< base_region > (*generator_fn)(std::string label, std::vector< core::input_parameter_memo > const& params);


   private:
    // Creator of region objects.
    generator_fn factory_;


   public:
    // Main Ctor
    //
    // \param label : name of the region subtype.
    // \param desc : description of the region subtype.
    // \param fn : constructor for region subtype.
    region_definition(std::string label, std::string desc, generator_fn fn)
    : input_definition( label, desc )
    , factory_( fn )
    {}


   private:
    region_definition() = delete;


   public:
    virtual ~region_definition() {}


   private:
    region_definition(region_definition && source) = delete;

    region_definition(const region_definition & source) = delete;

    region_definition & operator=(const region_definition & source) = delete;


   public:
    // Generate a base_region from the given label 
    // and parameters.
    boost::shared_ptr< base_region > operator()(std::string label, const std::vector< core::input_parameter_memo > & params) const
    {
      return (*(this->factory_))( label, params );
    }

using input_definition::add_definition;
using input_definition::empty;
using input_definition::has_definition;
using input_definition::label;
using input_definition::publish_help;
using input_definition::size;

};
class region_meta : public core::input_base_meta
 {
   private:
    // Where we put newly defined regions
    boost::shared_ptr< geometry_manager > manager_;

    // Build type from object.
    boost::ptr_vector< region_definition > type_to_object_;

    // The region's label. This must be unique within a simulation.
    std::string label_;

    // The label identifying the region's class.
    std::string type_name_;

    // Set of parameters for this region type. This should be 
    // defined by the individual regions.
    std::vector< core::input_parameter_memo > parameter_set_;

    // Inaccessible default ctor.
    region_meta() = default;


   public:
    // Main ctor.
    region_meta(boost::shared_ptr< geometry_manager > man);

    ~region_meta();

    // Add a new region type definition.
    //
    // \pre not has_definition(defn.type_name_)
    void add_definition(std::unique_ptr< region_definition > & defn);

    // Get a list of definition labels/keys
    std::vector< std::string > definition_key_list();

    // Ary there no evaluator definitions?
    bool empty() const { return this->type_to_object_.empty(); }
    

    virtual void publish_help(core::input_help & helper) const override;

    // Do we have a region with this typename?
    bool has_definition(std::string type_name);


   private:
    //Read a name/value entry in the input file. Return true if the entry was processed.
    //
    //throw an error if input file is incorrect (using UTILITY_INPUT macro)
    virtual bool do_read_entry(core::input_base_reader & reader) override;

    // Perform checks at the end of reading a section and create/add result to
    // simulations. If multiple sections may be read, this method must also
    // reset the meta object state.
    virtual void do_read_end(const core::input_base_reader & reader) override;

    // Reset the object state on entry into read_seaction. This
    // provides a stronger exception safety if the delegate is
    // reused after raising an exception in read_section.
    virtual void do_reset() override;


   public:
    // The number of evaluator definitions
    std::size_t size() const { return this->type_to_object_.size(); }
    


};

} // namespace geometry
#endif
