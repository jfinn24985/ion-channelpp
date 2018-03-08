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


#ifndef DEBUG
#define DEBUG 0
#endif

#define BOOST_TEST_MODULE core_test
#include <boost/test/unit_test.hpp>

#include "core/core_test/core_test_suite.hpp"
#include "core/constants.hpp"
#include "core/fixed_width_output_filter.hpp"
#include "core/help_entry.hpp"
#include "core/help_subtype.hpp"
#include "core/help_section.hpp"
#include "core/input_definition.hpp"
#include "core/input_delegater.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
#include "core/input_node.hpp"
#include "core/input_parameter_memo.hpp"
#include "core/input_parameter_reference.hpp"
#include "core/input_preprocess.hpp"
#include "core/input_reader.hpp"
#include "core/input_document.hpp"
#include "core/strngs.hpp"
#include "core/core_test/test_meta.hpp"

// Manuals
#include "geometry/coordinate.hpp"
// -
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <type_traits>
// - 
//Test common functionality of input_base_reader derived classes
template< class Reader > void core_test_suite::test_read_input_buffer()
{
  const std::string dummy_input( "\n\n# A comment\nsection # Another comment\nname1 = value1\n1 2.3 4 not a num # 6.2\nname1 value1" );
  const std::string dummy_filename( "dummy" );
  const boost::filesystem::path dummy_path( boost::filesystem::absolute( "dummy" ) );
  const std::string dummy_path_str( dummy_path.string() );
  
  {
    Reader rdr;
    // TEST reader next part 1
    rdr.add_buffer( dummy_filename, dummy_input );
    BOOST_CHECK( rdr.next() );
    BOOST_CHECK_EQUAL( "section # Another comment", rdr.line() );
    BOOST_CHECK_EQUAL( "section", rdr.name() );
    BOOST_CHECK_EQUAL( "", rdr.value() );
    BOOST_CHECK_EQUAL( 4, rdr.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_path_str, rdr.current_filename() );
    BOOST_CHECK_NE( dummy_filename, rdr.current_filename() );
  
    // TEST reader next part 2
    BOOST_CHECK( rdr.next() );
    BOOST_CHECK_EQUAL( "name1 = value1", rdr.line() );
    BOOST_CHECK_EQUAL( "name1", rdr.name() );
    BOOST_CHECK_EQUAL( "value1", rdr.value() );
    BOOST_CHECK_EQUAL( 5, rdr.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_path_str, rdr.current_filename() );
    BOOST_CHECK_NE( dummy_filename, rdr.current_filename() );
    // TEST reader next part 3
    BOOST_CHECK( rdr.next() );
    BOOST_CHECK_EQUAL( "1 2.3 4 not a num # 6.2", rdr.line() );
    BOOST_CHECK_EQUAL( "1", rdr.name() );
    BOOST_CHECK_EQUAL( "2.3 4 not a num", rdr.value() );
    BOOST_CHECK_EQUAL( 6, rdr.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_path_str, rdr.current_filename() );
    BOOST_CHECK_NE( dummy_filename, rdr.current_filename() );
    // TEST reader next part 4
    BOOST_CHECK( rdr.next() );
    BOOST_CHECK_EQUAL( "name1 value1", rdr.line() );
    BOOST_CHECK_EQUAL( "name1", rdr.name() );
    BOOST_CHECK_EQUAL( "value1", rdr.value() );
    BOOST_CHECK_EQUAL( 7, rdr.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_path_str, rdr.current_filename() );
    BOOST_CHECK_NE( dummy_filename, rdr.current_filename() );
    // TEST reader next part 5
    BOOST_CHECK( not rdr.next() );
    BOOST_CHECK_EQUAL( "", rdr.line() );
    BOOST_CHECK_EQUAL( "", rdr.name() );
    BOOST_CHECK_EQUAL( "", rdr.value() );
    // zero line number and "" indicate no file.
    BOOST_CHECK_EQUAL( 0, rdr.current_line_number() );
    BOOST_CHECK_EQUAL( "", rdr.current_filename() );
    // Check stability of calling next after last element.
    BOOST_CHECK( not rdr.next() );
    BOOST_CHECK_EQUAL( "", rdr.line() );
    BOOST_CHECK_EQUAL( "", rdr.name() );
    BOOST_CHECK_EQUAL( "", rdr.value() );
    // zero line number and "" indicate no file.
    BOOST_CHECK_EQUAL( 0, rdr.current_line_number() );
    BOOST_CHECK_EQUAL( "", rdr.current_filename() );
  }

}

//  Test  'input_base_meta' derived class base class functionality
//  - check no default ctor
//  - check no copy ability
//  - check virtual dtor
//  - check three argument ctor : label, is_multiple, is_required
template< class Meta > void core_test_suite::test_input_base_meta()
{
  //// CTOR TESTS
  {
    // Static Lifetime method tests
    BOOST_CHECK( not typename std::is_default_constructible< Meta >::type{} );
    BOOST_CHECK( not typename std::is_copy_constructible< Meta >::type{} );
    BOOST_CHECK( not typename std::is_move_constructible< Meta >::type{} );
    BOOST_CHECK( not (typename std::is_assignable< Meta, Meta >::type{}) );
    BOOST_CHECK( typename std::has_virtual_destructor< Meta >::type{} );
  }
  // ----------
  const std::string label( "name" );
  {
    std::unique_ptr< core::input_base_meta > dobj( new Meta( label, false, false ) );
    BOOST_CHECK_EQUAL( dobj->section_label(), label );
    BOOST_CHECK_EQUAL( dobj->multiple(), false );
    BOOST_CHECK_EQUAL( dobj->required(), false );
  }
  {
    std::unique_ptr< core::input_base_meta > dobj( new Meta( label, true, false ) );
    BOOST_CHECK_EQUAL( dobj->section_label(), label );
    BOOST_CHECK_EQUAL( dobj->multiple(), true );
    BOOST_CHECK_EQUAL( dobj->required(), false );
  }
  {
    std::unique_ptr< core::input_base_meta > dobj( new Meta( label, false, true ) );
    BOOST_CHECK_EQUAL( dobj->section_label(), label );
    BOOST_CHECK_EQUAL( dobj->multiple(), false );
    BOOST_CHECK_EQUAL( dobj->required(), true );
  }
  {
    std::unique_ptr< core::input_base_meta > dobj( new Meta( label, true, true ) );
    BOOST_CHECK_EQUAL( dobj->section_label(), label );
    BOOST_CHECK_EQUAL( dobj->multiple(), true );
    BOOST_CHECK_EQUAL( dobj->required(), true );
  }
  
  

}

//
//Tests the following with valid input
// - empty, size
// - has_section
// - add_section
// - add_option
// - print : 1 and 2 arg version
BOOST_AUTO_TEST_CASE( input_help_test )
{
  {
    // Static Lifetime method tests
  
    BOOST_CHECK( std::is_default_constructible< core::input_help >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< core::input_help >::type{} );
    BOOST_CHECK( not std::is_move_constructible< core::input_help >::type{} );
    BOOST_CHECK( ( not std::is_assignable< core::input_help, core::input_help >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< core::input_help >::type{} );
  }
  
  std::string section1( core::strngs::fsspec() );
  std::string section2( core::strngs::fssalt() );
  std::string sectdesc1( "Specie definition input section variables and expected values." );
  std::string sectdesc2( "Salt definition input section variables and expected values." );
  core::help_entry entry1( core::strngs::fsname(), "two letters", "", "required", "Specie code name (quotes optional)." );
  core::help_entry entry2( core::strngs::fsrtgr(), "number", "[0,1)", "required", "\""+core::strngs::fsfree()+"\" type only, Relative probability this specie is used in an individual ion grand-canonical trial compared to other species." );
  core::help_entry entry3( core::strngs::fsname(), "four letters", "", "required", "Salt code name (quotes optional), letters should be code of the two species that make up the salt (e.g. NaCl, CaCl)." );
  
  {
    // test adding a section
    core::input_help tmp;
  
    BOOST_CHECK( tmp.empty() );
    BOOST_CHECK_EQUAL( tmp.size(), 0ul );
    BOOST_CHECK( not tmp.has_section( section1 ) );
  
    tmp.add_section( { section1, sectdesc1 } );
  
    BOOST_CHECK( not tmp.empty() );
    BOOST_CHECK_EQUAL( tmp.size(), 1ul );
    BOOST_CHECK( tmp.has_section( section1 ) );
  
    // test adding an option after adding section
    tmp.get_section( section1 ).add_entry( entry1 );
  
    {
    // test adding a section (with description) after section has been added.
    // This should simply add the description to the existing option.
    try
    {
      tmp.add_section( { section1, sectdesc2 } );
      BOOST_ERROR( "expected \"tmp.add_section\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Can not add two sections with the same name: specie" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"tmp.add_section\" was not expected type: " ) + err.what() );
    }
    }
  }
  {
    // test adding two sections
    core::input_help tmp;
  
    BOOST_CHECK( tmp.empty() );
    BOOST_CHECK_EQUAL( tmp.size(), 0ul );
    BOOST_CHECK( not tmp.has_section( section1 ) );
  
    tmp.add_section( { section1, sectdesc1 } );
  
    BOOST_CHECK( not tmp.empty() );
    BOOST_CHECK_EQUAL( tmp.size(), 1ul );
    BOOST_CHECK( tmp.has_section( section1 ) );
  
    // test adding an option
    tmp.get_section( section1 ).add_entry( entry1 );
    tmp.get_section( section1 ).add_entry( entry2 );
  
    BOOST_CHECK( not tmp.has_section( section2 ) );
    tmp.add_section( { section2, sectdesc2 } );
  
    BOOST_CHECK_EQUAL( tmp.size(), 2ul );
    BOOST_CHECK( tmp.has_section( section1 ) );
    BOOST_CHECK( tmp.has_section( section2 ) );
  
    // test adding an option (note entry2 already on section1 )
    tmp.get_section( section2 ).add_entry( entry2 );
    tmp.get_section( section2 ).add_entry( entry3 );
  
    {
      // test of printing a section
      std::stringstream os;
      tmp.write( section1, os );
      //std::cout << "--\n" << os.rdbuf() << "--\n";
      const std::string output( os.str() );
      boost::tokenizer<> tok( output );
      boost::tokenizer<> toksearch1( sectdesc1 );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch1.begin(), toksearch1.end() ) );
      // Check description of section 2 is not present
      boost::tokenizer<> toksearch2( sectdesc2 );
      BOOST_CHECK( tok.end() == std::search( tok.begin(), tok.end(), toksearch2.begin(), toksearch2.end() ) );
      boost::tokenizer<> toksearch3( entry1.description() );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch3.begin(), toksearch3.end() ) );
      boost::tokenizer<> toksearch4( entry3.description() );
      BOOST_CHECK( tok.end() == std::search( tok.begin(), tok.end(), toksearch4.begin(), toksearch4.end() ) );
    }
    {
      // test printing everything
      std::stringstream os;
      tmp.write( os );
      //std::cout << "--\n" << os.rdbuf() << "--\n";
      const std::string output( os.str() );
      boost::tokenizer<> tok( output );
      boost::tokenizer<> toksearch1( sectdesc1 );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch1.begin(), toksearch1.end() ) );
      // Check description of section 2 is present
      boost::tokenizer<> toksearch2( sectdesc2 );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch2.begin(), toksearch2.end() ) );
      boost::tokenizer<> toksearch3( entry2.description() );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch3.begin(), toksearch3.end() ) );
      boost::tokenizer<> toksearch4( entry3.description() );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch4.begin(), toksearch4.end() ) );
    }
    {
      // test printing parameter
      std::stringstream os;
      tmp.write( section1, core::strngs::fsname(), os );
      //std::cout << "--\n" << os.rdbuf() << "--\n";
      const std::string output( os.str() );
      boost::tokenizer<> tok( output );
      boost::tokenizer<> toksearch1( sectdesc1 );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch1.begin(), toksearch1.end() ) );
      boost::tokenizer<> toksearch3( entry1.description() );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch3.begin(), toksearch3.end() ) );
    }
    {
      // test printing parameter
      std::stringstream os;
      tmp.write( "", core::strngs::fsname(), os );
      //std::cout << "--\n" << os.rdbuf() << "--\n";
      const std::string output( os.str() );
      boost::tokenizer<> tok( output );
      // Check description of section 2 is present
      boost::tokenizer<> toksearch1( sectdesc2 );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch1.begin(), toksearch1.end() ) );
      boost::tokenizer<> toksearch4( entry3.description() );
      BOOST_CHECK( tok.end() != std::search( tok.begin(), tok.end(), toksearch4.begin(), toksearch4.end() ) );
    }
  
  }
  
  

}

// Test strngs functions give expected value
//
//  - 89 strings tested
BOOST_AUTO_TEST_CASE( dictionary_test )
{
  // A
  BOOST_CHECK_EQUAL( core::strngs::fsaccu(), "accum" );
  BOOST_CHECK_EQUAL( core::strngs::fsadd(),  "add" );
  // B
  BOOST_CHECK_EQUAL( core::strngs::bulk_label(), "bulk" );
  // C
  BOOST_CHECK_EQUAL( core::strngs::fscgin(), "calgin" );
  BOOST_CHECK_EQUAL( core::strngs::fschex(), "chex" );
  BOOST_CHECK_EQUAL( core::strngs::fschnl(), "channel" );
  BOOST_CHECK_EQUAL( core::strngs::fschon(), "chonly" );
  BOOST_CHECK_EQUAL( core::strngs::fschpt(), "usepot" );
  BOOST_CHECK_EQUAL( core::strngs::fsclac(), "calacc" );
  BOOST_CHECK_EQUAL( core::strngs::fsclmb(), "calmob" );
  BOOST_CHECK_EQUAL( core::strngs::fsconf(), "conf" );
  BOOST_CHECK_EQUAL( core::strngs::fscrdf(), "calrdf" );
  BOOST_CHECK_EQUAL( core::strngs::fsctrg(), "ctarg" );
  BOOST_CHECK_EQUAL( core::strngs::comment_begin(), "#" );
  BOOST_CHECK_EQUAL( core::strngs::comment_end(), "" );
  // D
  BOOST_CHECK_EQUAL( core::strngs::fsd(),    "d" );
  BOOST_CHECK_EQUAL( core::strngs::fsdrg(),  "drg" );
  BOOST_CHECK_EQUAL( core::strngs::fsdrmi(), "drmaxin" );
  BOOST_CHECK_EQUAL( core::strngs::fsdrmo(), "drmaxout" );
  BOOST_CHECK_EQUAL( core::strngs::fsdxf(),  "dxf" );
  BOOST_CHECK_EQUAL( core::strngs::fsdxw(),  "dxw" );
  BOOST_CHECK_EQUAL( core::strngs::fsdzg(),  "dzg" );
  // E
  BOOST_CHECK_EQUAL( core::strngs::fsend(),  "end" );
  BOOST_CHECK_EQUAL( core::strngs::fsenth(), "enthalpy" );
  BOOST_CHECK_EQUAL( core::strngs::fsentr(), "entropy" );
  BOOST_CHECK_EQUAL( core::strngs::fsepsc(), "epsch" );
  BOOST_CHECK_EQUAL( core::strngs::fsepsp(), "epspr" );
  BOOST_CHECK_EQUAL( core::strngs::fsepsw(), "epsw" );
  BOOST_CHECK_EQUAL( core::strngs::fsexct(), "excited" );
  BOOST_CHECK_EQUAL( core::strngs::evaluator_label(), "evaluator" );
  // F
  BOOST_CHECK_EQUAL( core::strngs::fsflxd(), "flex" );
  BOOST_CHECK_EQUAL( core::strngs::fsfree(), "free" );
  BOOST_CHECK_EQUAL( core::strngs::fsfver(), "fileversion" );
  // G
  BOOST_CHECK_EQUAL( core::strngs::fsgeom(), "geom" );
  BOOST_CHECK_EQUAL( core::strngs::fsgrid(), "usegrid" );
  BOOST_CHECK_EQUAL( core::strngs::fsgrl1(), "rl1" );
  BOOST_CHECK_EQUAL( core::strngs::fsgrl4(), "rl4" );
  BOOST_CHECK_EQUAL( core::strngs::fsgrl5(), "rl5" );
  BOOST_CHECK_EQUAL( core::strngs::fsgrlc(), "rlcurv" );
  BOOST_CHECK_EQUAL( core::strngs::fsgrlv(), "rlvest" );
  BOOST_CHECK_EQUAL( core::strngs::fsgrnd(), "ground" );
  BOOST_CHECK_EQUAL( core::strngs::fsgzl1(), "zl1" );
  BOOST_CHECK_EQUAL( core::strngs::fsgzl4(), "zl4" );
  BOOST_CHECK_EQUAL( core::strngs::fsgzlm(), "zlimit" );
  BOOST_CHECK_EQUAL( core::strngs::fsgzoc(), "zocc" );
  // H
  BOOST_CHECK_EQUAL( core::strngs::horizontal_bar(), "----------------------------------------------------------------------" );
  // I
  BOOST_CHECK_EQUAL( core::strngs::fsincl(), "include" );
  BOOST_CHECK_EQUAL( core::strngs::fsisav(), "isave" );
  BOOST_CHECK_EQUAL( core::strngs::fsislt(), "cation" );
  BOOST_CHECK_EQUAL( core::strngs::fsiwid(), "iwidom" );
  BOOST_CHECK_EQUAL( core::strngs::imc_label(), "super-looper" );
  BOOST_CHECK_EQUAL( core::strngs::inner_label(), "inner" );
  BOOST_CHECK_EQUAL( core::strngs::inputpattern_label(), "input" );
  // J
  // K
  BOOST_CHECK_EQUAL( core::strngs::fskmob(), "mobk" );
  // L
  BOOST_CHECK_EQUAL( core::strngs::localizer_label(), "localize" );
  // M
  BOOST_CHECK_EQUAL( core::strngs::fsmobl(), "mob" );
  // N
  BOOST_CHECK_EQUAL( core::strngs::fsname(), "name" );
  BOOST_CHECK_EQUAL( core::strngs::fsnavr(), "naver" );
  BOOST_CHECK_EQUAL( core::strngs::fsnmcf(), "multiconf" );
  BOOST_CHECK_EQUAL( core::strngs::fsn(),    "n" );
  BOOST_CHECK_EQUAL( core::strngs::fsnoch(), "nocharge" );
  BOOST_CHECK_EQUAL( core::strngs::fsnsrt(), "oldreg" );
  BOOST_CHECK_EQUAL( core::strngs::fsnstp(), "nstep" );
  BOOST_CHECK_EQUAL( core::strngs::fsnsub(), "nsub" );
  BOOST_CHECK_EQUAL( core::strngs::fsntrg(), "ntarg" );
  BOOST_CHECK_EQUAL( core::strngs::fsnblk(), "nbulk" );
  // O
  BOOST_CHECK_EQUAL( core::strngs::outputdir_label(), "outputdir" );
  // P
  BOOST_CHECK_EQUAL( core::strngs::fsptch(), "patch" );
  // Q
  // R
  BOOST_CHECK_EQUAL( core::strngs::fsregn(), "region" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtex(), "ratexc" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtgr(), "ratgr" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtid(), "ratind" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtjp(), "ratjmp" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtmv(), "ratmov" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtrg(), "ratreg" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtsl(), "ratslt" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtsp(), "ratspc" );
  BOOST_CHECK_EQUAL( core::strngs::fsrtsw(), "ratswap" );
  BOOST_CHECK_EQUAL( core::strngs::rate_label(), "rate" );
  // S
  BOOST_CHECK_EQUAL( core::strngs::fssalt(), "salt" );
  BOOST_CHECK_EQUAL( core::strngs::fsspec(), "specie" );
  BOOST_CHECK_EQUAL( core::strngs::fssubs(), "subspecie" );
  BOOST_CHECK_EQUAL( core::strngs::sampler_label(), "sampler" );
  BOOST_CHECK_EQUAL( core::strngs::simulator_label(), "simulator" );
  // T
  BOOST_CHECK_EQUAL( core::strngs::fstry(),  "trial" );
  BOOST_CHECK_EQUAL( core::strngs::fstsi(),  "kelvin" );
  BOOST_CHECK_EQUAL( core::strngs::fstype(), "type" );
  // U, V
  // W
  BOOST_CHECK_EQUAL( core::strngs::fswidm(), "calwid" );
  // X, Y
  // Z
  BOOST_CHECK_EQUAL( core::strngs::fsz(),    "z" );
  

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_error_lifetime_test )
{
  // Static Lifetime method tests
  
  // NOTE, not default constructible because it is an abstract class
  BOOST_CHECK( not std::is_default_constructible< core::input_error >::type{} );
  BOOST_CHECK( std::is_copy_constructible< core::input_error >::type{} );
  BOOST_CHECK( std::is_move_constructible< core::input_error >::type{} );
  BOOST_CHECK( (std::is_assignable< core::input_error, core::input_error >::type{}) );
  BOOST_CHECK( std::has_virtual_destructor< core::input_error >::type{} );

}

