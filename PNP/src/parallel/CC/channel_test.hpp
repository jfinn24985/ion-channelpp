#ifndef IONCH__CHANNEL_TEST_HPP
#define IONCH__CHANNEL_TEST_HPP


#include <string>

typedef CxxTest::TestSuite typedef1;
class channel_test_suite : public typedef1
 {
   private:
    const std::string canon_input;


   public:
    channel_test_suite();

    // Test the setting up of the channel section help output message
    static void test_channel_help();

    // test 'filver'
    static void test_file_version();

    static inline void test_execution_environment() {
        std::stringstream ss;
        TS_ASSERT_THROWS_NOTHING(ionch::channel::execution_environment (ss));
      };

    static void test_ctor();

    // test 'set_run_number()' + 'firun' + 'fuuid'
    //
    // Part 1: firun and fuuid are unset before set_run_number
    //
    // Part 2: firun and fuuid are set after set_run_number
    //       firun = %03d and fuuid has 16 chars
    static void test_set_run_number();

    // Part 3: second call to set_run_number raises an exception
    static inline void test_set_run_number_twice() {
        ionch::channel ch;
        TS_ASSERT_THROWS (ch.firun(), std::runtime_error&);
        TS_ASSERT_THROWS (ch.fuuid(), std::runtime_error&);
        TS_ASSERT_THROWS_NOTHING(ch.set_run_number(1));
        TS_ASSERT_THROWS(ch.set_run_number(2), std::runtime_error&);
      };

    // Call to filcur raises an exception before reading an input file.
    static inline void test_filcur() {
        ionch::channel ch;
        TS_ASSERT_THROWS (ch.filcur(), std::runtime_error&);
      };

    void test_input_file_routines();

    static void test_serialize();


};
#endif
