#ifndef IONCH_PLATFORM_IMC_META_HPP
#define IONCH_PLATFORM_IMC_META_HPP


#include "core/input_base_meta.hpp"
#include <boost/function.hpp>

#include <boost/serialization/map.hpp>
#include <string>
#include "observable/sampler_meta.hpp"
#include <cstddef>
#include "utility/bitset.hpp"
#include <boost/serialization/vector.hpp>

namespace observable { class base_observable; } 
namespace core { class input_base_reader; } 
namespace platform { class simulation; } 

namespace platform {

class imc_meta : public core::input_base_meta
 {
   public:
    typedef boost::function2<void, std::map< std::string, std::string >&, platform::simulator&> sampler_generator_fn;

    // Indices used to check for missing options.
    enum
     {
      SAMPLER_TYPE = 0// Type of sampler option

    };

    static std::map< std::string, observable::sampler_definition::sampler_generator_fn > type_to_object_;


   private:
    static std::size_t counter_;

    std::map<std::string, std::string> parameter_set_;

    // Check for missing options.
    std::bitset<1> missing_required_tags_;

    //The type label of this observable
    std::string type_;


   public:
    imc_meta();

    virtual ~imc_meta();

    // Get a list of definition labels/keys
    std::vector< std::string > definition_key_list();


   protected:
    //Read an entry in the input file. Return true if the entry was processed.
    //
    //throw an error if input file is incorrect (using UTILITY_INPUT macro)
    virtual bool do_read_entry(core::input_base_reader & reader) override;

    // Perform checks at the end of reading a section before creating an IGCMC object
    // and adding it to the simulation. This does not reset the meta data as 'multiple'
    // is false.
    virtual void do_read_end(simulation & sim) override;


   private:
    // Reset the object
    void do_reset() override;


};

} // namespace platform
#endif