// Test input error exception class.
BOOST_AUTO_TEST_CASE( input_error_test )
{
  BOOST_CHECK_EQUAL( core::input_error::bad_boolean_message(), "Expected a boolean value (ie. true|false)." );
  
  BOOST_CHECK_EQUAL( str( core::input_error::bad_key_message_format() % "(a, b, c, d)" ), "A value from the list (a, b, c, d) was expected." );
  
  BOOST_CHECK_EQUAL( core::input_error::non_empty_value_message(), "Expected a non-empty value." );
  
  BOOST_CHECK_EQUAL( core::input_error::number_outside_range_message(), "Numeric value given is outside numeric range." );
  
  BOOST_CHECK_EQUAL( core::input_error::number_nan_message(), "Not-a-number (NaN) values are not allowed in input." );
  
  BOOST_CHECK_EQUAL( core::input_error::number_inf_message(), "Infinity (inf) values are not allowed in input." );
  
  BOOST_CHECK_EQUAL( core::input_error::number_negative_message(), "Input value must be greater than or equal to zero." );
  
  BOOST_CHECK_EQUAL( core::input_error::number_zero_message(), "Input value must be greater than zero." );
  
  BOOST_CHECK_EQUAL( core::input_error::number_bad_ordinal_message(), "A positive integer value was expected." );
  
  BOOST_CHECK_EQUAL( core::input_error::number_bad_message(), "A numeric value was expected." );
  
  {
    const std::string msg{ "Title parameter \"name\" with value (value) in section \"section\".\n** message **" };
    core::input_error err{ core::input_error::parameter_value_error( "Title", "section", "name", "value", nullptr, "message" ) };
    BOOST_CHECK_EQUAL( err.parameter(), "name" );
    BOOST_CHECK_EQUAL( err.section(), "section" );
  
    //std::cout << err.what() << "\n";
    BOOST_CHECK( std::string( err.what() ).find( msg ) != std::string::npos );
  }

}

// Check strngs::has_spaces.
BOOST_AUTO_TEST_CASE( strngs_has_spaces_test )
{
  std::vector< std::pair< std::string, std::size_t > > words;
  words.push_back( std::make_pair( "test case", true ) );
  words.push_back( std::make_pair( " case", true ) );
  words.push_back( std::make_pair( "case ", true ) );
  words.push_back( std::make_pair( "case\tpp", true ) );
  words.push_back( std::make_pair( "case\npp", true ) );
  words.push_back( std::make_pair( "case", false ) );
  words.push_back( std::make_pair( "ca4e", false ) );
  words.push_back( std::make_pair( "456.7", false ) );
  for( auto item : words )
  {
    BOOST_CHECK_EQUAL( core::strngs::has_spaces( item.first ), item.second );
  }
  

}

// Check strngs::is_one_word
BOOST_AUTO_TEST_CASE( strngs_is_one_word_test )
{
  std::vector< std::pair< std::string, std::size_t > > words;
  words.push_back( std::make_pair( "test case", false ) );
  words.push_back( std::make_pair( " case", false ) );
  words.push_back( std::make_pair( "case ", false ) );
  words.push_back( std::make_pair( "case\tpp", false ) );
  words.push_back( std::make_pair( "case\npp", false ) );
  words.push_back( std::make_pair( "case", true ) );
  words.push_back( std::make_pair( "ca4e", false ) );
  words.push_back( std::make_pair( "456.7", false ) );
  for( auto item : words )
  {
    BOOST_CHECK_EQUAL( core::strngs::is_one_word( item.first ), item.second );
  }
  

}

// Check strngs::is_one_word
BOOST_AUTO_TEST_CASE( strngs_is_valid_name )
{
  std::vector< std::pair< std::string, std::size_t > > words;
  words.push_back( std::make_pair( "test case", false ) );
  words.push_back( std::make_pair( " case", false ) );
  words.push_back( std::make_pair( "case ", false ) );
  words.push_back( std::make_pair( "case\tpp", false ) );
  words.push_back( std::make_pair( "case\npp", false ) );
  words.push_back( std::make_pair( "case_pp", true ) );
  words.push_back( std::make_pair( "case_", true ) );
  words.push_back( std::make_pair( "case", true ) );
  words.push_back( std::make_pair( "ca4e", true ) );
  words.push_back( std::make_pair( "456.7", true ) );
  for( auto item : words )
  {
    BOOST_CHECK_EQUAL( core::strngs::is_valid_name( item.first ), item.second );
  }
  

}

