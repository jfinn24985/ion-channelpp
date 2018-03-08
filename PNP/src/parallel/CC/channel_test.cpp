

#ifndef DEBUG
#define DEBUG 0
#endif

#include "parallel/CC/channel_test.hpp"

channel_test_suite::channel_test_suite()
    : canon_input (
      "channel\n"
      "nstep 10000\n"
      "naver 1000\n"
      "nbulk 1000\n"
      "name \"Help is on the way\"\n"
      "usepot .true.\n"
      "end\n\n")

{

}

// Test the setting up of the channel section help output message
void channel_test_suite::test_channel_help()

{
  ionch::channel::help_information(ionch::input_help::exemplar());
  TS_ASSERT (ionch::input_help::exemplar().have_section(ionch::constants::fschnl()));

}

// test 'filver' + 'fvermx'

void channel_test_suite::test_file_version()

{
  TS_ASSERT_EQUALS (ionch::channel::input_file_version, 1);

}

void channel_test_suite::test_ctor()

{
  ionch::channel ch;
  TS_ASSERT (ch.is_valid ());
  TS_ASSERT_EQUALS (1000ul, ch.steps_simulation ()); // default 1000
  TS_ASSERT_EQUALS (100ul, ch.steps_thermalisation ()); // default 100
  TS_ASSERT_EQUALS (100ul, ch.steps_bulk_simulation ()); // default 100
  TS_ASSERT (ch.title().empty ()); // no default
  TS_ASSERT (ch.use_input_chemical_potentials ()); // default true
  {
    std::stringstream ss;
    ch.execution_environment (ss);
    const std::string sout (ss.str());
    TS_ASSERT_DIFFERS(sout.find("MACHINE INTEGER SIZE"), std::string::npos);
    TS_ASSERT_DIFFERS(sout.find("OPERATING SYSTEM"), std::string::npos);
    TS_ASSERT_DIFFERS(sout.find("----------------------------------------------------------------------"), std::string::npos);
  }

}

// test 'set_run_number()' + 'firun' + 'fuuid'
//
// Part 1: firun and fuuid are unset before set_run_number
//
// Part 2: firun and fuuid are set after set_run_number
//       firun = %03d and fuuid has 16 chars
void channel_test_suite::test_set_run_number()

{
  ionch::channel ch;
  ch.set_run_number(1);
  const std::string uuid1 (ch.fuuid());
  TS_ASSERT_EQUALS (ch.firun(), "001");
  TS_ASSERT_EQUALS (ch.fuuid().size(), 32);

}

void channel_test_suite::test_input_file_routines() 
{
  ionch::channel var1;
  {
    boost::shared_ptr< std::istream > is (new std::stringstream (canon_input));
    ionch::input_reader irdr ("dummy", is);
    while (irdr.next ())
    {
      if (irdr.name() == ionch::constants::fschnl())
      {
        TS_ASSERT_THROWS_NOTHING(var1.read_input (irdr));
      }
    }
  }
  TS_ASSERT_EQUALS (var1.steps_simulation(), 10000ul);
  TS_ASSERT_EQUALS (var1.steps_thermalisation(), 1000ul);
  TS_ASSERT_EQUALS (var1.steps_bulk_simulation(), 1000ul);
  TS_ASSERT_EQUALS (var1.use_input_chemical_potentials(), true);
  TS_ASSERT_EQUALS (var1.title(), "Help is on the way");
  {
    std::stringstream os;
    var1.echo_input (os);
    std::string actual_input(os.str());
    std::pair < std::string::const_iterator, std::string::const_iterator > result = std::mismatch(canon_input.begin(), canon_input.end(), actual_input.begin());
    TS_ASSERT_EQUALS (std::size_t(result.first - canon_input.begin()), canon_input.size());
    TS_ASSERT_EQUALS (canon_input, actual_input);
  }

}

void channel_test_suite::test_serialize()

{
  ionch::channel ch;
  ch.set_run_number(1);
  const std::string afuuid (ch.fuuid());
  TS_ASSERT_EQUALS (afuuid.size(), 32);
  const std::string afirun(ch.firun());
  TS_ASSERT_EQUALS (afirun, "001");
  TS_ASSERT_EQUALS (1000ul, ch.steps_simulation ()); // default 1000
  TS_ASSERT_EQUALS (100ul, ch.steps_thermalisation ()); // default 100
  TS_ASSERT_EQUALS (100ul, ch.steps_bulk_simulation ()); // default 100
  TS_ASSERT (ch.title().empty ()); // no default
  TS_ASSERT (ch.use_input_chemical_potentials ()); // default true

  std::stringstream store;
  {
    boost::archive::text_oarchive oa(store);
    // write class instance to archive
    oa << ch;
  }
  {
    ionch::channel ch1;
    boost::archive::text_iarchive ia(store);
    // write class instance to archive
    ia >> ch1;
    TS_ASSERT_EQUALS (ch1.firun(), afirun);
    TS_ASSERT_EQUALS (ch1.fuuid(), afuuid);
    TS_ASSERT_EQUALS (1000ul, ch.steps_simulation ()); // default 1000
    TS_ASSERT_EQUALS (100ul, ch.steps_thermalisation ()); // default 100
    TS_ASSERT_EQUALS (100ul, ch.steps_bulk_simulation ()); // default 100
    TS_ASSERT (ch.title().empty ()); // no default
    TS_ASSERT (ch.use_input_chemical_potentials ()); // default true
  }

}

