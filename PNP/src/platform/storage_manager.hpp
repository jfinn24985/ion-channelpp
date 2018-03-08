#ifndef IONCH_PLATFORM_STORAGE_MANAGER_HPP
#define IONCH_PLATFORM_STORAGE_MANAGER_HPP

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

#include <string>
#include <cstddef>
#include <boost/serialization/shared_ptr.hpp>
#include <iostream>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>



namespace platform { class simulation; } 
namespace core { class input_document; } 
namespace core { class input_parameter_memo; } 
namespace observable { class base_sink; } 
namespace core { class input_base_reader; } 
namespace core { class input_delegater; } 

namespace platform {

// Manage access to data sources and sinks.  Manage the
// system's log stream.
//
// This handles interaction with the external system.
// This includes managing what happens when a simulation is
// being run in parallel (eg MPI, OpenMP etc).
//
// In this library each simulation instance runs in a
// single thread, but an application can have multiple
// threads running.

class storage_manager
 {
   private:
    // The name of the checkpoint file. Rarely should this
    // be altered from the default.
    
    std::string checkpoint_name_;

    // The input file name template
    //
    // Filename base can be a simple filename or a format string that
    // takes a number. The number substituted into the string will be
    // the run number. For example the default "input.%1$03d.inp" with
    // run number of 12 will give filename "input.012.inp".
    
    std::string filename_base_;

    //Boost Format template defining the directory for output files.
    //Directory name is generated using
    //
    //  boost::format(output_dir_fmt_) % run_index
    //
    //The default is a format that gives '006/dat' where '006' is
    //the three digit run number string and the UUID is not used.
    std::string output_dir_fmt_;

    // The name of the output file. If an extension is
    // given it should be used to select the file type and
    // use an appropriate base_sink subclass.
    std::string output_name_;

    //The index of this run. 
    //
    //Defaults to 1.
    std::size_t run_number_;

    // A number to initialise the random number generator.
    std::size_t seed_value_;

    // Not serialized
    boost::shared_ptr<std::ostream> log_;

    // Are we using the cache log or have cut over to output log?
    bool cutover_log_;


   public:
    storage_manager();

    virtual ~storage_manager();


   private:
    // no move
    storage_manager(storage_manager && source);

    // no copy
    storage_manager(const storage_manager & source);

    // no assignment
    storage_manager & operator=(const storage_manager & source);


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version);

   public:
    // Get the default name of the checkpoint file
    static std::string default_checkpoint_name();

    // Get the default template for the input filename.
    static std::string default_filename_base();

    // Get the default name of the output file
    static std::string default_output_name();

    static std::string default_output_dir_fmt();

    constexpr static std::size_t default_run_number()
    {
      return 1ul;
    }
    

    // The name of the checkpoint file.
    std::string checkpoint_name() const
    {
      return this->checkpoint_name_;
    }

    // Make a unique seed for a simulation based on the given seed value.
    //
    // For serial simulations this method should be a no-op. For 
    // parallel simulations it should produce a unique and
    // reproducable seed value for each simulation.
    virtual std::size_t compute_seed(std::size_t seed) const = 0;

    // Details about the current simulation to be written to the
    // log at the start of the simulation.
    void description(std::ostream & os) const;


   private:
    // Details about the current storage object.
    virtual void do_description(std::ostream & os) const = 0;


   public:
    // Set the template/regular expression for deriving input filenames.
    const std::string& filename_base() const
    {
       return this->filename_base_;
    }

    // A word used to idenitfy the storage manager type in the input file.
    virtual std::string get_label() const = 0;

    // Return the simulations log output stream.  If no
    // log stream is currently open then this calls open_log.
    // Therefore this may throw any exception as open_log.
    virtual std::ostream & get_log();

    // Have we switched from in-memory log to output log?
    bool is_cutover() const
    {
      return this->cutover_log_;
    }
    // The main function. This should catch any anticipated exceptions.
    virtual bool main(int argc, char ** argv, simulation & sim) = 0;

    // The output directory template.
    std::string output_dir_fmt() const
    {
      return this->output_dir_fmt_;
    }

    // The target file name of the output file.
    std::string output_name() const
    {
      return this->output_name_;
    }

    // Generate a random value to seed the random number generator.
    //
    // This uses an operating system provided random source to generate
    // a single random unsigned int suitable to seed the psuedo-random
    // number generator. 
    
    static uint32_t random_seed_value();

    //Get the index (default=1) of the current application run
    std::size_t run_number() const
    {
      return this->run_number_;
    }

    // Return a reference to the system error log.
    //
    // On UNIX systems this will be the standard error stream.
    std::ostream & system_log() const;