BOOST_AUTO_TEST_CASE( numerical_constants_test )
{
  // Invariant : file_version always less/equal to file_version_max
  BOOST_CHECK_EQUAL( core::constants::filver, 1 );
  BOOST_CHECK_EQUAL( core::constants::fvermx, 1 );
  BOOST_CHECK_LE( core::constants::filver, core::constants::fvermx );
  // TEST CONSTANTS HAVE FIXED VALUES
  BOOST_CHECK_EQUAL( core::constants::angstrom(), 1.0E-10 );
  BOOST_CHECK_EQUAL( core::constants::avogadro_number(), 6.02214E23 );
  BOOST_CHECK_EQUAL( core::constants::boltzmann_constant(), 1.3806E-23 );
  BOOST_CHECK_EQUAL( core::constants::electron_charge(), 1.6021917E-19 );
  BOOST_CHECK_EQUAL( core::constants::epsilon_0(), 8.8542E-12 );
  BOOST_CHECK_EQUAL( core::constants::pi(), 3.141592653589793 );
  BOOST_CHECK_EQUAL( core::constants::to_SI(), 1660.539276735512625080121 );
  
  

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_lifetime_test )
{
  // Static Lifetime method tests
  
  // NOTE, not default constructible because it is an abstract class
  BOOST_CHECK( not std::is_default_constructible< core::input_base_reader >::type{} );
  BOOST_CHECK( not std::is_copy_constructible< core::input_base_reader >::type{} );
  BOOST_CHECK( not std::is_move_constructible< core::input_base_reader >::type{} );
  BOOST_CHECK( (not std::is_assignable< core::input_base_reader, core::input_base_reader >::type{}) );
  BOOST_CHECK( std::has_virtual_destructor< core::input_base_reader >::type{} );

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_decomment_test )
{
  // TEST Decommenting
  const std::string input1("#abc  '"); // empty
  
  const std::string expect2(" ");
  const std::string input2(" #abc  "); // -> | |
  
  const std::string expect3("abc  ");
  const std::string input3("abc  # "); // -> |abc  |
  
  const std::string input4("abc   ");  // unchanged
  
  const std::string input5("abc  \\# "); // unchanged
  
  const std::string input6("\"abc  # \""); // unchanged
  
  const std::string expect7("\'abc\'  ");
  const std::string input7("\'abc\'  # ");
  
  const std::string input8("\"abc\"  \"# "); // -> unchanged
  
  const std::string expect9("\"abc\'  \"");
  const std::string input9("\"abc\'  \"# ");
  
  const std::string expectA("abc  \\\\");
  const std::string inputA("abc  \\\\# ");
  std::string result;
  
  result = core::input_base_reader::decomment(input1);
  BOOST_CHECK(result.empty());
  
  result = core::input_base_reader::decomment(input2);
  BOOST_CHECK_EQUAL(result, expect2);
  
  result = core::input_base_reader::decomment(input3);
  BOOST_CHECK_EQUAL(result, expect3);
  
  result = core::input_base_reader::decomment(input4);
  BOOST_CHECK_EQUAL(result, input4);
  
  result = core::input_base_reader::decomment(input5);
  BOOST_CHECK_EQUAL(result, input5);
  
  result = core::input_base_reader::decomment(input6);
  BOOST_CHECK_EQUAL(result, input6);
  
  result = core::input_base_reader::decomment(input7);
  BOOST_CHECK_EQUAL(result, expect7);
  
  result = core::input_base_reader::decomment(input8);
  BOOST_CHECK_EQUAL(result, input8);
  
  result = core::input_base_reader::decomment(input9);
  BOOST_CHECK_EQUAL(result, expect9);
  
  result = core::input_base_reader::decomment(inputA);
  BOOST_CHECK_EQUAL(result, expectA);
  

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_dequote_test )
{
  // Test of dequoting
  {
    const std::string expect("abc  ");
    const std::string input1("'abc  '");   // -> |abc  |
    const std::string input2("\"abc  \""); // -> |abc  |
    const std::string input3("\"abc  '");  // unchanged
    const std::string input4("\"abc  \" ");// -> |abc  |
    const std::string input5("\"abc  ' "); // unchanged
    std::string result;
  
    result = core::input_base_reader::dequote(input1);
    BOOST_CHECK_EQUAL(result, expect);
  
    result = core::input_base_reader::dequote(input2);
    BOOST_CHECK_EQUAL(result, expect);
  
    result = core::input_base_reader::dequote(input3);
    BOOST_CHECK_EQUAL(result, input3);
  
    result = core::input_base_reader::dequote(input4);
    BOOST_CHECK_EQUAL(result, expect);
  
    result = core::input_base_reader::dequote(input5);
    BOOST_CHECK_EQUAL(result, input5);
  }
  {
    // No quotes
    // Unquoted text: unchanged
    std::string result = "The lazy dog.";
    const std::string pass = "The lazy dog.";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Uneven quoted (front) text: unchanged
    std::string result = "\"The lazy dog.";
    const std::string pass = "\"The lazy dog.";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Uneven quotes (back) text: unchanged
    std::string result = "The lazy dog.\"";
    const std::string pass = "The lazy dog.\"";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Matched quotes [\"] text: strip quotes
    std::string result = "\"The lazy dog.\"";
    const std::string pass = "The lazy dog.";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Matched multi-quoted text: strip one set of quotes
    std::string result = "\"\"The lazy dog.\"\"";
    const std::string pass = "\"The lazy dog.\"";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Matched quoted with one end embedded in text: unchanged
    std::string result = "\"The\" lazy dog.";
    const std::string pass = "\"The\" lazy dog.";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Matched quoted ['] text: strip quotes
    std::string result = "'The lazy dog.'";
    const std::string pass = "The lazy dog.";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Mismatched quoted [\"...'] text: unchanged
    std::string result = "\"The lazy dog.'";
    const std::string pass = "\"The lazy dog.'";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Matched quoted [\"] text with external spacing: strip quotes and
    // whitespace
    std::string result = "\"The lazy dog. \" ";
    const std::string pass = "The lazy dog. ";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }
  {
    // Matched quoted [\"] text with internal spacing: strip quotes, leave
    // internal whitespace
    std::string result = "\"The lazy dog. \"";
    const std::string pass = "The lazy dog. ";
    result = core::input_base_reader::dequote(result);
    BOOST_CHECK_EQUAL(result, pass);
  }

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_read_as_floats_test )
{
  // READ AS FLOATS
  {
    std::vector< double > result;
    BOOST_CHECK( core::input_base_reader::read_as_floats("-1.0 2.4 1.3", result) );
    BOOST_CHECK_EQUAL( result.size(), 3 );
    BOOST_CHECK_EQUAL( result[0], -1.0 );
    BOOST_CHECK_EQUAL( result[1], 2.4 );
    BOOST_CHECK_EQUAL( result[2], 1.3 );
  }
  {
    std::vector< double > result;
    BOOST_CHECK( not core::input_base_reader::read_as_floats("1 2 3 ABC 4", result) );
    BOOST_CHECK_EQUAL( result.size(), 3 );
    BOOST_CHECK_EQUAL( result[0], 1.0 );
    BOOST_CHECK_EQUAL( result[1], 2.0 );
    BOOST_CHECK_EQUAL( result[2], 3.0 );
  }
  {
    std::vector< double > result;
    BOOST_CHECK( core::input_base_reader::read_as_floats("1", result) );
    BOOST_CHECK_EQUAL(result.size(),1);
    BOOST_CHECK_EQUAL(result[0],1.0);
  }

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_read_as_bool_test )
{
  // TEST BOOL CONVERTERS
  {
    // Converting string to bool
    // Converting string to bool: true -> true
    const std::string test ("true");
    bool pass = false;
    bool target = false;
    pass = core::input_base_reader::read_as_bool( test, target );
    BOOST_CHECK_EQUAL(pass, true);
    BOOST_CHECK_EQUAL(target, true);
  }
  {
    // Converting string to bool
    // Converting string to bool: false -> false
    const std::string test ("false");
    bool pass = true;
    bool target = false;
    pass = core::input_base_reader::read_as_bool( test, target );
    BOOST_CHECK_EQUAL(pass, true);
    BOOST_CHECK_EQUAL(target, false);
  }
  {
    // Converting string to bool
    // Converting string to bool: 0 -> false
    const std::string test ("0");
    bool pass = true;
    bool target = false;
    pass = core::input_base_reader::read_as_bool( test, target );
    BOOST_CHECK_EQUAL(pass, true);
    BOOST_CHECK_EQUAL(target, false);
  }
  {
    // Converting string to bool
    // Converting string to bool: 1 -> true
    const std::string test ("1");
    bool pass = false;
    bool target = false;
    pass = core::input_base_reader::read_as_bool( test, target );
    BOOST_CHECK_EQUAL(pass, true);
    BOOST_CHECK_EQUAL(target, true);
  
  }
  {
    // Converting string to bool
    // Converting string to bool: .false. -> false
    const std::string test (".false.");
    bool pass = true;
    bool target = false;
    pass = core::input_base_reader::read_as_bool( test, target );
    BOOST_CHECK_EQUAL(pass, true);
    BOOST_CHECK_EQUAL(target, false);
  }
  {
    // Converting string to bool
    // Converting string to bool: 2.0 -> ?true?
    const std::string test ("2.0");
    bool pass = true;
    bool target = false;
    pass = core::input_base_reader::read_as_bool( test, target );
    BOOST_CHECK_EQUAL(pass, false);
    BOOST_CHECK_EQUAL(target, false);
  }
  {
    // Converting string to bool
    // Converting string to bool: 2 -> ?true?
    const std::string test ("2");
    bool pass = true;
    bool target = false;
    pass = core::input_base_reader::read_as_bool( test, target );
    BOOST_CHECK_EQUAL(pass, false);
    BOOST_CHECK_EQUAL(target, false);
  }

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_bool_input_test )
{
  // Test of bool_input
  //
  // arg1 = name
  // arg2 = value
  // arg3 = title
  // arg4 = section
  // arg5 = target
  // arg6 = has_default
  // arg7 = default_value
  // arg8 = reader_ptr (here == nullptr)
  
  // TEST BOOL CONVERTER
  const std::string name( "choice" );
  const std::string title( "Test Base Reader" );
  const std::string sec( "core" );
  {
    // VALID INPUTS
  
    // Converting string to bool
    // Converting string to bool: true -> true
    core::input_parameter_memo param;
    param.set_name( name );
    bool pass = false;
    param.set_value( "true" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, true);
  
    param.set_value( "false" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, false);
  
    param.set_value( "0" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, false);
  
    param.set_value( "1" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, true);
  
    param.set_value( ".true." );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, true);
  
    param.set_value( ".false." );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, false);
  
    param.set_value( "TRuE" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, true);
  
    param.set_value( "FaLSE" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, false);
  
    param.set_value( ".t." );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, true);
  
    param.set_value( ".f." );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, false);
  
    param.set_value( "T" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, true);
  
    param.set_value( "F" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, false);
  
    param.set_value( "\"T\"" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, true);
  
    param.set_value( "\'F\'" );
    core::input_base_reader::bool_input( param, title, sec, pass, false, false );
    BOOST_CHECK_EQUAL(pass, false);
  
    core::input_base_reader::bool_input( name, "", title, sec, pass, true, false, nullptr );
    BOOST_CHECK_EQUAL(pass, false);
  
    core::input_base_reader::bool_input( name, "\'\'", title, sec, pass, true, true, nullptr );
    BOOST_CHECK_EQUAL(pass, true);
  
  }
  // INVALID INPUTS
  
  {
    try
    {
      // UNRECOGNISED
      bool pass = false;
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "Fa" );
      core::input_base_reader::bool_input( param, title, sec, pass, false, false );
      BOOST_ERROR( "expected \"core::input_base_reader::bool_input\" exception on INVALUD input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"choice\" with value (Fa) in section \"core\".\n** Expected a boolean value (ie. true|false). **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      bool pass = false;
      // EMPTY WITH NO DEFAULT
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "\"\"" );
      core::input_base_reader::bool_input( param, title, sec, pass, false, false );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"choice\" with value (\"\") in section \"core\".\n** Expected a non-empty value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      bool pass = false;
      // EMPTY WITH NO DEFAULT
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "" );
      core::input_base_reader::bool_input( param, title, sec, pass, false, false );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"choice\" in section \"core\".\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      bool pass = false;
      // ONLY WHITESPACE CHARACTERS
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( " \t\n" );
      core::input_base_reader::bool_input( param, title, sec, pass, false, false );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"choice\" with value ( \t\n) in section \"core\".\n** Expected a non-empty value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
  }

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_float_input_test )
{
  {
    const std::string name( "measure" );
    const std::string title( "Test Base Reader" );
    const std::string sec( "core" );
    // VALID INPUTS
    {
      core::input_parameter_memo param;
      param.set_name( name );
      double targ{ 0 };
      param.set_value( "1" );
      core::input_base_reader::float_input( param, title, sec, targ, false, false );
      BOOST_CHECK_EQUAL( targ, 1.0 );
      param.set_value( "100" );
      core::input_base_reader::float_input( param, title, sec, targ, false, false );
      BOOST_CHECK_EQUAL( targ, 100.0 );
      param.set_value( "31" );
      core::input_base_reader::float_input( param, title, sec, targ, false, false );
      BOOST_CHECK_EQUAL( targ, 31.0 );
      param.set_value( "184.125" );
      core::input_base_reader::float_input( param, title, sec, targ, false, false );
      BOOST_CHECK_EQUAL( targ, 184.125 );
      param.set_value( "0" );
      core::input_base_reader::float_input( param, title, sec, targ, false, false );
      BOOST_CHECK_EQUAL( targ, 0.0 );
      // Zero is not an eror
      param.set_value( "0" );
      core::input_base_reader::float_input( param, title, sec, targ, true, true );
      BOOST_CHECK_EQUAL( targ, 0 );
      // Negative is not normally an error
      param.set_value( "-10.5" );
      core::input_base_reader::float_input( param, title, sec, targ, false, false );
      BOOST_CHECK_EQUAL( targ, -10.5 );
    }
    // INVALID INPUTS
    {
      try
      {
        // EMPTY
        double targ{ 0 };
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "" );
        core::input_base_reader::float_input( param, title, sec, targ, true, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on EMPTY input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" in section \"core\".\n** Expected a value. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        // NEGATIVE when disallowed
        double targ{ 0 };
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "-1" );
        core::input_base_reader::float_input( param, title, sec, targ, true, true );
        BOOST_CHECK_EQUAL( targ, -1.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on NEGATIVE input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value (-1) in section \"core\".\n** Input value must be greater than or equal to zero. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        // ZERO when disallowed
        double targ{ 0 };
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "0" );
        core::input_base_reader::float_input( param, title, sec, targ, true, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on NEGATIVE input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value (0) in section \"core\".\n** Input value must be greater than zero. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
  
      try
      {
        double targ{ 0.0 };
        // OVERFLOW
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "184467440737.09E551616" );
        core::input_base_reader::float_input( param, title, sec, targ, false, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on bad input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value (184467440737.09E551616) in section \"core\".\n** Numeric value given is outside numeric range. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        double targ{ 0.0 };
        // TRIALING CHARACTERS
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "1KL" );
        core::input_base_reader::float_input( param, title, sec, targ, false, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on bad input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value (1KL) in section \"core\".\n** A numeric value was expected. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        double targ{ 0.0 };
        // ONLY WHITESPACE CHARACTERS
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( " \t\n" );
        core::input_base_reader::float_input( param, title, sec, targ, false, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on NEGATIVE input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value ( \t\n) in section \"core\".\n** Expected a non-empty value. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        double targ{ 0.0 };
        // LEADING CHARACTERS
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "K1" );
        core::input_base_reader::float_input( param, title, sec, targ, false, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on NEGATIVE input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value (K1) in section \"core\".\n** A numeric value was expected. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        double targ{ 0.0 };
        // NO NUMBER
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "ten" );
        core::input_base_reader::float_input( param, title, sec, targ, false, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on NEGATIVE input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value (ten) in section \"core\".\n** A numeric value was expected. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        double targ{ 0.0 };
        // INFINITY
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "-INF" );
        core::input_base_reader::float_input( param, title, sec, targ, false, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on NEGATIVE input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value (-INF) in section \"core\".\n** Infinity (inf) values are not allowed in input. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
  
      try
      {
        double targ{ 0.0 };
        // NOT-A-NUMBER
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "NAN" );
        core::input_base_reader::float_input( param, title, sec, targ, false, false );
        BOOST_CHECK_EQUAL( targ, 0.0 );
        BOOST_ERROR( "expected \"core::input_base_reader::float_input\" exception on NEGATIVE input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measure\" with value (NAN) in section \"core\".\n** Not-a-number (NaN) values are not allowed in input. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::float_input\" was not expected type: " ) + err.what() );
      }
  
    }
  }

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_floats_input_test )
{
  {
    // core::input_base_reader::floats_input( param, title, sec, targ );
    const std::string name( "measures" );
    const std::string title( "Test Base Reader" );
    const std::string sec( "core" );
    // VALID INPUTS
    {
      std::vector< double > targ{};
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "1 2 3" );
      core::input_base_reader::floats_input( param, title, sec, targ );
      BOOST_CHECK_EQUAL( targ.size(), 3 );
      BOOST_CHECK_EQUAL( targ[0], 1.0 );
      BOOST_CHECK_EQUAL( targ[1], 2.0 );
      BOOST_CHECK_EQUAL( targ[2], 3.0 );
      BOOST_CHECK_NO_THROW( core::input_base_reader::floats_input( name, "100", title, sec, targ, nullptr ) );
      BOOST_CHECK_EQUAL( targ.size(), 1 );
      BOOST_CHECK_EQUAL( targ[0], 100.0 );
      BOOST_CHECK_NO_THROW( core::input_base_reader::floats_input( name, "184.125 0 -10.5", title, sec, targ, nullptr ) );
      BOOST_CHECK_EQUAL( targ.size(), 3 );
      BOOST_CHECK_EQUAL( targ[0], 184.125 );
      BOOST_CHECK_EQUAL( targ[1], 0.0 );
      BOOST_CHECK_EQUAL( targ[2], -10.5 );
    }
    // INVALID INPUTS
    {
      try
      {
        std::vector< double > targ{};
        // EMPTY
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "" );
        core::input_base_reader::floats_input( param, title, sec, targ );
        BOOST_CHECK_EQUAL( targ.size(), 0ul );
        BOOST_ERROR( "expected \"core::input_base_reader::floats_input\" exception on EMPTY input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measures\" in section \"core\".\n** Expected a value. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::floats_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        std::vector< double > targ{};
        // OVERFLOW
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "0 184467440737.09E551616 1 2 3" );
        core::input_base_reader::floats_input( param, title, sec, targ );
        BOOST_CHECK_EQUAL( targ.size(), 0ul );
        BOOST_ERROR( "expected \"core::input_base_reader::floats_input\" exception on OVERFLOW input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measures\" with value (0 184467440737.09E551616 1 2 3) in section \"core\".\n** Element 184467440737.09E551616: Numeric value given is outside numeric range. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::floats_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        std::vector< double > targ{};
        // TRAILING CHARACTERS
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "4 1KL 1 2 3" );
        core::input_base_reader::floats_input( param, title, sec, targ );
        BOOST_CHECK_EQUAL( targ.size(), 0ul );
        BOOST_ERROR( "expected \"core::input_base_reader::floats_input\" exception on TRAILING CHARACTERS input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measures\" with value (4 1KL 1 2 3) in section \"core\".\n** Element KL: A numeric value was expected. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::floats_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        std::vector< double > targ{};
        // ONLY WHITESPACE CHARACTERS
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( " \t\n" );
        core::input_base_reader::floats_input( param, title, sec, targ );
        BOOST_CHECK_EQUAL( targ.size(), 0ul );
        BOOST_ERROR( "expected \"core::input_base_reader::floats_input\" exception on ONLY WHITESPACE CHARACTERS input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measures\" with value ( \t\n) in section \"core\".\n** Expected a non-empty value. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::floats_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        std::vector< double > targ{};
        // LEADING CHARACTERS
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "K1 1 2 3" );
        core::input_base_reader::floats_input( param, title, sec, targ );
        BOOST_CHECK_EQUAL( targ.size(), 0ul );
        BOOST_ERROR( "expected \"core::input_base_reader::floats_input\" exception on LEADING CHARACTERS input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measures\" with value (K1 1 2 3) in section \"core\".\n** Element K1: A numeric value was expected. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::floats_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        std::vector< double > targ{};
        // NOT A NUMBER
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "0 ten 1 2 3" );
        core::input_base_reader::floats_input( param, title, sec, targ );
        BOOST_CHECK_EQUAL( targ.size(), 0ul );
        BOOST_ERROR( "expected \"core::input_base_reader::floats_input\" exception on NOT A NUMBER input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measures\" with value (0 ten 1 2 3) in section \"core\".\n** Element ten: A numeric value was expected. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::floats_input\" was not expected type: " ) + err.what() );
      }
      try
      {
        std::vector< double > targ{};
        // INFINITY
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "0 -INF 1 2 3" );
        core::input_base_reader::floats_input( param, title, sec, targ );
        BOOST_CHECK_EQUAL( targ.size(), 0ul );
        BOOST_ERROR( "expected \"core::input_base_reader::floats_input\" exception on INFINITY input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measures\" with value (0 -INF 1 2 3) in section \"core\".\n** Element -INF: Infinity (inf) values are not allowed in input. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::floats_input\" was not expected type: " ) + err.what() );
      }
  
      try
      {
        std::vector< double > targ{};
        // NAN TEXT
        core::input_parameter_memo param;
        param.set_name( name );
        param.set_value( "0 NAN 1 2 3" );
        core::input_base_reader::floats_input( param, title, sec, targ );
        BOOST_CHECK_EQUAL( targ.size(), 0ul );
        BOOST_ERROR( "expected \"core::input_base_reader::floats_input\" exception on NON NUMERIC TEXT input not thrown" );
      }
      catch( core::input_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Test Base Reader parameter \"measures\" with value (0 NAN 1 2 3) in section \"core\".\n** Element NAN: Not-a-number (NaN) values are not allowed in input. **" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::floats_input\" was not expected type: " ) + err.what() );
      }
  
    }
  }

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_key_input_test )
{
  // core::input_base_reader::key_input( name, val, title, sec, targ, keylist, nullptr );
  const std::string name( "label" );
  const std::string title( "Test Base Reader" );
  const std::string sec( "core" );
  std::vector< std::string > keylist { { "a", "b", "c", "d" } };
  // VALID INPUTS
  {
    core::input_parameter_memo param;
    param.set_name( name );
    std::size_t targ{ 0ul };
    param.set_value( "a" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 0ul );
    param.set_value( "b" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 1ul );
    param.set_value( "c" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 2ul );
    param.set_value( "d" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 3ul );
    // with quotes
    param.set_value( "\"a\"" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );;
    BOOST_CHECK_EQUAL( targ, 0ul );
    // + leading space
    param.set_value( "\'  b\'" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 1ul );
    param.set_value( "\'b\'" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 1ul );
    param.set_value( "\"c\"" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 2ul );
    // + trailing space
    param.set_value( "\"c  \"" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 2ul );
    param.set_value( "\'d\'" );
    core::input_base_reader::key_input( param, title, sec, targ, keylist );
    BOOST_CHECK_EQUAL( targ, 3ul );
  }
  // INVALID INPUTS
  {
    try
    {
      // NOT IN LIST
      std::size_t targ{ 0ul };
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "f" );
      core::input_base_reader::key_input( param, title, sec, targ, keylist );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::key_input\" exception on NOT IN LIST input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value (f) in section \"core\".\n** A value from the list (a,b,c,d) was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::key_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::size_t targ{ 0ul };
      // ONLY WHITESPACE CHARACTERS
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( " \t\n" );
      core::input_base_reader::key_input( param, title, sec, targ, keylist );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::key_input\" exception on ONLY WHITESPACE CHARACTERS input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value ( \t\n) in section \"core\".\n** Expected a non-empty value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::key_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::size_t targ{ 0ul };
      // EMPTY
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "" );
      core::input_base_reader::key_input( param, title, sec, targ, keylist );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::key_input\" exception on EMPTY input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" in section \"core\".\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::key_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::size_t targ{ 0ul };
      // EMPTY QUOTES
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "\"\"" );
      core::input_base_reader::key_input( param, title, sec, targ, keylist );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::key_input\" exception on EMPTY QUOTES input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value (\"\") in section \"core\".\n** Expected a non-empty value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::key_input\" was not expected type: " ) + err.what() );
    }
  }

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_ordinal_input_test )
{
  // core::input_base_reader::ordinal_input( name, val, title, sec, targ, nullptr );
  const std::string name( "label" );
  const std::string title( "Test Base Reader" );
  const std::string sec( "core" );
  // VALID INPUTS
  {
    std::size_t targ{ 0ul };
    core::input_parameter_memo param;
    param.set_name( name );
    //BOOST_CHECK_NO_THROW(
    param.set_value( "1" );
    core::input_base_reader::ordinal_input( param, title, sec, targ ); // );
    BOOST_CHECK_EQUAL( targ, 1ul );
    param.set_value( "100" );
    core::input_base_reader::ordinal_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, 100ul );
    param.set_value( "31" );
    core::input_base_reader::ordinal_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, 31ul );
    param.set_value( "18446744073709551614" );
    core::input_base_reader::ordinal_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, 18446744073709551614ul );
    param.set_value( "0" );
    core::input_base_reader::ordinal_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, 0ul );
    // Leading whitespace is not an error
    param.set_value( " 1" );
    core::input_base_reader::ordinal_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, 1ul );
    // Trialing whitespace is not an error
    param.set_value( "10  " );
    core::input_base_reader::ordinal_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, 10ul );
  }
  // INVALID INPUTS
  {
    try
    {
      // NEGATIVE
      std::size_t targ{ 0ul };
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "-1" );
      core::input_base_reader::ordinal_input( param, title, sec, targ );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value (-1) in section \"core\".\n** Input value must be greater than or equal to zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::size_t targ{ 0ul };
      // OVERFLOW
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "18446744073709551616" );
      core::input_base_reader::ordinal_input( param, title, sec, targ );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value (18446744073709551616) in section \"core\".\n** Numeric value given is outside numeric range. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::size_t targ{ 0ul };
      // TRIALING CHARACTERS
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "1KL" );
      core::input_base_reader::ordinal_input( param, title, sec, targ );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value (1KL) in section \"core\".\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::size_t targ{ 0ul };
      // ONLY WHITESPACE CHARACTERS
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( " \t\n" );
      core::input_base_reader::ordinal_input( param, title, sec, targ );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value ( \t\n) in section \"core\".\n** Expected a non-empty value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::size_t targ{ 0ul };
      // LEADING CHARACTERS
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "K1" );
      core::input_base_reader::ordinal_input( param, title, sec, targ );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value (K1) in section \"core\".\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::size_t targ{ 0ul };
      // NOT A NUMBER
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "ten" );
      core::input_base_reader::ordinal_input( param, title, sec, targ );
      BOOST_CHECK_EQUAL( targ, 0ul );
      BOOST_ERROR( "expected \"core::input_base_reader::ordinal_input\" exception on NEGATIVE input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value (ten) in section \"core\".\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::ordinal_input\" was not expected type: " ) + err.what() );
    }
  }

}

// Test of input_base_reader static methods.
BOOST_AUTO_TEST_CASE( input_base_reader_text_input_test )
{
  // core::input_base_reader::text_input( name, val, title, sec, targ, nullptr );
  const std::string name( "label" );
  const std::string title( "Test Base Reader" );
  const std::string sec( "core" );
  // VALID INPUTS
  {
    std::string targ{};
    core::input_parameter_memo param;
    param.set_name( name );
    //BOOST_CHECK_NO_THROW(
    param.set_value( "1" );
    core::input_base_reader::text_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, "1" );
    param.set_value( "\"100\"" );
    core::input_base_reader::text_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, "100" );
    param.set_value( "twelve" );
    core::input_base_reader::text_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, "twelve" );
    param.set_value( "\"184" );
    core::input_base_reader::text_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, "\"184" );
    param.set_value( "\" TEN \"" );
    core::input_base_reader::text_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, " TEN " );
    // Leading whitespace is not an error
    param.set_value( " 1" );
    core::input_base_reader::text_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, " 1" );
    // Trialing whitespace is not an error
    param.set_value( "10  " );
    core::input_base_reader::text_input( param, title, sec, targ );
    BOOST_CHECK_EQUAL( targ, "10  " );
  }
  // INVALID INPUTS
  {
    try
    {
      // EMPTY
      std::string targ{};
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "" );
      core::input_base_reader::text_input( param, title, sec, targ );
      BOOST_ERROR( "expected \"core::input_base_reader::text_input\" exception on EMPTY input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" in section \"core\".\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::text_input\" was not expected type: " ) + err.what() );
    }
    try
    {
      std::string targ{};
      // EMPTY QUOTES
      core::input_parameter_memo param;
      param.set_name( name );
      param.set_value( "\"\"" );
      core::input_base_reader::text_input( param, title, sec, targ );
      BOOST_ERROR( "expected \"core::input_base_reader::text_input\" exception on EMPTY QUOTES input not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Test Base Reader parameter \"label\" with value (\"\") in section \"core\".\n** Expected a non-empty value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"core::input_base_reader::text_input\" was not expected type: " ) + err.what() );
    }
  }

}

// Test methods
//  - ACCESSORS: line, name, trimmed_line, value
//  - MUTATORS: next, set_line, clear

