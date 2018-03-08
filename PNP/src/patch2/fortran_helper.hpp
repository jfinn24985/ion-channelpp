#ifndef IONCH_PATCH_FORTRAN_HELPER_HPP
#define IONCH_PATCH_FORTRAN_HELPER_HPP


#include <string>

namespace patch {

// License GPL 1
class write_on_exit
 {
   private:
    std::ostringstream & what_to_write;

    const int where_to_write;

    const std::string msg;


   public:
    inline write_on_exit(std::ostringstream & what, int file_id) :
        what_to_write(what)
        , where_to_write(file_id)
        ,msg("Failed write")
      {};

    inline write_on_exit(std::ostringstream & what, int file_id, std::string error_msg) :
        what_to_write(what)
        ,where_to_write(file_id)
        ,msg(error_msg)
      {};

    inline ~write_on_exit() {
        const std::string val(what_to_write.str());
        if (val.size() != 0)
          {
    	UTILITY_STDC_ERROR(-1 != ::write(where_to_write, val.data(), val.size()), msg);
          }
      };


   private:
    // Unimplemented
    write_on_exit(const write_on_exit & );

    write_on_exit & operator =(const write_on_exit & );


};

} // namespace patch
#endif
