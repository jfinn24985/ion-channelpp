#ifndef IONCH_CORE_INPUT_DEFINITION_HPP
#define IONCH_CORE_INPUT_DEFINITION_HPP


#include <string>
#include <boost/serialization/map.hpp>
#include "core/help_entry.hpp"
#include <cstddef>

namespace core { class input_help; } 

namespace core {

// Information about a subtype and its special parameters.  Provides uniform
// interface to be used to instantiate a subtype from an input file.
//
// pattern: virtual noncopyable
class input_definition
 {
   private:
    // The label of this subtype in the input file.
    std::string label_;

    // A description of the subtype.
    std::string description_;

    // Map of allowed parameter names and help documentation.
    std::map< std::string, help_entry > definitions_;


   public:
    input_definition(std::string label, std::string desc)
    : label_( label )
    , description_( desc )
    , definitions_()
    {}

    virtual ~input_definition();


   private:
    input_definition(input_definition && source) = delete;

    input_definition(const input_definition & source) = delete;

    input_definition & operator=(const input_definition & source) = delete;


   public:
    // Add a help entry for this subtype
    //
    // \pre not had_definition( entry.title )
    void add_definition(help_entry entry);

    std::string description() const
    {
      return this->description_;
    }

    // Are there no parameter definitions?
    bool empty() const
    {
      return this->definitions_.empty();
    }

    bool has_definition(std::string name) const;

    std::string label() const
    {
      return this->label_;
    }

    // Put help information from definition into helper
    // object.
    void publish_help(input_help & helper, std::string seclabel) const;

    // HOw many parameter definitions are there?
    std::size_t size() const
    {
      return this->definitions_.size();
    }


};

} // namespace core
#endif