BOOST_AUTO_TEST_CASE( input_base_reader_test )
{
  ////////////
  // Simple (no-op) derived class to enable testing of base class methods
  class test_reader: public core::input_base_reader
  {
  public:
    bool do_next() override
    {
      return false;
    };
    std::string current_filename() const override
    {
      return std::string();
    }
    std::size_t current_line_number() const override
    {
      return 0;
    }
    void add_buffer(const boost::filesystem::path &, std::string) override {}
    void add_include(const boost::filesystem::path &) override {}
  };
  //--template < class Archive >
  //--void serialize( Archive &ar, input_base_reader &rr, const int version = 0)
  //--{
  //--   rr.serialize( ar );
  //--};
  {
    // TEST reader ctor
    test_reader test;
    BOOST_CHECK_EQUAL( test.line().size(), 0 );
    BOOST_CHECK_EQUAL( test.name().size(), 0 );
    BOOST_CHECK_EQUAL( test.value().size(), 0 );
  }
  std::stringstream store;
  {
    // TEST set_line
    test_reader test;
  
    // serialization
    boost::archive::text_oarchive oa( store );
    {
      // name only
      const std::string ll( "section" );
      BOOST_CHECK( test.set_line( ll ) );
      BOOST_CHECK_EQUAL( test.line(), ll );
      BOOST_CHECK_EQUAL( test.trimmed_line(), ll );
      BOOST_CHECK_EQUAL( test.name(), ll );
      BOOST_CHECK_EQUAL( test.value().size(), 0 );
    }
    {
      // name value with '=' separator
      const std::string ll( "name1 = value1" );
      BOOST_CHECK( test.set_line( ll ) );
      BOOST_CHECK_EQUAL( test.line(), ll );
      BOOST_CHECK_EQUAL( test.trimmed_line(), ll );
      BOOST_CHECK_EQUAL( test.name(), "name1" );
      BOOST_CHECK_EQUAL( test.value(), "value1" );
  
      // SAVE POINT 1
      oa << test;
  
      BOOST_CHECK_NE( test.line().size(), 0 );
      BOOST_CHECK_NE( test.name().size(), 0 );
      BOOST_CHECK_NE( test.value().size(), 0 );
      test.clear();
      BOOST_CHECK_EQUAL( test.line().size(), 0 );
      BOOST_CHECK_EQUAL( test.name().size(), 0 );
      BOOST_CHECK_EQUAL( test.value().size(), 0 );
    }
    {
      // name value with ' ' separator
      const std::string ll( "name2    value2" );
      BOOST_CHECK( test.set_line( ll ) );
      BOOST_CHECK_EQUAL( test.line(), ll );
      BOOST_CHECK_EQUAL( test.trimmed_line(), ll );
      BOOST_CHECK_EQUAL( test.name(), "name2" );
      BOOST_CHECK_EQUAL( test.value(), "value2" );
      BOOST_CHECK_EQUAL( test.get_text( "Big", "little" ), "value2" );
  
      // SAVE POINT 2
      oa << test;
  
      BOOST_CHECK_NE( test.line().size(), 0 );
      BOOST_CHECK_NE( test.name().size(), 0 );
      BOOST_CHECK_NE( test.value().size(), 0 );
      BOOST_CHECK( not test.next() );
      BOOST_CHECK_EQUAL( test.line().size(), 0 );
      BOOST_CHECK_EQUAL( test.name().size(), 0 );
      BOOST_CHECK_EQUAL( test.value().size(), 0 );
    }
    {
      // comment line
      const std::string ll( "# section" );
      BOOST_CHECK( not test.set_line( ll ) );
      BOOST_CHECK_EQUAL( test.line(), ll );
      BOOST_CHECK_EQUAL( test.trimmed_line(), "" );
      BOOST_CHECK_EQUAL( test.name().size(), 0 );
      BOOST_CHECK_EQUAL( test.value().size(), 0 );
    }
    {
      // empty line
      const std::string ll( "" );
      BOOST_CHECK( not test.set_line( ll ) );
      BOOST_CHECK_EQUAL( test.line(), ll );
      BOOST_CHECK_EQUAL( test.trimmed_line(), ll );
      BOOST_CHECK_EQUAL( test.name().size(), 0 );
      BOOST_CHECK_EQUAL( test.value().size(), 0 );
    }
    {
      // line with values separated by spaces and a comment
      const std::string ll( "1 2.3 4 not a num # COMMENT" );
      BOOST_CHECK( test.set_line( ll ) );
      BOOST_CHECK_EQUAL( test.line(), ll );
      BOOST_CHECK_EQUAL( test.trimmed_line(), "1 2.3 4 not a num" );
      BOOST_CHECK_EQUAL( test.name(), "1" );
      BOOST_CHECK_EQUAL( test.value(), "2.3 4 not a num" );
    }
    {
      // name only with a comment
      const std::string ll( "section # COMMENT" );
      BOOST_CHECK( test.set_line( ll ) );
      BOOST_CHECK_EQUAL( test.line(), ll );
      BOOST_CHECK_EQUAL( test.name(), "section" );
      BOOST_CHECK_EQUAL( test.value().size(), 0 );
    }
    {
      // value only ??
      const std::string ll( " = section" );
      BOOST_CHECK( test.set_line( ll ) );
      BOOST_CHECK_EQUAL( test.line(), ll );
      BOOST_CHECK_EQUAL( test.name().size(), 0  );
      BOOST_CHECK_EQUAL( test.value(), "section" );
    }
  
  }
  {
  
    boost::archive::text_iarchive ia( store );
    {
      test_reader test;
      // FROM SAVE POINT 1
      ia >> test;
      BOOST_CHECK_EQUAL( test.line(), "name1 = value1" );
      BOOST_CHECK_EQUAL( test.name(), "name1" );
      BOOST_CHECK_EQUAL( test.value(), "value1" );
    }
    {
      test_reader test;
      // FROM SAVE POINT 2
      ia >> test;
      BOOST_CHECK_EQUAL( test.line(), "name2    value2" );
      BOOST_CHECK_EQUAL( test.name(), "name2" );
      BOOST_CHECK_EQUAL( test.value(), "value2" );
    }
  }
  

}

//
//Tests expected operation of the following on a valid input string
// - process_string
// - getline
// - serialization
// - eof
BOOST_AUTO_TEST_CASE( input_node_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< core::input_node >::type{} );
    BOOST_CHECK( std::is_copy_constructible< core::input_node >::type{} );
    BOOST_CHECK( std::is_move_constructible< core::input_node >::type{} );
    BOOST_CHECK( (std::is_assignable< core::input_node, core::input_node >::type{}) );
    BOOST_CHECK( not std::has_virtual_destructor< core::input_node >::type{} );
  }
  
  const std::string dummy_input("\n\n# A comment\nsection # Another comment\nname1 = value1\n1 2.3 4 not a num # 6.2\nname1 value1\n\n");
  const std::string  dummy_filename("dummy");
  const boost::filesystem::path dummy_path( boost::filesystem::absolute( "dummy" ) );
  const std::vector< std::string > lines = { "", "", "# A comment", "section # Another comment", "name1 = value1", "1 2.3 4 not a num # 6.2", "name1 value1", "", "", "" };
  const std::vector< std::size_t > pos = { 1, 2, 14, 40, 55, 79, 92, 93, 94, 94 };
  const std::vector< std::size_t > linenums = { 1, 2, 3, 4, 5, 6, 7, 8, 8, 8 };
  
  std::stringstream store;
  {
    core::input_node node;
    node.set_buffer( dummy_filename, dummy_input );
  
    BOOST_CHECK_EQUAL( node.file_position(), 0 );
    BOOST_CHECK_EQUAL( node.line_number(), 0 );
    BOOST_CHECK_EQUAL( node.path(), dummy_path );
    BOOST_CHECK_EQUAL( node.eof(), false );
  
    boost::archive::text_oarchive oa( store );
    oa << node;
  
    for (std::size_t ii = 0; ii != 10; ++ii )
    {
      std::string line;
      node.getline( line );
      BOOST_CHECK( node.eof() == (ii >= 7) );
      BOOST_CHECK_EQUAL( line, lines[ii] );
      BOOST_CHECK_EQUAL( node.file_position(), pos[ii] );
      BOOST_CHECK_EQUAL( node.line_number(), linenums[ii] );
      BOOST_CHECK_EQUAL( node.path(), dummy_path );
    }
    node.rewind();
    BOOST_CHECK_EQUAL( node.file_position(), 0 );
    BOOST_CHECK_EQUAL( node.line_number(), 0 );
    BOOST_CHECK_EQUAL( node.path(), dummy_path );
    for (std::size_t ii = 0; ii != 8; ++ii )
    {
      std::string line;
      node.getline( line );
      BOOST_CHECK( node.eof() == (ii >= 7) );
      BOOST_CHECK_EQUAL( line, lines[ii] );
      BOOST_CHECK_EQUAL( node.file_position(), pos[ii] );
      BOOST_CHECK_EQUAL( node.line_number(), linenums[ii] );
      BOOST_CHECK_EQUAL( node.path(), dummy_path );
    }
  }
  {
    core::input_node node;
    boost::archive::text_iarchive ia( store );
    ia >> node;
    BOOST_CHECK_EQUAL( node.file_position(), 0 );
    BOOST_CHECK_EQUAL( node.line_number(), 0 );
    BOOST_CHECK_EQUAL( node.path(), dummy_path );
  
    for (std::size_t ii = 0; ii != 8; ++ii )
    {
      std::string line;
      node.getline( line );
      BOOST_CHECK( node.eof() == (ii >= 7) );
      BOOST_CHECK_EQUAL( line, lines[ii] );
      BOOST_CHECK_EQUAL( node.file_position(), pos[ii] );
      BOOST_CHECK_EQUAL( node.line_number(), linenums[ii] );
      BOOST_CHECK_EQUAL( node.path(), dummy_path );
    }
  }
  
  // Test of input that is a single line without \n
  {
    const std::string single_input("section ");
    core::input_node node;
    node.set_buffer( dummy_filename, single_input );
  
    BOOST_CHECK_EQUAL( node.file_position(), 0 );
    BOOST_CHECK_EQUAL( node.line_number(), 0 );
    BOOST_CHECK_EQUAL( node.path(), dummy_path );
    BOOST_CHECK_EQUAL( node.eof(), false );
  
    std::string line;
    node.getline( line );
    BOOST_CHECK( node.eof() );
    BOOST_CHECK_EQUAL( line, single_input );
    BOOST_CHECK_EQUAL( node.file_position(), single_input.size() );
    BOOST_CHECK_EQUAL( node.line_number(), 1 );
    BOOST_CHECK_EQUAL( node.path(), dummy_path );
  }
  
  // Test of empty input
  {
    const std::string empty_input{};
    core::input_node node;
    node.set_buffer( dummy_filename, empty_input );
  
    BOOST_CHECK_EQUAL( node.file_position(), 0 );
    BOOST_CHECK_EQUAL( node.line_number(), 0 );
    BOOST_CHECK_EQUAL( node.path(), dummy_path );
    BOOST_CHECK_EQUAL( node.eof(), true );
  }
  
  // Test of empty single line input
  {
    const std::string empty_input{"\n"};
    core::input_node node;
    node.set_buffer( dummy_filename, empty_input );
  
    BOOST_CHECK_EQUAL( node.file_position(), 0 );
    BOOST_CHECK_EQUAL( node.line_number(), 0 );
    BOOST_CHECK_EQUAL( node.path(), dummy_path );
    BOOST_CHECK_EQUAL( node.eof(), false );
    std::string line;
    node.getline( line );
    BOOST_CHECK( node.eof() );
    BOOST_CHECK_EQUAL( line, "" );
    BOOST_CHECK_EQUAL( node.file_position(), 1 );
    BOOST_CHECK_EQUAL( node.line_number(), 1 );
    BOOST_CHECK_EQUAL( node.path(), dummy_path );
  }
  

}

BOOST_AUTO_TEST_CASE( input_parameter_memo_test )
{
  const std::string dummy_input( "\n\n# A comment\nsection # Another comment\nname1 = value1\n1 2.3 4 not a num # 6.2\nname1 value1" );
  const std::string dummy_filename( boost::filesystem::absolute( "dummy" ).string() );
  std::stringstream store;
  const std::string empty_str;
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< core::input_parameter_memo >::type{} );
    BOOST_CHECK( std::is_copy_constructible< core::input_parameter_memo >::type{} );
    BOOST_CHECK( std::is_move_constructible< core::input_parameter_memo >::type{} );
    BOOST_CHECK( ( std::is_assignable< core::input_parameter_memo, core::input_parameter_memo >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< core::input_parameter_memo >::type{} );
  }
  {
    // TEST reader ctor
    core::input_parameter_memo test;
    BOOST_CHECK_EQUAL( empty_str, test.line() );
    BOOST_CHECK_EQUAL( empty_str, test.name() );
    BOOST_CHECK_EQUAL( empty_str, test.value() );
    BOOST_CHECK_EQUAL( 0, test.line_number() );
    BOOST_CHECK_EQUAL( empty_str, test.filename() );
  }
  {
    // TEST parameter_memo next part 1
    std::istringstream input( dummy_input );
    core::input_reader rdr( dummy_filename, input );
    rdr.next();
    {
      core::input_parameter_memo test( rdr );
      BOOST_CHECK_EQUAL( "section # Another comment",test.line() );
      BOOST_CHECK_EQUAL( "section",test.name() );
      BOOST_CHECK_EQUAL( "",test.value() );
      BOOST_CHECK_EQUAL( 4,test.line_number() );
      BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
      // copy ctor
      {
        core::input_parameter_memo test2( test );
        BOOST_CHECK_EQUAL( "section # Another comment",test2.line() );
        BOOST_CHECK_EQUAL( "section",test2.name() );
        BOOST_CHECK_EQUAL( "",test2.value() );
        BOOST_CHECK_EQUAL( 4,test2.line_number() );
        BOOST_CHECK_EQUAL( dummy_filename,test2.filename() );
      }
      boost::archive::text_oarchive oa( store );
      oa << test;
    }
    // TEST parameter_memo next part 2
    rdr.next();
    {
      core::input_parameter_memo test( rdr );
      BOOST_CHECK_EQUAL( "name1 = value1",test.line() );
      BOOST_CHECK_EQUAL( "name1",test.name() );
      BOOST_CHECK_EQUAL( "value1",test.value() );
      BOOST_CHECK_EQUAL( 5,test.line_number() );
      BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
      // assignment ctor
      {
        core::input_parameter_memo test2;
        BOOST_CHECK_EQUAL( empty_str, test2.line() );
        test2 = test;
        BOOST_CHECK_EQUAL( "name1 = value1",test2.line() );
        BOOST_CHECK_EQUAL( "name1",test2.name() );
        BOOST_CHECK_EQUAL( "value1",test2.value() );
        BOOST_CHECK_EQUAL( 5,test2.line_number() );
        BOOST_CHECK_EQUAL( dummy_filename,test2.filename() );
      }
    }
    // TEST parameter_memo next part 3
    rdr.next();
    {
      core::input_parameter_memo test( rdr );
      BOOST_CHECK_EQUAL( "1 2.3 4 not a num # 6.2",test.line() );
      BOOST_CHECK_EQUAL( "1",test.name() );
      BOOST_CHECK_EQUAL( "2.3 4 not a num",test.value() );
      BOOST_CHECK_EQUAL( 6,test.line_number() );
      BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
      // move ctor
      {
        core::input_parameter_memo test2( std::move( test ) );
        BOOST_CHECK_EQUAL( "1 2.3 4 not a num # 6.2",test2.line() );
        BOOST_CHECK_EQUAL( "1",test2.name() );
        BOOST_CHECK_EQUAL( "2.3 4 not a num",test2.value() );
        BOOST_CHECK_EQUAL( 6,test2.line_number() );
        BOOST_CHECK_EQUAL( dummy_filename,test2.filename() );
      }
    }
    // TEST parameter_memo next part 4
    rdr.next();
    {
      core::input_parameter_memo test( rdr );
      BOOST_CHECK_EQUAL( "name1 value1",test.line() );
      BOOST_CHECK_EQUAL( "name1",test.name() );
      BOOST_CHECK_EQUAL( "value1",test.value() );
      BOOST_CHECK_EQUAL( 7,test.line_number() );
      BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
      // copy from base ctor
      {
        core::input_parameter_memo test2( static_cast< const core::base_input_parameter& >( test ) );
        BOOST_CHECK_EQUAL( "name1 value1",test2.line() );
        BOOST_CHECK_EQUAL( "name1",test2.name() );
        BOOST_CHECK_EQUAL( "value1",test2.value() );
        BOOST_CHECK_EQUAL( 7,test2.line_number() );
        BOOST_CHECK_EQUAL( dummy_filename,test2.filename() );
      }
    }
  
    // TEST parameter_memo next part 5
    rdr.next();
    {
      core::input_parameter_memo test( rdr );
      // zero line number and "" indicate no file.
      BOOST_CHECK_EQUAL( empty_str, test.line() );
      BOOST_CHECK_EQUAL( empty_str, test.name() );
      BOOST_CHECK_EQUAL( empty_str, test.value() );
      BOOST_CHECK_EQUAL( 0, test.line_number() );
      BOOST_CHECK_EQUAL( empty_str, test.filename() );
    }
  }
  {
    core::input_parameter_memo test;
    boost::archive::text_iarchive ia( store );
    ia >> test;
    BOOST_CHECK_EQUAL( "section # Another comment",test.line() );
    BOOST_CHECK_EQUAL( "section",test.name() );
    BOOST_CHECK_EQUAL( "",test.value() );
    BOOST_CHECK_EQUAL( 4,test.line_number() );
    BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
  }

}

BOOST_AUTO_TEST_CASE( input_parameter_reference_test )
{
  const std::string dummy_input( "\n\n# A comment\nsection # Another comment\nname1 = value1\n1 2.3 4 not a num # 6.2\nname1 value1" );
  const std::string dummy_filename( boost::filesystem::absolute( "dummy" ).string() );
  const std::string empty_str;
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< core::input_parameter_reference >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< core::input_parameter_reference >::type{} );
    BOOST_CHECK( not std::is_move_constructible< core::input_parameter_reference >::type{} );
    BOOST_CHECK( not ( std::is_assignable< core::input_parameter_reference, core::input_parameter_reference >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< core::input_parameter_reference >::type{} );
  }
  {
    // TEST parameter_reference next part 1
    std::istringstream input( dummy_input );
    core::input_reader rdr( dummy_filename, input );
    rdr.next();
    {
      core::input_parameter_reference test( rdr );
      BOOST_CHECK_EQUAL( "section # Another comment",test.line() );
      BOOST_CHECK_EQUAL( "section",test.name() );
      BOOST_CHECK_EQUAL( "",test.value() );
      BOOST_CHECK_EQUAL( 4,test.line_number() );
      BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
    }
    // TEST parameter_reference next part 2
    rdr.next();
    {
      core::input_parameter_reference test( rdr );
      BOOST_CHECK_EQUAL( "name1 = value1",test.line() );
      BOOST_CHECK_EQUAL( "name1",test.name() );
      BOOST_CHECK_EQUAL( "value1",test.value() );
      BOOST_CHECK_EQUAL( 5,test.line_number() );
      BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
    }
    // TEST parameter_reference next part 3
    rdr.next();
    {
      core::input_parameter_reference test( rdr );
      BOOST_CHECK_EQUAL( "1 2.3 4 not a num # 6.2",test.line() );
      BOOST_CHECK_EQUAL( "1",test.name() );
      BOOST_CHECK_EQUAL( "2.3 4 not a num",test.value() );
      BOOST_CHECK_EQUAL( 6,test.line_number() );
      BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
      // TEST parameter_reference next part 4
      rdr.next();
      BOOST_CHECK_EQUAL( "name1 value1",test.line() );
      BOOST_CHECK_EQUAL( "name1",test.name() );
      BOOST_CHECK_EQUAL( "value1",test.value() );
      BOOST_CHECK_EQUAL( 7,test.line_number() );
      BOOST_CHECK_EQUAL( dummy_filename,test.filename() );
    }
  
    // TEST parameter_reference next part 5
    rdr.next();
    {
      core::input_parameter_reference test( rdr );
      // zero line number and "" indicate no file.
      BOOST_CHECK_EQUAL( empty_str, test.line() );
      BOOST_CHECK_EQUAL( empty_str, test.name() );
      BOOST_CHECK_EQUAL( empty_str, test.value() );
      BOOST_CHECK_EQUAL( 0, test.line_number() );
      BOOST_CHECK_EQUAL( empty_str, test.filename() );
    }
  }

}

