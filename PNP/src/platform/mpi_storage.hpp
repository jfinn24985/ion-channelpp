#ifndef IONCH_PLATFORM_MPI_STORAGE_HPP
#define IONCH_PLATFORM_MPI_STORAGE_HPP


#include "platform/storage_manager.hpp"
#include <cstddef>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/mpi.hpp>

#include <string>
#include <iostream>
#include <boost/serialization/vector.hpp>

namespace core { class input_document; } 
namespace core { class input_parameter_memo; } 
namespace platform { class simulation; } 
namespace observable { class base_sink; } 
namespace core { class input_base_reader; } 
namespace platform { class storage_meta; } 

namespace platform {

// The first parallel storage version using MPI and
// simple (non-parallel) data access.

class mpi_storage : public storage_manager
 {
   private:
    // The MPI rank of this instance.  ** NOT SERIALIZED **
    std::size_t rank_;

    // The number of MPI jobs.
    std::size_t sibling_count_;

    //MPI environment
    boost::shared_ptr< boost::mpi::environment > env_;

    // The main communicator. ** NOT SERIALIZED **
    boost::shared_ptr< boost::mpi::communicator > world_;


   public:
    mpi_storage();


   private:
    mpi_storage(std::size_t rank);


   public:
    virtual ~mpi_storage();


   private:
    // no move
    mpi_storage(mpi_storage && source);

    // no copy
    mpi_storage(const mpi_storage & source);

    // no assignment
    mpi_storage & operator=(const mpi_storage & source);


  friend class boost::serialization::access;    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
      {
        ar & boost::serialization::base_object< platform::storage_manager >( *this );
      }

   public:
    // A word used to idenitfy the storage manager type in the input file.
    std::string get_label() const override
    {
      return mpi_storage::type_label_();
    }


   private:
    // Details about the current storage object.
    void do_description(std::ostream & os) const override;

    // Write extra data of derived simulator object into the input
    // document object at the given section.
    void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Throw an exception as this manager takes no extra
    // parameters.
    void set_parameters(const std::vector< core::input_parameter_memo >& params) override;

    // Abort the MPI system with the given error code.
    void abort(int err_num);

    // Wait for all other MPI jobs
    void barrier();

    // Make a unique seed for a simulation based on the given seed value.
    //
    // For serial simulations this method should be a no-op. For 
    // parallel simulations it should produce a unique and
    // reproducable seed value for each simulation.
    virtual std::size_t compute_seed(std::size_t seed) const
    {
      return seed + this->rank_;
    }

    // Computer the output directory for the current simulator object
    // from the output_dir_fmt template.
    std::string compute_output_dir() const override;

    virtual bool main(int argc, char ** argv, simulation & sim) override;

    // Create/open the simulation's output sink.
    //
    // If extension is not given then a sink of type gdbm_sink
    // is returned.
    boost::shared_ptr< observable::base_sink > open_output() override;

    // Create/open the simulation's input stream.
    //
    // This uses find_input_filename to identify the
    // input file then opens it using a input_reader
    // type reader object.
    //
    // \throw as per find_input_filename
    boost::shared_ptr< core::input_base_reader > open_input() override;

    // Create/open the simulation's checkpoint stream.
    boost::shared_ptr< std::ostream > open_checkpoint() override;


   private:
    // Create the system's log output stream. In this case
    // the log file is to standard out.
    boost::shared_ptr< std::ostream > open_log() override;


   public:
    std::size_t rank() const
    {
      return this->rank_;
    }

    std::size_t size() const
    {
      return this->sibling_count_;
    }

    static void add_definition(storage_meta & meta);

    // The string "local" used to identify this storage
    // manager in the input file.
    static std::string type_label_();


};

} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( platform::mpi_storage );
#endif
