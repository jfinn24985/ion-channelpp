#ifndef IONCH_PERIODIC_CUBE_ALTERNATER_SAMPLER_HPP
#define IONCH_PERIODIC_CUBE_ALTERNATER_SAMPLER_HPP


#include "observable/base_observable.hpp"
#include "utility/estimater.hpp"
#include <cstddef>
#include "particle/specie_key.hpp"
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <string>

namespace core { class input_document; } 
namespace platform { class simulator; } 
namespace observable { class report_manager; } 

namespace periodic_cube {

//  Companion class to alternater_choice to broadcast the
//  add/remove probabilities.
class alternater_sampler : public observable::base_observable
 {
   public:
    struct probability_datum 
    {
        // Add probability
        utility::estimater add;

        // Remove probability
        utility::estimater remove;

        // Specie this data is for.
        std::size_t specie_key;

        probability_datum()
        : add(), remove(), specie_key() {}
        ~probability_datum() {}

        probability_datum(probability_datum && source)
        : add(std::move(source.add))
        , remove(std::move(source.remove))
        , specie_key(std::move(source.specie_key))
        {}

        probability_datum(const probability_datum & source)
        : add(source.add)
        , remove(source.remove)
        , specie_key(source.specie_key)
        {}

        probability_datum & operator=(probability_datum source)
        {
          this->swap(source);
          return *this;
        }

        void swap(probability_datum & source)
        {
          std::swap(this->add, source.add);
          std::swap(this->remove, source.remove);
          std::swap(this->specie_key, source.specie_key);
        }

    };
    

   private:
    //Set of add/remove probabilites
    std::vector< probability_datum > probabilities_;


   public:
    alternater_sampler() = default;

    virtual ~alternater_sampler();

    //Log message descibing the observable and its parameters
    void description(std::ostream & out) const override;


   private:
    // This sampler is not intended to be generated from an input file, so this 
    // method must delete entry from output doc.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    //The human readable label of this variable.
    //
    //General format will be "{type-label}[:obj-label]"
    //
    //For derived classes that will only have a single 
    //sampler instantiated, this label will be the same
    //as the sampler type-label in the input document.
    virtual std::string get_label() const override
    {
      return alternater_sampler::type_label_();
    }
    

    //Retrieve the current value of the variable.
    //
    ///return type vector< probability_datum >
    virtual boost::any get_value() const override;

    // Does nothing as this object does not store the observations
    // it samples.
    void prepare(platform::simulator & sim) override;

    //Broadcast add trial success probabilities. Does not output
    //data anywhere.
    void on_sample(const platform::simulator & sim) override;

    //Does nothing.
    void on_report(const platform::simulator & sim, std::ostream & out, observable::report_manager & reporter) override;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialization(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< sampled_observable >(*this);
        // Ignore choices_ and signal objects
      };


   public:
    static std::string type_label_();


};

} // namespace periodic_cube
#endif