    // Write contents of simulation as an input document
    void write_document(core::input_document & wr);


   private:
    // Write extra data of derived simulator object into the input
    // document object at the given section.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const = 0;


   public:
    // Signal that we can switch from in-memory log to output log.
    void cutover_log();

    // Set the name of the checkpoint file.
    //
    // \require not fn.empty
    void set_checkpoint_name(std::string fn);

    // Set the template/regular expression for deriving input filenames.
    void set_filename_base(std::string fmt);

    // Function to set log to an externally defined output stream.
    //
    // \return The previous stream.
    boost::shared_ptr< std::ostream > set_log(boost::shared_ptr< std::ostream > os_ptr);

    //Set template string used to create directory path for 
    //output files.
    //
    //Formatting is based on Boost Format so the string can
    //be in Boost Format, Posix printf or printf style.
    void set_output_dir_fmt(std::string fmt);

    // Set the name of the output file. If an extension is
    // given it should be used to select the file type and
    // use an appropriate base_sink subclass.
    //
    // \require has_valid_extension( fmt )
    void set_output_name(std::string fmt);

    // Subtype specific parameters (from the input file).
    virtual void set_parameters(const std::vector< core::input_parameter_memo >& params) = 0;

    // Set the run number.
    //
    // NOTE: To use automatic filename and directory name
    // generation this must be called before input and output
    // files or directories are used.
    
    void set_run_number(std::size_t num)
    {
      this->run_number_ = num;
    }

    // Test the name of the output file to see if it has a 
    // known extension. (Having no extension is interpreted
    // as using the default extension and is therefore also
    // true)
    static bool has_valid_extension(std::string fmt);

    // Use file extension (or default extension) to open a sink object.
    //
    // \pre has_valid_extension( path.native )
    static boost::shared_ptr< observable::base_sink > open_sink_by_extension(const boost::filesystem::path & fpath, std::string def_ext);

    // Generate path to the checkpoint file.
    //
    // Default implementation :
    //
    // \return compute_output_dir + checkpoint_name
    //
    // \pre not checkpoint_name.empty
    
    virtual std::string checkpoint_path() const;

    // Computer the output directory for the current simulator object
    // from the output_dir_fmt template.
    virtual std::string compute_output_dir() const = 0;


   protected:
    // Ensure the output directory exists, creating if necessary.
    // Return the path to the directory. Throws error if output
    // directory path exists but is not a directory.
    
    boost::filesystem::path ensure_output_dir();


   public:
    // Generate a series of possible filenames and return
    // the first filename that is found. Throw an error if no
    // filenames are found. The template string is processed
    // using 'printf' or boost::format like function.
    //
    // For a filename_base of "input.%1$03d.inp" and run number
    // of 11 the filenames checked for will be:
    //
    //  * input.011.inp
    //
    //  * input.inp
    //
    // If no "%" is present it assumes the string is directly
    // the filename.
    //
    // \post exists(return val)
    
    std::string find_input_filename() const;

    // Generate path to the output file.
    //
    // Default implementation :
    //
    // \return compute_output_dir + output_name
    //
    // \pre not checkpoint_name.empty
    
    virtual std::string output_path() const;

    // Create/open the simulation's output sink.
    virtual boost::shared_ptr< observable::base_sink > open_output() = 0;

    // Create/open the simulation's input stream.
    virtual boost::shared_ptr< core::input_base_reader > open_input() = 0;

    // Create/open the simulation's checkpoint stream.
    virtual boost::shared_ptr< std::ostream > open_checkpoint() = 0;


   private:
    // Create the system's log output stream.
    virtual boost::shared_ptr< std::ostream > open_log() = 0;


   public:
    // Adds factory methods and input parsers for all the types that can be instantiated from 
    // the input file.
    static void build_input_delegater(boost::shared_ptr< storage_manager > fstype, core::input_delegater & delegate);


   protected:
    // Process the command line.
    //
    // return is one of:
    //
    // * bad options (0)
    //
    // * help (1)
    //
    // * input (2)
    //
    // * restart (3)
    // 
    
    int process_command_line(int argc, char ** argv);


   public:
    std::size_t get_seed_value() const
    {
      return this->seed_value_;
    }

    void set_seed_value(std::size_t seed)
    {
      this->seed_value_ = seed;
    }


};
template<class Archive> inline void storage_manager::serialize(Archive & ar, const unsigned int version) 
{
  ar & checkpoint_name_;
  ar & filename_base_;
  ar & output_dir_fmt_;
  ar & output_name_;
  ar & run_number_;
  ar & seed_value_;
  ar & cutover_log_;

}


} // namespace platform
#endif
