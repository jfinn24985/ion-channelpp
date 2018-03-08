#ifndef IONCH_PLATFORM_SIMULATOR_META_HPP
#define IONCH_PLATFORM_SIMULATOR_META_HPP


#include "core/input_definition.hpp"
#include <boost/function.hpp>

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <string>
#include "core/input_base_meta.hpp"
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "utility/bitset.hpp"
#include <cstddef>
#include "utility/unique_ptr.hpp"

namespace platform { class simulation_manager; } 
namespace core { class input_parameter_memo; } 
namespace platform { class simulation; } 
namespace core { class input_base_reader; } 
namespace core { class input_help; } 

namespace platform {

// Provide information useful for interpreting the input file. This includes
// a simulation_manager generator function, a type name and any optional
// parameters (with help descriptions).
class simulation_definition : private core::input_definition
{
   public:
    typedef boost::function1<boost::shared_ptr< simulation_manager >, const std::vector< core::input_parameter_memo > &> sim_manager_generator_fn;


   private:
    sim_manager_generator_fn factory_;


   public:
    // Main Ctor
    //
    // \param label : name of the region subtype.
    simulation_definition(std::string label, std::string desc, sim_manager_generator_fn fn)
    : input_definition( label, desc )
    , factory_( fn )
    {}


   private:
    simulation_definition() = delete;


   public:
    virtual ~simulation_definition() {}


   private:
    simulation_definition(simulation_definition && source) = delete;

    simulation_definition(const simulation_definition & source) = delete;

    simulation_definition & operator=(const simulation_definition & source) = delete;


   public:
    // Generate a base_region from the given label 
    // and parameters.
    boost::shared_ptr< simulation_manager > operator()(const std::vector< core::input_parameter_memo > & params) const
    {
      return this->factory_( params );
    }

using input_definition::add_definition;
using input_definition::empty;
using input_definition::has_definition;
using input_definition::label;
using input_definition::description;
using input_definition::publish_help;
using input_definition::size;

};
//Meta class between the input file and a simulation class
//object
//
//multiple = false
//
//Implementation Notes: The simulation objects are held by
//reference.
//
//Base parameters:
//
// * ntarg : target particle number : required : >0
//
// * epsw : media relative permittivity (has default) >0.0
// 
// * kelvin : temperature in Kelvin (has default) >0.0
//
// * inner : inner loop size (has default) >0
//
// * isave : checkpoint/report frequency (has default?) >0
//
// * name : Title for the simulation (optional)
//
// * type : simulation subtype
//
//Base simulation subtype parameters
//
// * nstep : number of steps in production phase
//
// * naver : number of steps in thermalization phase.

class simulator_meta : public core::input_base_meta
 {
   private:
    // The different simulation subtypes that can be performed.
    boost::ptr_vector< simulation_definition > types_;

    simulation * sim_;

    //  Which input entrys have been seen
    std::bitset<5> required_tags_;

    std::vector<core::input_parameter_memo> params_;

    //The type label of the current observable
    std::string type_;

    //The number of step cycles to perform in the equilibration phase of the simulation.
    std::size_t naver_;

    //The number of step cycles to perform in the production phase of the simulation.
    std::size_t nstep_;

    // indices in missing option bitset
    enum
     {
      SIMULATOR_PROD = 0,// entry 'nstep'
      SIMULATOR_THERM = 1,// entry 'naver'
      SIMULATOR_CHECK = 2,// entry 'isave'
      SIMULATOR_INNER = 3,// entry 'inner'
      SIMULATOR_TYPE// Check if "type" parameter was present

    };

    // Read an entry in the input file
    //
    // throw an error if input file is incorrect (using UTILITY_INPUT macro)
    bool do_read_entry(core::input_base_reader & reader) override;

    // Check for completeness of input at end of section. It does not
    // reset the meta data as 'multiple' is false.
    void do_read_end(const core::input_base_reader & reader) override;


   public:
    // Get a list of definition labels/keys
    std::vector< std::string > definition_key_list();

    // Add ctor for input "simulation" section
    //
    // \pre not has_type( ctor.type_name_ )
    // \post has_type( ctor.type_name_ )
    void add_type(std::unique_ptr< simulation_definition >& ctor);

    // Is there a ctor to match input "simulation" section with
    // "type = [label]"
    bool has_type(std::string label);

    // The number of storage type definitions
    std::size_t size() const { return this->types_.size(); }
    

    // Ary there no storage type definitions?
    bool empty() const { return this->types_.empty(); }
    

    simulator_meta(simulation & sim);

    virtual ~simulator_meta();


   private:
    // Reset the object
    void do_reset() override;


   public:
    void publish_help(core::input_help & helper) const override;


};

} // namespace platform

#endif