BOOST_AUTO_TEST_CASE( input_preprocess_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< core::input_preprocess >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< core::input_preprocess >::type{} );
    BOOST_CHECK( not std::is_move_constructible< core::input_preprocess >::type{} );
    BOOST_CHECK( (not std::is_assignable< core::input_preprocess, core::input_preprocess >::type{}) );
    BOOST_CHECK( std::has_virtual_destructor< core::input_preprocess >::type{} );
  }
  {
     // TEST reader ctor
     core::input_preprocess test;
     BOOST_CHECK_EQUAL( "", test.line() );
     BOOST_CHECK_EQUAL( "", test.name() );
     BOOST_CHECK_EQUAL( "", test.value() );
     BOOST_CHECK_EQUAL( 0, test.current_line_number() );
     BOOST_CHECK_EQUAL( "", test.current_filename() );
  }
  {
     // Test standard reader behavior (uses memory buffers only). 
     core_test_suite::test_read_input_buffer< core::input_preprocess >();
  }
  // test serialization
  //
  // input_preprocess should read the content of the input files, including any
  // "include" files, into internal buffers that can be serialized.
  std::stringstream store;
  const std::string dummy_input("\n\n# A comment\nsection # Another comment\nname1 = value1\n1 2.3 4 not a num # 6.2\nname1 value1");
  const std::string  dummy_filename("dummy");
  const boost::filesystem::path dummy_path( boost::filesystem::absolute( "dummy" ) );
  {
     core::input_preprocess test;
     test.add_buffer( dummy_filename, dummy_input );
     boost::archive::text_oarchive oa( store );
     oa << test;
  }
  {
     core::input_preprocess rdr;
     boost::archive::text_iarchive ia( store );
     ia >> rdr;
  
     BOOST_CHECK( rdr.next() );
     BOOST_CHECK_EQUAL( "section # Another comment", rdr.line() );
     BOOST_CHECK_EQUAL( "section", rdr.name() );
     BOOST_CHECK_EQUAL( "", rdr.value() );
     BOOST_CHECK_EQUAL( 4, rdr.current_line_number() );
     BOOST_CHECK_EQUAL( dummy_path, rdr.current_filename() );
  
     // TEST reader next part 2
     BOOST_CHECK(rdr.next());
     BOOST_CHECK_EQUAL("name1 = value1",rdr.line());
     BOOST_CHECK_EQUAL("name1",rdr.name());
     BOOST_CHECK_EQUAL("value1",rdr.value());
     BOOST_CHECK_EQUAL(5,rdr.current_line_number());
     BOOST_CHECK_EQUAL(dummy_path,rdr.current_filename());
  
     // TEST reader next part 3
     BOOST_CHECK(rdr.next());
     BOOST_CHECK_EQUAL("1 2.3 4 not a num # 6.2",rdr.line());
     BOOST_CHECK_EQUAL("1",rdr.name());
     BOOST_CHECK_EQUAL("2.3 4 not a num",rdr.value());
     BOOST_CHECK_EQUAL(6,rdr.current_line_number());
     BOOST_CHECK_EQUAL(dummy_path,rdr.current_filename());
  
     // TEST reader next part 4
     BOOST_CHECK(rdr.next());
     BOOST_CHECK_EQUAL("name1 value1",rdr.line());
     BOOST_CHECK_EQUAL("name1",rdr.name());
     BOOST_CHECK_EQUAL("value1",rdr.value());
     BOOST_CHECK_EQUAL(7,rdr.current_line_number());
     BOOST_CHECK_EQUAL(dummy_path,rdr.current_filename());
  
     // TEST reader next part 5
     BOOST_CHECK(not rdr.next());
     BOOST_CHECK_EQUAL("",rdr.line());
     BOOST_CHECK_EQUAL("",rdr.name());
     BOOST_CHECK_EQUAL("",rdr.value());
  }
  

}

BOOST_AUTO_TEST_CASE( input_reader_test )
{
  std::string dummy_input( "\n\n# A comment\nsection # Another comment\nname1 = value1\n1 2.3 4 not a num # 6.2\nname1 value1" );
  std::string dummy_filename( boost::filesystem::absolute( "dummy" ).string() );
  
  {
    core_test_suite::test_read_input_buffer< core::input_reader >();
  }
  {
    // TEST reader ctor
    std::istringstream input( dummy_input );
    core::input_reader test( dummy_filename,input );
    BOOST_CHECK_EQUAL( "",test.line() );
    BOOST_CHECK_EQUAL( "",test.name() );
    BOOST_CHECK_EQUAL( "",test.value() );
    BOOST_CHECK_EQUAL( 0,test.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_filename,test.current_filename() );
  
  }
  {
    // TEST reader next part 1
    std::istringstream input( dummy_input );
    core::input_reader test( dummy_filename,input );
    BOOST_CHECK( test.next() );
    BOOST_CHECK_EQUAL( "section # Another comment",test.line() );
    BOOST_CHECK_EQUAL( "section",test.name() );
    BOOST_CHECK_EQUAL( "",test.value() );
    BOOST_CHECK_EQUAL( 4,test.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_filename,test.current_filename() );
  
    // TEST reader next part 2
    BOOST_CHECK( test.next() );
    BOOST_CHECK_EQUAL( "name1 = value1",test.line() );
    BOOST_CHECK_EQUAL( "name1",test.name() );
    BOOST_CHECK_EQUAL( "value1",test.value() );
    BOOST_CHECK_EQUAL( 5,test.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_filename,test.current_filename() );
  
    // TEST reader next part 3
    BOOST_CHECK( test.next() );
    BOOST_CHECK_EQUAL( "1 2.3 4 not a num # 6.2",test.line() );
    BOOST_CHECK_EQUAL( "1",test.name() );
    BOOST_CHECK_EQUAL( "2.3 4 not a num",test.value() );
    BOOST_CHECK_EQUAL( 6,test.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_filename,test.current_filename() );
  
    {
      std::vector< double > result;
      test.read_as_floats( test.line(), result );
      BOOST_CHECK_EQUAL( result.size(),3 );
      BOOST_CHECK_EQUAL( result[0],1.0 );
      BOOST_CHECK_EQUAL( result[1],2.3 );
      BOOST_CHECK_EQUAL( result[2],4.0 );
    }
    // TEST reader next part 4
    BOOST_CHECK( test.next() );
    BOOST_CHECK_EQUAL( "name1 value1",test.line() );
    BOOST_CHECK_EQUAL( "name1",test.name() );
    BOOST_CHECK_EQUAL( "value1",test.value() );
    BOOST_CHECK_EQUAL( 7,test.current_line_number() );
    BOOST_CHECK_EQUAL( dummy_filename,test.current_filename() );
  
    // TEST reader next part 5
    BOOST_CHECK( not test.next() );
    BOOST_CHECK_EQUAL( "",test.line() );
    BOOST_CHECK_EQUAL( "",test.name() );
    BOOST_CHECK_EQUAL( "",test.value() );
    // zero line number and "" indicate no file.
    BOOST_CHECK_EQUAL( 0,test.current_line_number() );
    BOOST_CHECK_EQUAL( "",test.current_filename() );
    // Check stability of calling next after last element.
    BOOST_CHECK( not test.next() );
    BOOST_CHECK_EQUAL( "",test.line() );
    BOOST_CHECK_EQUAL( "",test.name() );
    BOOST_CHECK_EQUAL( "",test.value() );
    // zero line number and "" indicate no file.
    BOOST_CHECK_EQUAL( 0,test.current_line_number() );
    BOOST_CHECK_EQUAL( "",test.current_filename() );
  
  }
  

}

//use the 'test_meta' class to test the non-virtual members of
//the abstract base class 'input_base_meta'
BOOST_AUTO_TEST_CASE( input_section_test )
{
  // CTOR TESTS
  // ----------
  const std::string empty_str;
  const std::string label1 { "label1" };
  const std::string name1 { "name1" };
  const std::string name2 { "name2" };
  const std::string name3 { "name3" };
  const std::string name4 { "name4" };
  const std::string value1 { "value1" };
  const double value2 { 1.25 };
  const std::string svalue2 { "1.250000" };
  const std::size_t value3 { 125ul };
  const std::string svalue3 { "125" };
  const bool value4 { true };
  const std::string svalue4 { "true" };
  std::stringstream store;
  {
     // default empty ctor
     core::input_section isec;
     BOOST_CHECK_EQUAL( isec.label(), empty_str );
     BOOST_CHECK_EQUAL( isec.size(), 0 );
     BOOST_CHECK_EQUAL( isec.empty(), true );
  
     isec.set_label( label1 );
     BOOST_CHECK_EQUAL( isec.label(), label1 );
     BOOST_CHECK_EQUAL( isec.size(), 0 );
     BOOST_CHECK_EQUAL( isec.empty(), true );
  
     BOOST_CHECK_EQUAL( isec.has_entry( name1 ), false );
     isec.add_entry( name1, value1 );
     BOOST_CHECK_EQUAL( isec.label(), label1 );
     BOOST_CHECK_EQUAL( isec.has_entry( name1 ), true );
     BOOST_CHECK_EQUAL( isec.get_entry( name1 ), value1 );
     BOOST_CHECK_EQUAL( isec.size(), 1 );
     BOOST_CHECK_EQUAL( isec.empty(), false );
  
     BOOST_CHECK_EQUAL( isec.has_entry( name2 ), false );
     isec.add_entry( name2, value2 );
     BOOST_CHECK_EQUAL( isec.has_entry( name2 ), true );
     BOOST_CHECK_EQUAL( isec.get_entry( name2 ), svalue2 );
     BOOST_CHECK_EQUAL( isec.size(), 2 );
  
     BOOST_CHECK_EQUAL( isec.has_entry( name3 ), false );
     isec.add_entry( name3, value3 );
     BOOST_CHECK_EQUAL( isec.has_entry( name3 ), true );
     BOOST_CHECK_EQUAL( isec.get_entry( name3 ), svalue3 );
     BOOST_CHECK_EQUAL( isec.size(), 3 );
  
     BOOST_CHECK_EQUAL( isec.has_entry( name4 ), false );
     isec.add_entry( name4, value4 );
     BOOST_CHECK_EQUAL( isec.has_entry( name4 ), true );
     BOOST_CHECK_EQUAL( isec.get_entry( name4 ), svalue4 );
     BOOST_CHECK_EQUAL( isec.size(), 4 );
  
     {
        // serialization
        boost::archive::text_oarchive oa( store );
        oa << isec;
        BOOST_CHECK_EQUAL( isec.label(), label1 );
        BOOST_CHECK_EQUAL( isec.has_entry( name1 ), true );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), value1 );
        BOOST_CHECK_EQUAL( isec.has_entry( name2 ), true );
        BOOST_CHECK_EQUAL( isec.get_entry( name2 ), svalue2 );
        BOOST_CHECK_EQUAL( isec.has_entry( name3 ), true );
        BOOST_CHECK_EQUAL( isec.get_entry( name3 ), svalue3 );
        BOOST_CHECK_EQUAL( isec.has_entry( name4 ), true );
        BOOST_CHECK_EQUAL( isec.get_entry( name4 ), svalue4 );
        BOOST_CHECK_EQUAL( isec.size(), 4 );
        BOOST_CHECK_EQUAL( isec.empty(), false );
     }
  
     {
        // copy ctor
        core::input_section isec_copy( isec );
        BOOST_CHECK_EQUAL( isec_copy.label(), label1 );
        BOOST_CHECK_EQUAL( isec_copy.has_entry( name1 ), true );
        BOOST_CHECK_EQUAL( isec_copy.get_entry( name1 ), value1 );
        BOOST_CHECK_EQUAL( isec_copy.has_entry( name2 ), true );
        BOOST_CHECK_EQUAL( isec_copy.get_entry( name2 ), svalue2 );
        BOOST_CHECK_EQUAL( isec_copy.has_entry( name3 ), true );
        BOOST_CHECK_EQUAL( isec_copy.get_entry( name3 ), svalue3 );
        BOOST_CHECK_EQUAL( isec_copy.has_entry( name4 ), true );
        BOOST_CHECK_EQUAL( isec_copy.get_entry( name4 ), svalue4 );
        BOOST_CHECK_EQUAL( isec_copy.size(), 4 );
        BOOST_CHECK_EQUAL( isec_copy.empty(), false );
     }
     {
        // assignment
        core::input_section isec_copy;
        BOOST_CHECK_EQUAL( isec_copy.label(), empty_str );
        BOOST_CHECK_EQUAL( isec_copy.size(), 0 );
        BOOST_CHECK_EQUAL( isec_copy.empty(), true );
  
        isec_copy = isec;
  
        BOOST_CHECK_EQUAL( isec_copy.label(), label1 );
        BOOST_CHECK_EQUAL( isec_copy.has_entry( name1 ), true );
        BOOST_CHECK_EQUAL( isec_copy.get_entry( name1 ), value1 );
        BOOST_CHECK_EQUAL( isec_copy.has_entry( name2 ), true );
        BOOST_CHECK_EQUAL( isec_copy.get_entry( name2 ), svalue2 );
        BOOST_CHECK_EQUAL( isec_copy.has_entry( name3 ), true );
        BOOST_CHECK_EQUAL( isec_copy.get_entry( name3 ), svalue3 );
        BOOST_CHECK_EQUAL( isec_copy.has_entry( name4 ), true );
        BOOST_CHECK_EQUAL( isec_copy.get_entry( name4 ), svalue4 );
        BOOST_CHECK_EQUAL( isec_copy.size(), 4 );
        BOOST_CHECK_EQUAL( isec_copy.empty(), false );
     }
  
     {
        // move
        core::input_section isec_copy( isec );
        core::input_section isec_move( std::move( isec_copy ) );
        BOOST_CHECK_EQUAL( isec_copy.label(), empty_str );
        BOOST_CHECK_EQUAL( isec_copy.size(), 0 );
        BOOST_CHECK_EQUAL( isec_copy.empty(), true );
  
        BOOST_CHECK_EQUAL( isec_move.label(), label1 );
        BOOST_CHECK_EQUAL( isec_move.has_entry( name1 ), true );
        BOOST_CHECK_EQUAL( isec_move.get_entry( name1 ), value1 );
        BOOST_CHECK_EQUAL( isec_move.has_entry( name2 ), true );
        BOOST_CHECK_EQUAL( isec_move.get_entry( name2 ), svalue2 );
        BOOST_CHECK_EQUAL( isec_move.has_entry( name3 ), true );
        BOOST_CHECK_EQUAL( isec_move.get_entry( name3 ), svalue3 );
        BOOST_CHECK_EQUAL( isec_move.has_entry( name4 ), true );
        BOOST_CHECK_EQUAL( isec_move.get_entry( name4 ), svalue4 );
        BOOST_CHECK_EQUAL( isec_move.size(), 4 );
        BOOST_CHECK_EQUAL( isec_move.empty(), false );
     }
  
     {
        // swap
        core::input_section isec_copy( isec );
        core::input_section isec_move;
  
        std::swap( isec_move, isec_copy );
  
        BOOST_CHECK_EQUAL( isec_copy.label(), empty_str );
        BOOST_CHECK_EQUAL( isec_copy.size(), 0 );
        BOOST_CHECK_EQUAL( isec_copy.empty(), true );
  
        BOOST_CHECK_EQUAL( isec_move.label(), label1 );
        BOOST_CHECK_EQUAL( isec_move.has_entry( name1 ), true );
        BOOST_CHECK_EQUAL( isec_move.get_entry( name1 ), value1 );
        BOOST_CHECK_EQUAL( isec_move.has_entry( name2 ), true );
        BOOST_CHECK_EQUAL( isec_move.get_entry( name2 ), svalue2 );
        BOOST_CHECK_EQUAL( isec_move.has_entry( name3 ), true );
        BOOST_CHECK_EQUAL( isec_move.get_entry( name3 ), svalue3 );
        BOOST_CHECK_EQUAL( isec_move.has_entry( name4 ), true );
        BOOST_CHECK_EQUAL( isec_move.get_entry( name4 ), svalue4 );
        BOOST_CHECK_EQUAL( isec_move.size(), 4 );
        BOOST_CHECK_EQUAL( isec_move.empty(), false );
     }
  }
  {
     // test add_entry
     // add_entry(string)  ABOVE
     // add_entry(unsigned long) as size_t ABOVE
     // add_entry(double) ABOVE
     {
        // add_entry(bool)
        core::input_section isec;
        isec.add_entry( name1, true );
        isec.add_entry( name2, false );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "true" );
        BOOST_CHECK_EQUAL( isec.get_entry( name2 ), "false" );
     }
     {
        // add_entry(int)
        core::input_section isec;
        int value { -170465 };
        isec.add_entry( name1, value );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "-170465" );
     }
     {
        // add_entry(long)
        core::input_section isec;
        int64_t value { -17046500000000000L };
        isec.add_entry( name1, value );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "-17046500000000000" );
     }
     {
        // add_entry(long long)
        core::input_section isec;
        long long value { -1704650000000000000LL };
        isec.add_entry( name1, value );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "-1704650000000000000" );
     }
     {
        // add_entry(uint)
        core::input_section isec;
        uint32_t value { 170465 };
        isec.add_entry( name1, value );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "170465" );
     }
     {
        // add_entry(unsigned long long)
        core::input_section isec;
        unsigned long long value { 170465000000000000ULL };
        isec.add_entry( name1, value );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "170465000000000000" );
     }
     {
        // add_entry(float)
        core::input_section isec;
        float value { 1.125E15F };
        isec.add_entry( name1, value );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "1.125e+15" );
     }
     {
        // add_entry(long double)
        core::input_section isec;
        long double value { 1.125E500L };
        isec.add_entry( name1, value );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "1.125e+500" );
     }
     {
        // template< class T > add_entry(T)
        core::input_section isec;
        geometry::coordinate value { 17.0, 4.0, 1965.0 };
        isec.add_entry( name1, value );
        BOOST_CHECK_EQUAL( isec.get_entry( name1 ), "17 4 1965" );
     }
  }
  {
     // deserialization
     core::input_section isec;
     boost::archive::text_iarchive ia( store );
     ia >> isec;
  
     BOOST_CHECK_EQUAL( isec.label(), label1 );
     BOOST_CHECK_EQUAL( isec.has_entry( name1 ), true );
     BOOST_CHECK_EQUAL( isec.get_entry( name1 ), value1 );
     BOOST_CHECK_EQUAL( isec.has_entry( name2 ), true );
     BOOST_CHECK_EQUAL( isec.get_entry( name2 ), svalue2 );
     BOOST_CHECK_EQUAL( isec.has_entry( name3 ), true );
     BOOST_CHECK_EQUAL( isec.get_entry( name3 ), svalue3 );
     BOOST_CHECK_EQUAL( isec.has_entry( name4 ), true );
     BOOST_CHECK_EQUAL( isec.get_entry( name4 ), svalue4 );
     BOOST_CHECK_EQUAL( isec.size(), 4 );
     BOOST_CHECK_EQUAL( isec.empty(), false );
  
     // Write test
     {
        std::stringstream output;
        isec.write( output );
        const std::string result { output.str() };
        boost::tokenizer< boost::escaped_list_separator< char > > tok(result, { "\\", " \n\t", "\"\'" } );
        std::vector< std::string > wordlist = { label1, name1, value1, name2, svalue2, name3, svalue3, name4, svalue4, "end" };
        BOOST_CHECK( std::equal( wordlist.begin(), wordlist.end(), tok.begin() ) );
        auto t_iter = tok.begin();
        for (auto w_iter = wordlist.begin(); w_iter != wordlist.end(); ++w_iter, ++t_iter)
        {
           BOOST_CHECK_EQUAL( *t_iter, *w_iter );
        }
     }
  }

}

//use the 'test_meta' class to test the non-virtual members of
//the abstract base class 'input_base_meta'
BOOST_AUTO_TEST_CASE( input_document_test )
{
  // CTOR TESTS
  // ----------
  const std::size_t version{ 21 };
  std::stringstream store;
  const std::string label1 { "label1" };
  const std::string label2 { "label2" };
  {
     // default
     core::input_document idoc( version );
     BOOST_CHECK_EQUAL( idoc.size(), 0 );
     BOOST_CHECK_EQUAL( idoc.empty(), true );
     BOOST_CHECK_EQUAL( idoc.version(), version );
  
     std::size_t idx;
     idx = idoc.add_section( label1 );
  
     BOOST_CHECK_EQUAL( idx, 0 );
     BOOST_CHECK_EQUAL( idoc[ idx ].label(), label1 );
     BOOST_CHECK_EQUAL( idoc.size(), 1 );
     BOOST_CHECK_EQUAL( idoc.empty(), false );
     BOOST_CHECK_EQUAL( idoc.version(), version );
  
     idx = idoc.add_section( label2 );
  
     BOOST_CHECK_EQUAL( idx, 1 );
     BOOST_CHECK_EQUAL( idoc[ idx ].label(), label2 );
     BOOST_CHECK_EQUAL( idoc.size(), 2 );
  
     boost::archive::text_oarchive oa( store );
     oa << idoc;
  
     {
        // copy ctor
        core::input_document idoc2( idoc );
        BOOST_CHECK_EQUAL( idoc2.version(), version );
        BOOST_CHECK_EQUAL( idoc2[ 0 ].label(), label1 );
        BOOST_CHECK_EQUAL( idoc2[ 1 ].label(), label2 );
        BOOST_CHECK_EQUAL( idoc2.size(), 2 );
        BOOST_CHECK_EQUAL( idoc2.empty(), false );
        // check idoc unchanged
        BOOST_CHECK_EQUAL( idoc.version(), version );
        BOOST_CHECK_EQUAL( idoc[ 0 ].label(), label1 );
        BOOST_CHECK_EQUAL( idoc[ 1 ].label(), label2 );
        BOOST_CHECK_EQUAL( idoc.size(), 2 );
        BOOST_CHECK_EQUAL( idoc.empty(), false );
     }
  
     {
        // assignement
        core::input_document idoc2( 1 );
        BOOST_CHECK_EQUAL( idoc2.size(), 0 );
        BOOST_CHECK_EQUAL( idoc2.empty(), true );
        BOOST_CHECK_EQUAL( idoc2.version(), 1 );
  
        idoc2 = idoc;
        BOOST_CHECK_EQUAL( idoc2.version(), version );
        BOOST_CHECK_EQUAL( idoc2[ 0 ].label(), label1 );
        BOOST_CHECK_EQUAL( idoc2[ 1 ].label(), label2 );
        BOOST_CHECK_EQUAL( idoc2.size(), 2 );
        BOOST_CHECK_EQUAL( idoc2.empty(), false );
        // check idoc unchanged
        BOOST_CHECK_EQUAL( idoc.version(), version );
        BOOST_CHECK_EQUAL( idoc[ 0 ].label(), label1 );
        BOOST_CHECK_EQUAL( idoc[ 1 ].label(), label2 );
        BOOST_CHECK_EQUAL( idoc.size(), 2 );
        BOOST_CHECK_EQUAL( idoc.empty(), false );
     }
  
     {
        // move
        core::input_document idoc2( idoc );
        core::input_document idoc3( std::move( idoc2 ) );
        BOOST_CHECK_EQUAL( idoc2.size(), 0 );
        BOOST_CHECK_EQUAL( idoc2.empty(), true );
        // value of idoc2.version is probably implementation dependent
  
        BOOST_CHECK_EQUAL( idoc3.version(), version );
        BOOST_CHECK_EQUAL( idoc3[ 0 ].label(), label1 );
        BOOST_CHECK_EQUAL( idoc3[ 1 ].label(), label2 );
        BOOST_CHECK_EQUAL( idoc3.size(), 2 );
        BOOST_CHECK_EQUAL( idoc3.empty(), false );
     }
  }
  {
     core::input_document idoc( 0 );
     boost::archive::text_iarchive ia( store );
     ia >> idoc;
  
     // check idoc unchanged
     BOOST_CHECK_EQUAL( idoc.version(), version );
     BOOST_CHECK_EQUAL( idoc[ 0 ].label(), label1 );
     BOOST_CHECK_EQUAL( idoc[ 1 ].label(), label2 );
     BOOST_CHECK_EQUAL( idoc.size(), 2 );
     BOOST_CHECK_EQUAL( idoc.empty(), false );
     // Write test
     {
        std::stringstream output;
        idoc.write( output );
        const std::string result { output.str() };
        boost::tokenizer< boost::escaped_list_separator< char > > tok(result, { "\\", " \n\t", "\"\'" } );
        std::vector< std::string > wordlist = { core::strngs::fsfver(), "21", "", label1, "end", "", label2, "end" };
        BOOST_CHECK( std::equal( wordlist.begin(), wordlist.end(), tok.begin() ) );
        auto t_iter = tok.begin();
        for (auto w_iter = wordlist.begin(); w_iter != wordlist.end(); ++w_iter, ++t_iter)
        {
           BOOST_CHECK_EQUAL( *t_iter, *w_iter );
        }
     }}

}

//use the 'test_meta' class to test the non-virtual members of
//the abstract base class 'input_base_meta'
BOOST_AUTO_TEST_CASE( input_definition_test )
{
  {
    // Static Lifetime method tests: noncopyable virtual pattern
    BOOST_CHECK( not std::is_default_constructible< core::input_definition >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< core::input_definition >::type{} );
    BOOST_CHECK( not std::is_move_constructible< core::input_definition >::type{} );
    BOOST_CHECK( not( std::is_assignable< core::input_definition, core::input_definition >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< core::input_definition >::type{} );
  }
  // ----------
  const std::string label1 { "label1" };
  const std::string description1 { "blab blah belch" };
  const std::string seclabel { "label2" };
  const std::string name1 { "name1" };
  const std::string desc1 { "desc1" };
  const std::string name2 { "name2" };
  const std::string desc2 { "desc2" };
  {
    // default
    core::input_definition idoc( label1, description1 );
    BOOST_CHECK_EQUAL( idoc.label(), label1 );
    BOOST_CHECK_EQUAL( idoc.description(), description1 );
    BOOST_CHECK_EQUAL( idoc.size(), 0 );
    BOOST_CHECK_EQUAL( idoc.empty(), true );
  
    BOOST_CHECK_NO_THROW( idoc.add_definition( { name1, "", "", "", desc1 } ) );
    BOOST_CHECK( idoc.has_definition( name1 ) );
    BOOST_CHECK_EQUAL( idoc.size(), 1 );
    BOOST_CHECK_EQUAL( idoc.empty(), false );
  
    BOOST_CHECK_NO_THROW( idoc.add_definition( { name2, "", "", "", desc2 } ) );
    BOOST_CHECK( idoc.has_definition( name2 ) );
    BOOST_CHECK_EQUAL( idoc.size(), 2 );
  
    core::input_help helper;
  
    idoc.publish_help( helper, seclabel );
  
    std::stringstream store;
    helper.write( store );
  
    const std::string msg( store.str() );
    //std::cout << msg << "\n";
    BOOST_CHECK_LT( msg.find( name1 + "\n          " + desc1 ), msg.size() );
    BOOST_CHECK_LT( msg.find( name2 + "\n          " + desc2 ), msg.size() );
  
    // Attempt to add definition with existing name.
    try
    {
      idoc.add_definition( { name1, "", "", "", desc1 } );
      BOOST_ERROR( "expected \"idoc.add_definition( name1, desc1 )\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Can not overwrite existing definition" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  
  

}

// Check lifetime methods of 'test_meta'.
// - check conforms to input_base_meta
// - check initialisation of attributes.
BOOST_AUTO_TEST_CASE( test_meta_lifetime_test )
{
  // CTOR TESTS
  core_test_suite::test_input_base_meta< test_meta >();
  // test test_meta attribute initialisation.
  {
    const std::string label( "name" );
    std::unique_ptr< test_meta > dobj( new test_meta( label, true, false ) );
    BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 0ul );
    BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 0ul );
    BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 0ul );
  }

}

// Test input file read_section processing
// * valid input
// ** empty section (only comments)
// ** single or multiple entries
// ** - non-unique entries
// 
BOOST_AUTO_TEST_CASE( test_meta_publish_help_test )
{
  {
    // 
    // Call publish_help, all other attributes unaffected.
    // 
    std::unique_ptr< test_meta > dobj( new test_meta( "label" ) );
    BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 0ul );
    {
      core::input_help dummy;
      dobj->publish_help( dummy );
      BOOST_CHECK_EQUAL( dobj->entry_map().size(), 0ul );
      BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 0ul );
      BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 0ul );
      BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 1ul );
    }
  }

}

// Test input file read_section processing
// * valid input
// ** empty section (only comments)
// ** single or multiple entries
// ** - non-unique entries
// * check other attributes unchanged
// 
BOOST_AUTO_TEST_CASE( test_meta_read_section_test )
{
  const std::string label( "name" );
  {
    //
    // Read section with no name/value pairs
    //
    std::unique_ptr< test_meta > dobj( new test_meta( label ) );
    {
      core::input_reader dummy;
      dummy.add_buffer( "test_filename", "name\n# comment\nend\n" );
      while( dummy.next() )
      {
        if( dummy.name() == "name" )
        {
          dobj->read_section( dummy );
          BOOST_CHECK_EQUAL( dobj->entry_map().size(), 0ul );
          BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 0ul );
          BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_reset_count(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 0ul );
        }
      }
    }
    {
      core::input_help dummy;
      dobj->publish_help( dummy );
      BOOST_CHECK_EQUAL( dobj->entry_map().size(), 0ul );
      BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 0ul );
      BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 1ul );
      BOOST_CHECK_EQUAL( dobj->get_reset_count(), 1ul );
      BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 1ul );
    }
  }
  {
    //
    // Read section with one name/value pair
    //
    std::unique_ptr< test_meta > dobj( new test_meta( label ) );
    {
      core::input_reader dummy;
      dummy.add_buffer( "test_filename", "name\nhello world\nend\n" );
      while( dummy.next() )
      {
        if( dummy.name() == "name" )
        {
          dobj->read_section( dummy );
          BOOST_CHECK_EQUAL( dobj->entry_map().size(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_reset_count(), 1ul );
        }
      }
    }
  }
  {
    //
    // Read section with three name/value pair
    //
    std::unique_ptr< test_meta > dobj( new test_meta( label ) );
    {
      core::input_reader dummy;
      dummy.add_buffer( "test_filename", "name\n# comment\na\nb c\nd e # hmm\nend\n" );
      while( dummy.next() )
      {
        if( dummy.name() == "name" )
        {
          dobj->read_section( dummy );
          BOOST_CHECK_EQUAL( dobj->entry_map().size(), 3ul );
          BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 3ul );
          BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_reset_count(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 0ul );
        }
      }
    }
    {
      core::input_help dummy;
      dobj->publish_help( dummy );
      BOOST_CHECK_EQUAL( dobj->entry_map().size(), 3ul );
      BOOST_CHECK_EQUAL( dobj->entry_map().count( "a" ), 1 );
      BOOST_CHECK_EQUAL( dobj->entry_map().count( "b" ), 1 );
      if( dobj->entry_map().count( "b" ) >= 1 )
      {
        BOOST_CHECK_EQUAL( dobj->entry_map().find( "b" )->second, "c" );
      }
      BOOST_CHECK_EQUAL( dobj->entry_map().count( "d" ), 1 );
      if( dobj->entry_map().count( "d" ) >= 1 )
      {
        BOOST_CHECK_EQUAL( dobj->entry_map().find( "d" )->second, "e" );
      }
      BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 3ul );
      BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 1ul );
      BOOST_CHECK_EQUAL( dobj->get_reset_count(), 1ul );
      BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 1ul );
    }
  }
  {
    //
    // Read section with three name/value pair, one duplicated
    //
    std::unique_ptr< test_meta > dobj( new test_meta( label ) );
    {
      core::input_reader dummy;
      dummy.add_buffer( "test_filename", "name\n# comment\na\nb c\nb e # hmm\nend\n" );
      // second value of "b" silently ignored.
      while( dummy.next() )
      {
        if( dummy.name() == "name" )
        {
          dobj->read_section( dummy );
          BOOST_CHECK_EQUAL( dobj->entry_map().size(), 2ul );
          BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 3ul );
          BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_reset_count(), 1ul );
          BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 0ul );
        }
      }
    }
    {
      core::input_help dummy;
      dobj->publish_help( dummy );
      BOOST_CHECK_EQUAL( dobj->entry_map().size(), 2ul );
      BOOST_CHECK_EQUAL( dobj->entry_map().count( "a" ), 1 );
      BOOST_CHECK_EQUAL( dobj->entry_map().count( "b" ), 1 );
      if( dobj->entry_map().count( "b" ) >= 1 )
      {
        BOOST_CHECK_EQUAL( dobj->entry_map().find( "b" )->second, "c" );
      }
      BOOST_CHECK_EQUAL( dobj->entry_map().count( "d" ), 0 );
      BOOST_CHECK_EQUAL( dobj->get_read_entry_count(), 3ul );
      BOOST_CHECK_EQUAL( dobj->get_read_end_count(), 1ul );
      BOOST_CHECK_EQUAL( dobj->get_reset_count(), 1ul );
      BOOST_CHECK_EQUAL( dobj->get_publish_help_count(), 1ul );
    }
  }
  
  
  

}

// Check for expected lifetime method behaviour:
// * non-virtual dtor
// * no defualt ctor
// * no copy behaviour (copy, move and assign)

BOOST_AUTO_TEST_CASE( input_delegater_lifetime_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< core::input_delegater >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< core::input_delegater >::type{} );
    BOOST_CHECK( not std::is_move_constructible< core::input_delegater >::type{} );
    BOOST_CHECK( not( std::is_assignable< core::input_delegater, core::input_delegater >::type{} ) );
    BOOST_CHECK( not( std::has_virtual_destructor< core::input_delegater >::type{} ) );
  }
  {
    // Construct delegater 
    core::input_delegater dlgater( 1 );
    BOOST_CHECK( dlgater.empty() );
    BOOST_CHECK_EQUAL( dlgater.size(), 0 );
    BOOST_CHECK_EQUAL( dlgater.max_version(), 1 );
    BOOST_CHECK_EQUAL( dlgater.read_version(), 0 );
  }
  

}

// Check adding input delegaters:
// * check added delegaters can be found.
// * - check failure to add delegaters with same name.

BOOST_AUTO_TEST_CASE( input_delegater_add_input_delegate_test )
{
  {
    const std::string label1( "label_one" );
    const std::string label2( "label_two" );
    boost::shared_ptr< test_meta > sobj1( new test_meta( label1, false, false ) );
    boost::shared_ptr< test_meta > sobj2( new test_meta( label2, true, false ) );
  
    // Construct delegater
    core::input_delegater dlgater( 1 );
  
    // add first delegater
    dlgater.add_input_delegate( sobj1 );
    BOOST_CHECK( not dlgater.empty() );
    BOOST_CHECK_EQUAL( dlgater.size(), 1 );
    BOOST_CHECK( dlgater.has_section( label1 ) );
    BOOST_CHECK( not dlgater.has_section( label2 ) );
  
    // add second delegater
    dlgater.add_input_delegate( sobj2 );
    BOOST_CHECK( not dlgater.empty() );
    BOOST_CHECK_EQUAL( dlgater.size(), 2 );
    BOOST_CHECK( dlgater.has_section( label1 ) );
    BOOST_CHECK( dlgater.has_section( label2 ) );
  
    // Process input buffer
    {
      core::input_reader dummy;
      dummy.add_buffer( "test_filename", "label_one\n# comment\na\nb c\nd e # hmm\nend\n" );
  
      dlgater.read_input( dummy );
  
      BOOST_CHECK_EQUAL( sobj1->get_read_entry_count(), 3ul );
      BOOST_CHECK_EQUAL( sobj1->get_read_end_count(), 1ul );
  
      BOOST_CHECK_EQUAL( sobj2->get_read_entry_count(), 0ul );
      BOOST_CHECK_EQUAL( sobj2->get_read_end_count(), 0ul );
    }
  }
  {
    // Construct delegater and add two meta objects with the same section name
    const std::string label1( "label_one" );
    const std::string label2( "label_two" );
    boost::shared_ptr< test_meta > sobj1( new test_meta( label1, false, false ) );
    boost::shared_ptr< test_meta > sobj2( new test_meta( label2, true, false ) );
    boost::shared_ptr< test_meta > sobj3( new test_meta( label2, true, false ) );
  
    core::input_delegater dlgater( 1 );
    dlgater.add_input_delegate( sobj1 );
    dlgater.add_input_delegate( sobj2 );
    try
    {
      dlgater.add_input_delegate( sobj3 );
      BOOST_ERROR( "expected \"dlgater.add_input_delegate( sobj3 )\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Can not add two meta objects for the same input section.\"label_two\"" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }

}

// Check handling of file version
// * check file version label in input file detected
// * - check on invalid file versions (version > delegater max)
// * - check on invalid file versions (version < 0)
BOOST_AUTO_TEST_CASE( input_delegater_input_file_versioning_test )
{
  {
    // Test valid file version processed
    boost::shared_ptr< core::input_base_meta > dobj( new test_meta( "test" ) );
  
    const std::string canon_input( "fileversion \t1\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
  
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj );
  
    BOOST_CHECK_NO_THROW( var1.read_input( dummy ) );
    BOOST_CHECK_EQUAL( var1.read_version(), 1ul );
  }
  {
    // Test invalid file version processed
    boost::shared_ptr< core::input_base_meta > dobj( new test_meta( "test" ) );
    const std::string canon_input( "fileversion 2\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj );
  
    try
    {
      var1.read_input( dummy );
      BOOST_ERROR( "expected \"var1.read_input( dummy )\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Input file version parameter \"fileversion\" with value (2) at input file \"" + boost::filesystem::current_path().native() + "/test_filename\" line 1.\n   >>fileversion 2<<\n** File version is too recent for this program **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    // Test invalid file version (non-number) processed
    boost::shared_ptr< core::input_base_meta > dobj( new test_meta( "test" ) );
    const std::string canon_input( "fileversion A2016\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj );
  
    try
    {
      var1.read_input( dummy );
      BOOST_ERROR( "expected \"var1.read_input( dummy )\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Input file version parameter \"fileversion\" with value (A2016) at input file \"" + boost::filesystem::current_path().native() + "/test_filename\" line 1.\n   >>fileversion A2016<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    // Test invalid file version (negative number) processed
    boost::shared_ptr< core::input_base_meta > dobj( new test_meta( "test" ) );
    const std::string canon_input( "fileversion -1\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj );
  
    try
    {
      var1.read_input( dummy );
      BOOST_ERROR( "expected \"var1.read_input( dummy )\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Input file version parameter \"fileversion\" with value (-1) at input file \"" + boost::filesystem::current_path().native() + "/test_filename\" line 1.\n   >>fileversion -1<<\n** Input value must be greater than or equal to zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }

}

// Check handling of input sections
// * check called meta classes match section in input
// * - check for unknown section
// * check for multiple section
// * - check for duplicate non-multiple section
BOOST_AUTO_TEST_CASE( input_delegater_input_sections_test )
{
  {
    // Test valid input with sections
    boost::shared_ptr< test_meta > dobj1( new test_meta( "test_one" ) );
    boost::shared_ptr< test_meta > dobj2( new test_meta( "test_two" ) );
    boost::shared_ptr< test_meta > dobj3( new test_meta( "test_three" ) );
  
    const std::string canon_input( "fileversion 1\ntest_one\nend\ntest_two\n\nend\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
  
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj1 );
    var1.add_input_delegate( dobj2 );
    var1.add_input_delegate( dobj3 );
  
    BOOST_CHECK_NO_THROW( var1.read_input( dummy ) );
  
    BOOST_CHECK_EQUAL( dobj1->get_read_end_count(), 1ul );
    BOOST_CHECK_EQUAL( dobj2->get_read_end_count(), 1ul );
    BOOST_CHECK_EQUAL( dobj3->get_read_end_count(), 0ul );
  }
  {
    // Test invalid input (unknown section)
    boost::shared_ptr< core::input_base_meta > dobj1( new test_meta( "test_one" ) );
    boost::shared_ptr< core::input_base_meta > dobj2( new test_meta( "test_two" ) );
    boost::shared_ptr< core::input_base_meta > dobj3( new test_meta( "test_three" ) );
  
    const std::string canon_input( "fileversion 1\ntest_one\nend\ntest4\n\nend\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
  
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj1 );
    var1.add_input_delegate( dobj2 );
    var1.add_input_delegate( dobj3 );
  
    try
    {
      var1.read_input( dummy );
      BOOST_ERROR( "expected \"var1.read_input( dummy )\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Unknown section label \"test4\" at input file \"" + boost::filesystem::current_path().native() + "/test_filename\" line 4.\n   >>test4<<\n** Section label must be one of : test_one test_three test_two. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    // Test invalid input (missing required section)
    boost::shared_ptr< core::input_base_meta > dobj1( new test_meta( "test_one" ) );
    boost::shared_ptr< core::input_base_meta > dobj2( new test_meta( "test_two" ) );
    boost::shared_ptr< core::input_base_meta > dobj3( new test_meta( "test_three", false, true ) );
  
    const std::string canon_input( "fileversion 1\ntest_one\nend\ntest_two\n\nend\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
  
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj1 );
    var1.add_input_delegate( dobj2 );
    var1.add_input_delegate( dobj3 );
  
    try
    {
      var1.read_input( dummy );
      BOOST_ERROR( "expected \"var1.read_input( dummy )\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Missing required sections detected at end of input file.\n** The following sections must be present in the input : test_three. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    // Test valid input (repeated multiple section)
    boost::shared_ptr< core::input_base_meta > dobj1( new test_meta( "test_one" ) );
    boost::shared_ptr< test_meta > dobj2( new test_meta( "test_two", true, false ) );
  
    const std::string canon_input( "fileversion 1\ntest_one\nend\ntest_two\n\nend\ntest_two\n\nend\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
  
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj1 );
    var1.add_input_delegate( dobj2 );
  
    BOOST_CHECK_NO_THROW( var1.read_input( dummy ) );
    BOOST_CHECK_EQUAL( dobj2->get_read_end_count(), 2ul );
  }
  {
    // Test invalid input (repeated non-multiple section)
    boost::shared_ptr< core::input_base_meta > dobj1( new test_meta( "test_one" ) );
    boost::shared_ptr< core::input_base_meta > dobj2( new test_meta( "test_two" ) );
  
    const std::string canon_input( "fileversion 1\ntest_one\nend\ntest_two\n\nend\ntest_two\n\nend\n" );
    core::input_reader dummy;
    dummy.add_buffer( "test_filename", canon_input );
  
    core::input_delegater var1( 1ul );
  
    var1.add_input_delegate( dobj1 );
    var1.add_input_delegate( dobj2 );
  
    try
    {
      var1.read_input( dummy );
      BOOST_ERROR( "expected \"var1.read_input( dummy )\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Once-only section \"test_two\" appears more than once, second appearance at input file \"" + boost::filesystem::current_path().native() + "/test_filename\" line 7.\n** Section \"test_two\" can only appear once. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  
  }

}

//use the 'test_meta' class to test the non-virtual members of
//the abstract base class 'input_base_meta'
BOOST_AUTO_TEST_CASE( indent_guard_lifetime_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< core::indent_guard >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< core::indent_guard >::type{} );
    BOOST_CHECK( not std::is_move_constructible< core::indent_guard >::type{} );
    BOOST_CHECK( not ( std::is_assignable< core::indent_guard, core::indent_guard >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< core::indent_guard >::type{} );
  }
  // CTOR TESTS
  // ----------
  {
    core::fixed_width_output_filter aset( 2, 1, 40 );
    BOOST_CHECK_EQUAL( aset.depth(), 1ul );
    {
      core::indent_guard grd( aset );
      BOOST_CHECK_EQUAL( aset.depth(), 2ul );
    }
    BOOST_CHECK_EQUAL( aset.depth(), 1ul );
  }

}

BOOST_AUTO_TEST_CASE( help_entry_lifetime_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< core::help_entry >::type{} );
    BOOST_CHECK( std::is_copy_constructible< core::help_entry >::type{} );
    BOOST_CHECK( std::is_move_constructible< core::help_entry >::type{} );
    BOOST_CHECK( ( std::is_assignable< core::help_entry, core::help_entry >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< core::help_entry >::type{} );
  }
  // CTOR TESTS
  // ----------
  const std::string nul_string;
  const std::string title( "title" );
  const std::string type_desc( "integral number" );
  const std::string range( "> 0" );
  const std::string defval( "10" );
  const std::string desc( "blah blah blah" );
  std::stringstream store;
  {
    core::help_entry hentry;
    BOOST_CHECK_EQUAL( hentry.default_value(), nul_string );
    BOOST_CHECK_EQUAL( hentry.description(), nul_string );
    BOOST_CHECK_EQUAL( hentry.range(), nul_string );
    BOOST_CHECK_EQUAL( hentry.title(), nul_string );
    BOOST_CHECK_EQUAL( hentry.type_desc(), nul_string );
  }
  {
    core::help_entry hentry( title, type_desc, range, defval, desc );
    BOOST_CHECK_EQUAL( hentry.default_value(), defval );
    BOOST_CHECK_EQUAL( hentry.description(), desc );
    BOOST_CHECK_EQUAL( hentry.range(), range );
    BOOST_CHECK_EQUAL( hentry.title(), title );
    BOOST_CHECK_EQUAL( hentry.type_desc(), type_desc );
    {
      core::help_entry hentry1( hentry );
      BOOST_CHECK_EQUAL( hentry1.default_value(), defval );
      BOOST_CHECK_EQUAL( hentry1.description(), desc );
      BOOST_CHECK_EQUAL( hentry1.range(), range );
      BOOST_CHECK_EQUAL( hentry1.title(), title );
      BOOST_CHECK_EQUAL( hentry1.type_desc(), type_desc );
    }
    {
      core::help_entry hentry1;
      BOOST_CHECK_EQUAL( hentry1.default_value(), nul_string );
      BOOST_CHECK_EQUAL( hentry1.description(), nul_string );
      BOOST_CHECK_EQUAL( hentry1.range(), nul_string );
      BOOST_CHECK_EQUAL( hentry1.title(), nul_string );
      BOOST_CHECK_EQUAL( hentry1.type_desc(), nul_string );
      hentry1 = hentry;
      BOOST_CHECK_EQUAL( hentry1.default_value(), defval );
      BOOST_CHECK_EQUAL( hentry1.description(), desc );
      BOOST_CHECK_EQUAL( hentry1.range(), range );
      BOOST_CHECK_EQUAL( hentry1.title(), title );
      BOOST_CHECK_EQUAL( hentry1.type_desc(), type_desc );
    }
    {
      core::help_entry hentry2( hentry );
      core::help_entry hentry1( std::move( hentry2 ) );
      BOOST_CHECK_EQUAL( hentry1.default_value(), defval );
      BOOST_CHECK_EQUAL( hentry1.description(), desc );
      BOOST_CHECK_EQUAL( hentry1.range(), range );
      BOOST_CHECK_EQUAL( hentry1.title(), title );
      BOOST_CHECK_EQUAL( hentry1.type_desc(), type_desc );
    }
    boost::archive::text_oarchive oa( store );
    oa << hentry;
  }
  {
    core::help_entry hentry;
    BOOST_CHECK_EQUAL( hentry.default_value(), nul_string );
    BOOST_CHECK_EQUAL( hentry.description(), nul_string );
    BOOST_CHECK_EQUAL( hentry.range(), nul_string );
    BOOST_CHECK_EQUAL( hentry.title(), nul_string );
    BOOST_CHECK_EQUAL( hentry.type_desc(), nul_string );
    boost::archive::text_iarchive ia( store );
    ia >> hentry;
    BOOST_CHECK_EQUAL( hentry.default_value(), defval );
    BOOST_CHECK_EQUAL( hentry.description(), desc );
    BOOST_CHECK_EQUAL( hentry.range(), range );
    BOOST_CHECK_EQUAL( hentry.title(), title );
    BOOST_CHECK_EQUAL( hentry.type_desc(), type_desc );
   }

}

BOOST_AUTO_TEST_CASE( help_entry_method_test )
{
  const std::string nul_string;
  const std::string title( "title" );
  const std::string type_desc( "integral number" );
  const std::string range( "> 0" );
  const std::string defval( "10" );
  const std::string desc( "blah blah blah" );
  {
    core::help_entry hentry;
    BOOST_CHECK_EQUAL( hentry.default_value(), nul_string );
    hentry.default_value( defval );
    BOOST_CHECK_EQUAL( hentry.default_value(), defval );
    BOOST_CHECK_EQUAL( hentry.description(), nul_string );
    hentry.description( desc );
    BOOST_CHECK_EQUAL( hentry.description(), desc );
    BOOST_CHECK_EQUAL( hentry.range(), nul_string );
    hentry.range( range );
    BOOST_CHECK_EQUAL( hentry.range(), range );
    BOOST_CHECK_EQUAL( hentry.title(), nul_string );
    hentry.title( title );
    BOOST_CHECK_EQUAL( hentry.title(), title );
    BOOST_CHECK_EQUAL( hentry.type_desc(), nul_string );
    hentry.type_desc( type_desc );
    BOOST_CHECK_EQUAL( hentry.type_desc(), type_desc );
  
    {
      const std::string output( "  title\n    [integral number: range(> 0), \n    default{10}]\n    blah blah blah\n" );
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      hentry.write( filt, os );
      //std::cout << "\n--\n" << os.str() << "\n--\n";
      BOOST_CHECK_EQUAL( os.str(), output );
    }
  }

}

BOOST_AUTO_TEST_CASE( help_subtype_lifetime_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< core::help_subtype >::type{} );
    BOOST_CHECK( std::is_copy_constructible< core::help_subtype >::type{} );
    BOOST_CHECK( std::is_move_constructible< core::help_subtype >::type{} );
    BOOST_CHECK( ( std::is_assignable< core::help_subtype, core::help_subtype >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< core::help_subtype >::type{} );
  }
  // CTOR TESTS
  // ----------
  const std::string nul_string;
  const std::string title( "title" );
  const std::string desc( "blah blah blah" );
  std::stringstream store;
  {
    core::help_subtype hsubtype;
    BOOST_CHECK_EQUAL( hsubtype.description(), nul_string );
    BOOST_CHECK_EQUAL( hsubtype.title(), nul_string );
    BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
    BOOST_CHECK( hsubtype.empty() );
    BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
  }
  {
    core::help_subtype hsubtype( title, desc );
    BOOST_CHECK_EQUAL( hsubtype.description(), desc );
    BOOST_CHECK_EQUAL( hsubtype.title(), title );
    BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
    BOOST_CHECK( hsubtype.empty() );
    BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
    {
      core::help_subtype hsubtype1( hsubtype );
      BOOST_CHECK_EQUAL( hsubtype1.description(), desc );
      BOOST_CHECK_EQUAL( hsubtype1.title(), title );
      BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
      BOOST_CHECK( hsubtype.empty() );
      BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
    }
    {
      core::help_subtype hsubtype1;
      BOOST_CHECK_EQUAL( hsubtype.description(), desc );
      BOOST_CHECK_EQUAL( hsubtype.title(), title );
      BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
      BOOST_CHECK( hsubtype.empty() );
      BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
      hsubtype1 = hsubtype;
      BOOST_CHECK_EQUAL( hsubtype1.description(), desc );
      BOOST_CHECK_EQUAL( hsubtype1.title(), title );
      BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
      BOOST_CHECK( hsubtype.empty() );
      BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
    }
    {
      core::help_subtype hsubtype2( hsubtype );
      core::help_subtype hsubtype1( std::move( hsubtype2 ) );
      BOOST_CHECK_EQUAL( hsubtype1.description(), desc );
      BOOST_CHECK_EQUAL( hsubtype1.title(), title );
      BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
      BOOST_CHECK( hsubtype.empty() );
      BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
    }
    boost::archive::text_oarchive oa( store );
    oa << hsubtype;
  }
  {
    core::help_subtype hsubtype;
    BOOST_CHECK_EQUAL( hsubtype.description(), nul_string );
    BOOST_CHECK_EQUAL( hsubtype.title(), nul_string );
    BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
    BOOST_CHECK( hsubtype.empty() );
    BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
    boost::archive::text_iarchive ia( store );
    ia >> hsubtype;
    BOOST_CHECK_EQUAL( hsubtype.description(), desc );
    BOOST_CHECK_EQUAL( hsubtype.title(), title );
    BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
    BOOST_CHECK( hsubtype.empty() );
    BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
   }

}

BOOST_AUTO_TEST_CASE( help_subtype_method_test )
{
  const std::string nul_string;
  const std::string title( "title" );
  const std::string type_desc( "integral number" );
  const std::string range( "> 0" );
  const std::string defval( "10" );
  const std::string desc( "blah blah blah" );
  const std::string output( "  title\n    blah blah blah\n    Options:\n    title\n      [integral number: range(> 0), \n      default{10}]\n      blah blah blah\n" );
  std::stringstream store;
  {
    core::help_subtype hsubtype;
    BOOST_CHECK_EQUAL( hsubtype.description(), nul_string );
    hsubtype.description( desc );
    BOOST_CHECK_EQUAL( hsubtype.description(), desc );
    BOOST_CHECK_EQUAL( hsubtype.title(), nul_string );
    hsubtype.title( title );
    BOOST_CHECK_EQUAL( hsubtype.title(), title );
  
    hsubtype.add_entry( { title, type_desc, range, defval, desc } );
    BOOST_CHECK_EQUAL( hsubtype.size(), 1ul );
    BOOST_CHECK( not hsubtype.empty() );
    BOOST_CHECK( hsubtype.has_entry( title ) );
    BOOST_CHECK_EQUAL( hsubtype.get_entry( title ).default_value(), defval );
    {
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      hsubtype.write( filt, os );
      //std::cout << "\n--\n" << os.str() << "\n--\n";
      BOOST_CHECK_EQUAL( os.str(), output );
    }
    boost::archive::text_oarchive oa( store );
    oa << hsubtype;
  }
  {
    const std::string title2( "title2" );
    const std::string type_desc2( "float number" );
    const std::string range2( "" );
    const std::string defval2( "10.0" );
    const std::string desc2( "blam blam blam" );
    const std::string output1( "  title\n    blah blah blah\n    Option:\n    title\n      [integral number: range(> 0), \n      default{10}]\n      blah blah blah\n" );
    const std::string output2( "  title\n    blah blah blah\n    Option:\n    title2\n      [float number: default{10.0}]\n      blam blam blam\n" );
    const std::string output3( "  title\n    blah blah blah\n    Options:\n    title\n      [integral number: range(> 0), \n      default{10}]\n      blah blah blah\n    title2\n      [float number: default{10.0}]\n      blam blam blam\n" );
  
    core::help_subtype hsubtype;
    BOOST_CHECK_EQUAL( hsubtype.description(), nul_string );
    BOOST_CHECK_EQUAL( hsubtype.title(), nul_string );
    BOOST_CHECK_EQUAL( hsubtype.size(), 0ul );
    BOOST_CHECK( hsubtype.empty() );
    BOOST_CHECK( hsubtype.begin() == hsubtype.end() );
    boost::archive::text_iarchive ia( store );
    ia >> hsubtype;
    BOOST_CHECK_EQUAL( hsubtype.description(), desc );
    BOOST_CHECK_EQUAL( hsubtype.title(), title );
    BOOST_CHECK_EQUAL( hsubtype.size(), 1ul );
    BOOST_CHECK( not hsubtype.empty() );
    BOOST_CHECK( hsubtype.has_entry( title ) );
    {
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      hsubtype.write( filt, os );
      BOOST_CHECK_EQUAL( os.str(), output );
    }
    hsubtype.add_entry( { title2, type_desc2, range2, defval2, desc2 } );
    BOOST_CHECK( hsubtype.has_entry( title2 ) );
    {
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      hsubtype.write( filt, title, os );
      BOOST_CHECK_EQUAL( os.str(), output1 );
    }
    {
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      hsubtype.write( filt, title2, os );
      //std::cout << os.rdbuf() << "\n";
      BOOST_CHECK_EQUAL( os.str(), output2 );
    }
    {
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      hsubtype.write( filt, os );
      //std::cout << os.rdbuf() << "\n";
      BOOST_CHECK_EQUAL( os.str(), output3 );
    }
    {
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      try
      {
        hsubtype.write( filt, "", os );
        BOOST_ERROR( "hsubtype.write( filt, \"\", os ) did not throw expected error." );
        //std::cout << os.rdbuf() << "\n";
        BOOST_CHECK_EQUAL( os.str(), output3 );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "REASON: \"Input parameter name should not be empty when using this 'write' method.\"" ) < msg.size() );
      }
    }
  }

}

BOOST_AUTO_TEST_CASE( help_section_lifetime_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< core::help_section >::type{} );
    BOOST_CHECK( std::is_copy_constructible< core::help_section >::type{} );
    BOOST_CHECK( std::is_move_constructible< core::help_section >::type{} );
    BOOST_CHECK( ( std::is_assignable< core::help_section, core::help_section >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< core::help_section >::type{} );
  }
  // CTOR TESTS
  // ----------
  const std::string nul_string;
  const std::string title( "title" );
  const std::string desc( "blah blah blah" );
  std::stringstream store;
  {
    core::help_section hsection;
    BOOST_CHECK_EQUAL( hsection.description(), nul_string );
    BOOST_CHECK_EQUAL( hsection.title(), nul_string );
    BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
    BOOST_CHECK( hsection.entry_empty() );
    BOOST_CHECK( hsection.entry_begin() == hsection.entry_end() );
    BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
    BOOST_CHECK( hsection.subtype_empty() );
    BOOST_CHECK( hsection.subtype_begin() == hsection.subtype_end() );
  }
  {
    core::help_section hsection( title, desc );
    BOOST_CHECK_EQUAL( hsection.description(), desc );
    BOOST_CHECK_EQUAL( hsection.title(), title );
    BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
    BOOST_CHECK( hsection.entry_empty() );
    BOOST_CHECK( hsection.entry_begin() == hsection.entry_end() );
    BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
    BOOST_CHECK( hsection.subtype_empty() );
    BOOST_CHECK( hsection.subtype_begin() == hsection.subtype_end() );
    {
      core::help_section hsection1( hsection );
      BOOST_CHECK_EQUAL( hsection1.description(), desc );
      BOOST_CHECK_EQUAL( hsection1.title(), title );
      BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
      BOOST_CHECK( hsection.entry_empty() );
      BOOST_CHECK( hsection.entry_begin() == hsection.entry_end() );
      BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
      BOOST_CHECK( hsection.subtype_empty() );
      BOOST_CHECK( hsection.subtype_begin() == hsection.subtype_end() );
    }
    {
      core::help_section hsection1;
      BOOST_CHECK_EQUAL( hsection.description(), desc );
      BOOST_CHECK_EQUAL( hsection.title(), title );
      BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
      BOOST_CHECK( hsection.entry_empty() );
      BOOST_CHECK( hsection.entry_begin() == hsection.entry_end() );
      BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
      BOOST_CHECK( hsection.subtype_empty() );
      BOOST_CHECK( hsection.subtype_begin() == hsection.subtype_end() );
      hsection1 = hsection;
      BOOST_CHECK_EQUAL( hsection1.description(), desc );
      BOOST_CHECK_EQUAL( hsection1.title(), title );
      BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
      BOOST_CHECK( hsection.entry_empty() );
      BOOST_CHECK( hsection.entry_begin() == hsection.entry_end() );
      BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
      BOOST_CHECK( hsection.subtype_empty() );
      BOOST_CHECK( hsection.subtype_begin() == hsection.subtype_end() );
    }
    {
      core::help_section hsection2( hsection );
      core::help_section hsection1( std::move( hsection2 ) );
      BOOST_CHECK_EQUAL( hsection1.description(), desc );
      BOOST_CHECK_EQUAL( hsection1.title(), title );
      BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
      BOOST_CHECK( hsection.entry_empty() );
      BOOST_CHECK( hsection.entry_begin() == hsection.entry_end() );
      BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
      BOOST_CHECK( hsection.subtype_empty() );
      BOOST_CHECK( hsection.subtype_begin() == hsection.subtype_end() );
    }
    boost::archive::text_oarchive oa( store );
    oa << hsection;
  }
  {
    core::help_section hsection;
    BOOST_CHECK_EQUAL( hsection.description(), nul_string );
    BOOST_CHECK_EQUAL( hsection.title(), nul_string );
    BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
    BOOST_CHECK( hsection.entry_empty() );
    BOOST_CHECK( hsection.entry_begin() == hsection.entry_end() );
    BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
    BOOST_CHECK( hsection.subtype_empty() );
    BOOST_CHECK( hsection.subtype_begin() == hsection.subtype_end() );
    boost::archive::text_iarchive ia( store );
    ia >> hsection;
    BOOST_CHECK_EQUAL( hsection.description(), desc );
    BOOST_CHECK_EQUAL( hsection.title(), title );
    BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
    BOOST_CHECK( hsection.entry_empty() );
    BOOST_CHECK( hsection.entry_begin() == hsection.entry_end() );
    BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
    BOOST_CHECK( hsection.subtype_empty() );
    BOOST_CHECK( hsection.subtype_begin() == hsection.subtype_end() );
   }

}

BOOST_AUTO_TEST_CASE( help_section_method_test )
{
  const std::string nul_string;
  const std::string title( "title" );
  const std::string type_desc( "integral number" );
  const std::string range( "> 0" );
  const std::string defval( "10" );
  const std::string desc( "blah blah blah" );
  const std::string title1( "title1" );
  const std::string title2( "title2" );
  std::stringstream store;
  {
    core::help_section hsection;
    BOOST_CHECK_EQUAL( hsection.description(), nul_string );
    hsection.description( desc );
    BOOST_CHECK_EQUAL( hsection.description(), desc );
    BOOST_CHECK_EQUAL( hsection.title(), nul_string );
    hsection.title( title );
    BOOST_CHECK_EQUAL( hsection.title(), title );
  
    hsection.add_entry( { title, type_desc, range, defval, desc } );
    BOOST_CHECK_EQUAL( hsection.entry_size(), 1ul );
    BOOST_CHECK( not hsection.entry_empty() );
    BOOST_CHECK( hsection.has_entry( title ) );
    BOOST_CHECK_EQUAL( hsection.get_entry( title ).default_value(), defval );
    {
      core::help_subtype hsubtype( title1, desc );
      hsubtype.add_entry( { title2, type_desc, range, defval, desc } );
      BOOST_CHECK_EQUAL( hsubtype.size(), 1ul );
      BOOST_CHECK( not hsubtype.empty() );
      BOOST_CHECK( hsubtype.has_entry( title2 ) );
      BOOST_CHECK_EQUAL( hsubtype.get_entry( title2 ).default_value(), defval );
      hsection.add_subtype( hsubtype );
    }
    BOOST_CHECK_EQUAL( hsection.subtype_size(), 1ul );
    BOOST_CHECK( not hsection.subtype_empty() );
    BOOST_CHECK( hsection.has_subtype( title1 ) );
    BOOST_CHECK_EQUAL( hsection.get_subtype( title1 ).description(), desc );
    boost::archive::text_oarchive oa( store );
    oa << hsection;
  }
  {
    core::help_section hsection;
    BOOST_CHECK_EQUAL( hsection.description(), nul_string );
    BOOST_CHECK_EQUAL( hsection.title(), nul_string );
    BOOST_CHECK_EQUAL( hsection.entry_size(), 0ul );
    BOOST_CHECK_EQUAL( hsection.subtype_size(), 0ul );
    boost::archive::text_iarchive ia( store );
    ia >> hsection;
    BOOST_CHECK_EQUAL( hsection.description(), desc );
    BOOST_CHECK_EQUAL( hsection.title(), title );
    BOOST_CHECK_EQUAL( hsection.entry_size(), 1ul );
    BOOST_CHECK_EQUAL( hsection.subtype_size(), 1ul );
    {
      const std::string output( "  title\n    blah blah blah\n    Options:\n    title\n      [integral number: range(> 0), \n      default{10}]\n      blah blah blah\n    type\n      [one of: title1]\n      This section represents a class of\n      types in the the simulation. Each \n      type can have individual options. \n      These options may only be valid in\n      combination with the specific type\n      .\n\n      Description of available types:\n      title1\n        blah blah blah\n        Options:\n        title2\n          [integral number: range(> 0), \n          default{10}]\n          blah blah blah\n" );
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      hsection.write( filt, os );
      //std::cout << "--\n" << os.str() << "--\n";
      BOOST_CHECK_EQUAL( os.str(), output );
    }
    {
      const std::string output( "  title\n    blah blah blah\n    type:\n    title1\n      blah blah blah\n      Option:\n      title2\n        [integral number: range(> 0), \n        default{10}]\n        blah blah blah\n" );
  
      core::fixed_width_output_filter filt( 2, 1, 40 );
      std::stringstream os;
      hsection.write( filt, title2, os );
      //std::cout << "--\n" << os.str() << "--\n";
      BOOST_CHECK_EQUAL( os.str(), output );
    }
  }

}

//use the 'test_meta' class to test the non-virtual members of
//the abstract base class 'input_base_meta'
BOOST_AUTO_TEST_CASE( output_filter_lifetime_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< core::fixed_width_output_filter >::type{} );
    BOOST_CHECK( std::is_copy_constructible< core::fixed_width_output_filter >::type{} );
    BOOST_CHECK( std::is_move_constructible< core::fixed_width_output_filter >::type{} );
    BOOST_CHECK( ( std::is_assignable< core::fixed_width_output_filter, core::fixed_width_output_filter >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< core::fixed_width_output_filter >::type{} );
  }
  // CTOR TESTS
  // ----------
  {
    core::fixed_width_output_filter aset( 0, 0, 0 );
    BOOST_CHECK_EQUAL( aset.text_width(), 0ul );
    BOOST_CHECK_EQUAL( aset.depth(), 0ul );
    BOOST_CHECK_EQUAL( aset.indent_size(), 0ul );
  }
  {
    core::fixed_width_output_filter aset( 2, 1, 40 );
    BOOST_CHECK_EQUAL( aset.text_width(), 40ul );
    BOOST_CHECK_EQUAL( aset.depth(), 1ul );
    BOOST_CHECK_EQUAL( aset.indent_size(), 2ul );
    {
      core::fixed_width_output_filter aset1( aset );
      BOOST_CHECK_EQUAL( aset1.text_width(), 40ul );
      BOOST_CHECK_EQUAL( aset1.depth(), 1ul );
      BOOST_CHECK_EQUAL( aset1.indent_size(), 2ul );
    }
    {
      core::fixed_width_output_filter aset2( aset );
      core::fixed_width_output_filter aset1( std::move( aset2 ) );
      BOOST_CHECK_EQUAL( aset1.text_width(), 40ul );
      BOOST_CHECK_EQUAL( aset1.depth(), 1ul );
      BOOST_CHECK_EQUAL( aset1.indent_size(), 2ul );
    }
  }

}

//  Use filter to format text and compare to expected results.
//
// TODO: text with embedded tabs and newlines
BOOST_AUTO_TEST_CASE( output_filter_method_test )
{
  // USAGE TESTS
  // ----------
  {
    core::fixed_width_output_filter aset( 2, 1, 10 );
    namespace io = boost::iostreams;
    {
      std::stringstream result;
      const std::string input( "This is some text that I want to format." );
      const std::string output( "  This is \n  some \n  text \n  that I \n  want to \n  format." );
      {
        io::filtering_ostream out;
        out.push( aset );
        out.push( result );
        io::write( out, input.data(), input.size() );
      }
      const std::string resultstr( result.str() );
      //std::cout << "\n--\n" << resultstr << "\n--\n";
      //std::cout << "\n**\n" << output << "\n**\n";
      BOOST_CHECK_EQUAL( resultstr, output );
      if( resultstr != output )
      {
        const std::size_t minsz = std::min( resultstr.size(), output.size() );
        for( std::size_t ii = 0; ii != minsz; ++ii )
        {
          BOOST_CHECK_EQUAL( resultstr[ii], output[ii] );
        }
      }
    }
    {
      std::stringstream result;
      const std::string input( "Now for some exceedingly elongated words." );
      const std::string output( "  Now for \n  some \n  exceedin\n  gly \n  elongate\n  d words." );
      {
        io::filtering_ostream out;
        out.push( aset );
        out.push( result );
        io::write( out, input.data(), input.size() );
      }
      const std::string resultstr( result.str() );
      //std::cout << "\n--\n" << resultstr << "\n--\n";
      //std::cout << "\n**\n" << output << "\n**\n";
      BOOST_CHECK_EQUAL( resultstr, output );
      if( resultstr != output )
      {
        const std::size_t minsz = std::min( resultstr.size(), output.size() );
        for( std::size_t ii = 0; ii != minsz; ++ii )
        {
          BOOST_CHECK_EQUAL( resultstr[ii], output[ii] );
        }
      }
    }
    {
      std::stringstream result;
      const std::string input( "Now to change the indenting." );
      const std::string output( "    Now to\n    change\n    the \n    indent\n    ing." );
      {
        aset.increment_depth();
        io::filtering_ostream out;
        out.push( aset );
        out.push( result );
        io::write( out, input.data(), input.size() );
      }
      const std::string resultstr( result.str() );
      //std::cout << "\n--\n" << resultstr << "\n--\n";
      //std::cout << "\n**\n" << output << "\n**\n";
      BOOST_CHECK_EQUAL( resultstr, output );
      if( resultstr != output )
      {
        const std::size_t minsz = std::min( resultstr.size(), output.size() );
        for( std::size_t ii = 0; ii != minsz; ++ii )
        {
          BOOST_CHECK_EQUAL( resultstr[ii], output[ii] );
        }
      }
    }
    {
      std::stringstream result;
      const std::string input( " a b c  d e f g h i j k l m n o p" );
      const std::string output( "  a b c  d\n  e f g h \n  i j k l \n  m n o p" );
      {
        aset.decrement_depth();
        io::filtering_ostream out;
        out.push( aset );
        out.push( result );
        io::write( out, input.data(), input.size() );
      }
      const std::string resultstr( result.str() );
      //std::cout << "\n--\n" << resultstr << "\n--\n";
      //std::cout << "\n**\n" << output << "\n**\n";
      BOOST_CHECK_EQUAL( resultstr, output );
      if( resultstr != output )
      {
        const std::size_t minsz = std::min( resultstr.size(), output.size() );
        for( std::size_t ii = 0; ii != minsz; ++ii )
        {
          BOOST_CHECK_EQUAL( resultstr[ii], output[ii] );
        }
      }
    }
    {
      std::stringstream result;
      const std::string input( " 1234567890 1234567890" );
      const std::string output( "  12345678\n  90 \n  12345678\n  90" );
      {
        io::filtering_ostream out;
        out.push( aset );
        out.push( result );
        io::write( out, input.data(), input.size() );
      }
      const std::string resultstr( result.str() );
      //std::cout << "\n--\n" << resultstr << "\n--\n";
      //std::cout << "\n**\n" << output << "\n**\n";
      BOOST_CHECK_EQUAL( resultstr, output );
      if( resultstr != output )
      {
        const std::size_t minsz = std::min( resultstr.size(), output.size() );
        for( std::size_t ii = 0; ii != minsz; ++ii )
        {
          BOOST_CHECK_EQUAL( resultstr[ii], output[ii] );
        }
      }
    }
    {
      std::stringstream result;
      const std::string input( " 1234567890 1234567890" );
      const std::string output( "1234567890\n1234567890" );
      {
        aset.decrement_depth();
        io::filtering_ostream out;
        out.push( aset );
        out.push( result );
        io::write( out, input.data(), input.size() );
      }
      const std::string resultstr( result.str() );
      //std::cout << "\n--\n" << resultstr << "\n--\n";
      //std::cout << "\n**\n" << output << "\n**\n";
      BOOST_CHECK_EQUAL( resultstr, output );
      if( resultstr != output )
      {
        const std::size_t minsz = std::min( resultstr.size(), output.size() );
        for( std::size_t ii = 0; ii != minsz; ++ii )
        {
          BOOST_CHECK_EQUAL( resultstr[ii], output[ii] );
        }
      }
    }
    {
      std::stringstream result;
      const std::string input( " -12.34567890 1234.567890" );
      const std::string output( "-12.345678\n90 \n1234.56789\n0" );
      {
        aset.decrement_depth();
        io::filtering_ostream out;
        out.push( aset );
        out.push( result );
        io::write( out, input.data(), input.size() );
      }
      const std::string resultstr( result.str() );
      //std::cout << "\n--\n" << resultstr << "\n--\n";
      //std::cout << "\n**\n" << output << "\n**\n";
      BOOST_CHECK_EQUAL( resultstr, output );
      if( resultstr != output )
      {
        const std::size_t minsz = std::min( resultstr.size(), output.size() );
        for( std::size_t ii = 0; ii != minsz; ++ii )
        {
          BOOST_CHECK_EQUAL( resultstr[ii], output[ii] );
        }
      }
    }
  }

}

