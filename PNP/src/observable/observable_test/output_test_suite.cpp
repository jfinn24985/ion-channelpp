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

#define BOOST_TEST_MODULE output_test
#include <boost/test/unit_test.hpp>

#include "observable/observable_test/output_test_suite.hpp"
#include "observable/archive_file.hpp"
#include "observable/field_format.hpp"
#include "observable/gdbm_sink.hpp"
#include "observable/memory_sink.hpp"
#include "observable/output_buffer_datum.hpp"
#include "observable/output_file.hpp"
#include "observable/output_row_datum.hpp"
#include "observable/output_series.hpp"
#include "observable/output_series_datum.hpp"
#include "observable/output_text.hpp"
#include "observable/widom_row.hpp"
#include "observable/output_rows.hpp"
#include "observable/base_sink.hpp"

// A generic buffer for transferring information.
std::string output_test_suite::output_test_datum::buffer;

void output_test_suite::output_test_datum::visit(observable::output_series & sink)
{
  buffer.append( "SERIES " );

}

void output_test_suite::output_test_datum::visit(observable::output_text & sink)
{
  buffer.append( "TEXT " );

}

void output_test_suite::output_test_datum::visit(observable::output_rows & sink)
{
  buffer.append( "ROWS " );

}

// Output a space separated list of the field labels.
void output_test_suite::output_test_row::labels(std::ostream & os) const
{
  os << "INDEX MEAN";
}

// Output a space separated list of the field units.
void output_test_suite::output_test_row::units(std::ostream & os) const
{
  os << "ORDINAL RATE";
}

// Output a space separated list of the field entries.
void output_test_suite::output_test_row::row(std::ostream & os) const
{
  os << this->buffer;
}

// Merge the given row into this row.
void output_test_suite::output_test_row::merge(std::unique_ptr< observable::output_row > source)
{
  output_test_row const* source_ptr = dynamic_cast< output_test_row const* >( source.get() );
  UTILITY_REQUIRE( source_ptr != nullptr, "Data is not of required subtype." ); 
  this->buffer.append( source_ptr->buffer );

}

void output_test_suite::base_output_field_test(std::unique_ptr< observable::output_field > fld)
{
  std::stringstream store;
  const std::string lbl = fld->label();
  const std::string unt = fld->unit();
  {
    const std::string lbl2{ "label2" };
    BOOST_CHECK_NO_THROW( fld->set_label( lbl2 ) );
    BOOST_CHECK_EQUAL( fld->label(), lbl2 );
    fld->set_label( lbl );
  }
  {
    const std::string unt2{ "unit2" };
    BOOST_CHECK_NO_THROW( fld->set_unit( unt2 ) );
    BOOST_CHECK_EQUAL( fld->unit(), unt2 );
    fld->set_unit( unt );
  }
  std::string output;
  utility::estimate_array arr( 2 );
  arr.append( { 0.1, 0.2 } );
  arr.append( { 0.11, 0.22 } );
  arr.append( { 0.12, 0.21 } );
  {
    std::stringstream os;
    fld->write( os, arr, 0, 0 );
    output = os.str();
   
    boost::archive::text_oarchive oa( store );
    oa << fld; 
  }
  {
    std::unique_ptr< observable::output_field > fld2;
    boost::archive::text_iarchive ia( store );
    ia >> fld2; 
    BOOST_CHECK_EQUAL( fld2->label(), lbl );
    BOOST_CHECK_EQUAL( fld2->unit(), unt );
    std::stringstream os;
    fld2->write( os, arr, 0, 0 );
    BOOST_CHECK_EQUAL( output, os.str() );
  }

}

//Check constructor of abstract base class.
BOOST_AUTO_TEST_CASE( output_field_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::output_field >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::output_field >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::output_field >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::output_field, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::output_field >::type{} );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( element_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::element_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::element_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::element_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::element_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::element_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  arr.append( { 0.0, 0.1 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::element_output aset( label1, unit1, true );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), false );
    aset.push_back( 0.1 );
    aset.push_back( 0.2 );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
  
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 3 );
      BOOST_CHECK_EQUAL( output2, "0.1" );
    }
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 1 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 3 );
      BOOST_CHECK_EQUAL( output2, "0.2" );
    }
  }
  {
    observable::element_output aset( label1, unit1, false );
    aset.push_back( 0.1 );
    aset.push_back( 0.2 );
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 3 );
      BOOST_CHECK_EQUAL( output2, "0.1" );
    }
    {
      aset.set_unit( unit2 );
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 3 );
      BOOST_CHECK_EQUAL( output2, "0.2" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::element_output > fld( new observable::element_output( label1, unit1, true ) );
    fld->push_back( 0.1 );
    fld->push_back( 0.2 );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( index_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::index_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::index_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::index_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::index_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::index_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  arr.append( { 0.0, 0.1 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::index_output aset( label1, unit1 );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), false );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 1 );
      BOOST_CHECK_EQUAL( output2, "0" );
    }
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 1 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 1 );
      BOOST_CHECK_EQUAL( output2, "0" );
    }
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 1 );
      BOOST_CHECK_EQUAL( output2, "1" );
    }
    {
      aset.set_unit( unit2 );
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 1 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 1 );
      BOOST_CHECK_EQUAL( output2, "1" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::index_output > fld( new observable::index_output( label1, unit1 ) );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( key_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::key_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::key_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::key_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::key_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::key_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  arr.append( { 0.0, 0.1 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::key_output aset( label1, unit1, true );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), false );
    aset.push_back( "L1" );
    aset.push_back( "L2" );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
  
    {
      aset.set_unit( unit2 );
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 2 );
      BOOST_CHECK_EQUAL( output2, "L1" );
    }
    {
      aset.set_unit( unit2 );
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 1 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 2 );
      BOOST_CHECK_EQUAL( output2, "L2" );
    }
  }
  {
    observable::key_output aset( label1, unit1, false );
    aset.push_back( "L1" );
    aset.push_back( "L2" );
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 2 );
      BOOST_CHECK_EQUAL( output2, "L1" );
    }
    {
      aset.set_unit( unit2 );
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 2 );
      BOOST_CHECK_EQUAL( output2, "L2" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::key_output > fld( new observable::key_output( label1, unit1, true ) );
    fld->push_back( "L1" );
    fld->push_back( "L2" );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( mean_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::mean_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::mean_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::mean_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::mean_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::mean_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  arr.append( { 0.0, 0.1 } );
  arr.append( { 0.1, 0.2 } );
  arr.append( { 0.2, 0.3 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::mean_output aset( label1, unit1 );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), true );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
  
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 3 );
      BOOST_CHECK_EQUAL( output2, "0.1" );
    }
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2.size(), 3 );
      BOOST_CHECK_EQUAL( output2, "0.2" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::mean_output > fld( new observable::mean_output( label1, unit1 ) );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( mean_variance_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::mean_variance_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::mean_variance_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::mean_variance_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::mean_variance_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::mean_variance_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  arr.append( { 0.0, 0.1 } );
  arr.append( { 0.1, 0.2 } );
  arr.append( { 0.2, 0.3 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::mean_variance_output aset( label1, unit1 );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), true );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
  
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "0.1 0.01" );
    }
    {
      aset.set_unit( unit2 );
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "0.2 0.01" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::mean_variance_output > fld( new observable::mean_variance_output( label1, unit1 ) );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( rank_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::rank_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::rank_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::rank_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::rank_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::rank_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  arr.append( { 0.0, 0.1 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::rank_output aset( label1, unit1 );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), false );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
  
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "0" );
    }
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "0" );
    }
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 1 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "1" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::rank_output > fld( new observable::rank_output( label1, unit1 ) );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( sample_count_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::sample_count_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::sample_count_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::sample_count_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::sample_count_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::sample_count_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  arr.append( { 0.0, 0.1 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::sample_count_output aset( label1, unit1 );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), false );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
  
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "1" );
    }
    {
      // Output should work
      std::stringstream os;
      arr.append( { 0.1, 0.2 } );
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 1 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "2" );
    }
    {
      // Output should work
      std::stringstream os;
      arr.append( { 0.2, 0.3 } );
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 1 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "3" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::sample_count_output > fld( new observable::sample_count_output( label1, unit1 ) );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( variance_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::variance_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::variance_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::variance_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::variance_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::variance_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  arr.append( { 0.0, 0.1 } );
  arr.append( { 0.1, 0.2 } );
  arr.append( { 0.2, 0.3 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::variance_output aset( label1, unit1 );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), true );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
  
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "0.01" );
    }
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "0.01" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::variance_output > fld( new observable::variance_output( label1, unit1 ) );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( digitizer_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::digitizer_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::digitizer_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::digitizer_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::digitizer_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::digitizer_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  utility::estimate_array arr( 2 );
  utility::digitizer axis( 0.1, 0.2, 2 );
  arr.append( { 0.0, 0.1 } );
  arr.append( { 0.1, 0.2 } );
  arr.append( { 0.2, 0.3 } );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::digitizer_output aset( label1, unit1, axis, observable::digitizer_output::USE_ALL );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), false );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
  
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "0.1 0.125 0.15" );
    }
    {
      // Output should work
      std::stringstream os;
      BOOST_CHECK_NO_THROW( aset.write( os, arr, 1, 0 ) );
      const std::string output2( os.str() );
      BOOST_CHECK( not output2.empty() );
      BOOST_CHECK_EQUAL( output2, "0.15 0.175 0.2" );
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::digitizer_output > fld( new observable::digitizer_output( label1, unit1, axis, observable::digitizer_output::USE_ALL ) );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Check derived output_field class.
BOOST_AUTO_TEST_CASE( digitizer_3d_output_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::digitizer_3d_output >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::digitizer_3d_output >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::digitizer_3d_output >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::digitizer_3d_output, observable::output_field >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::digitizer_3d_output >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  geometry::digitizer_3d axis( { 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 }, 0.5 );
  utility::estimate_array arr( axis.size() );
  std::vector< double > vec( axis.size() );
  for( double & var : vec ) var = 0.1;
  arr.append( vec );
  for( double & var : vec ) var = 0.2;
  arr.append( vec );
  for( double & var : vec ) var = 0.3;
  arr.append( vec );
  const std::string label1( "name" );
  const std::string label2( "day" );
  const std::string label3( "month" );
  const std::string label4( "year" );
  const std::string unit1( "label" );
  const std::string unit2( "ordinal1" );
  const std::string unit3( "ordinal2" );
  const std::string unit4( "ordinal3" );
  {
    observable::digitizer_3d_output aset( label1, unit1, axis, observable::digitizer_3d_output::USE_ALL );
    BOOST_CHECK_EQUAL( aset.label(), label1 );
    BOOST_CHECK_EQUAL( aset.unit(), unit1 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
    BOOST_CHECK_EQUAL( aset.consume_value(), false );
  
    // "unset" label
    aset.set_label( emptr_str );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    // set unit
    aset.set_unit( unit2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.label(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    aset.set_label( label2 );
    BOOST_CHECK_EQUAL( aset.label(), label2 );
    BOOST_CHECK_EQUAL( aset.unit(), unit2 );
    BOOST_CHECK_EQUAL( aset.valid(), true );
  
    // "unset" unit
    aset.set_unit( emptr_str );
    BOOST_CHECK_EQUAL( aset.unit(), emptr_str );
    BOOST_CHECK_EQUAL( aset.valid(), false );
  
    {
      // Output error if not valid
      try
      {
        std::stringstream os;
        aset.write( os, arr, 0, 0 );
        BOOST_ERROR( "expected \"copy.write( std::cout, 0 )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write incomplete field" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
      }
    }
    aset.set_unit( unit2 );
    {
      // Output should work
      std::vector< std::string > output2
      {
        "0 0 0 0.25 0.25 0.25 0.5 0.5 0.5",
        "0.5 0 0 0.75 0.25 0.25 1 0.5 0.5",
        "0 0.5 0 0.25 0.75 0.25 0.5 1 0.5",
        "0.5 0.5 0 0.75 0.75 0.25 1 1 0.5",
        "0 0 0.5 0.25 0.25 0.75 0.5 0.5 1",
        "0.5 0 0.5 0.75 0.25 0.75 1 0.5 1",
        "0 0.5 0.5 0.25 0.75 0.75 0.5 1 1",
        "0.5 0.5 0.5 0.75 0.75 0.75 1 1 1" };
      for( std::size_t idx = 0; idx != axis.size(); ++idx )
      {
        std::stringstream os;
        BOOST_CHECK_NO_THROW( aset.write( os, arr, idx, 0 ) );
        const std::string out( os.str() );
        //std::cout << "(" << idx << ")" << out << "\n"; 
        BOOST_CHECK( not out.empty() );
        BOOST_CHECK_EQUAL( out, output2[ idx ] );
      }
      {
        // Output should work
        std::stringstream os;
        BOOST_CHECK_NO_THROW( aset.write( os, arr, 0, 1 ) );
        const std::string out2( os.str() );
        BOOST_CHECK( not out2.empty() );
        BOOST_CHECK_EQUAL( out2, output2[ 0 ] );
      }
    }
  }
  
  // base class tests
  {
    std::unique_ptr< observable::digitizer_3d_output > fld( new observable::digitizer_3d_output( label1, unit1, axis, observable::digitizer_3d_output::USE_ALL ) );
    output_test_suite::base_output_field_test( std::move( fld ) );
  }

}

//Test output_row base class features.
void output_test_suite::base_output_row_test(std::unique_ptr< observable::output_row > data)
{
  // Use serialization to make copies of the datum.
  std::string capture;
  std::string labels, units, rows;
  {
    std::stringstream store;
    boost::archive::text_oarchive oa( store );
    oa << data;
    capture = store.str();
    {
      std::stringstream os;
      data->labels( os );
      labels = os.str();
    }
    {
      std::stringstream os;
      data->units( os );
      units = os.str();
    }
    {
      std::stringstream os;
      data->row( os );
      rows = os.str();
    }
  }
  {
    std::unique_ptr< observable::output_row > data2;
    std::stringstream store( capture );
    boost::archive::text_iarchive ia( store );
    ia >> data2;
  
    {
      std::stringstream os;
      data->labels( os );
      BOOST_REQUIRE_EQUAL( labels, os.str() );
    }
    {
      std::stringstream os;
      data->units( os );
      BOOST_REQUIRE_EQUAL( units, os.str() );
    }
    {
      std::stringstream os;
      data->row( os );
      BOOST_REQUIRE_EQUAL( rows, os.str() );
    }
  }
  // Try with output_rows object
  // ---------------------------
  {
    std::unique_ptr< observable::output_row > data2;
    std::stringstream store( capture );
    boost::archive::text_iarchive ia( store );
    ia >> data2;
  
    observable::output_rows rws( "label", false );
    // Output error
    BOOST_CHECK_NO_THROW( rws.receive_data( 0, std::move( data2 ) ) );
  }

}

// Test output_dataset derived class
BOOST_AUTO_TEST_CASE( output_test_row_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< output_test_suite::output_test_row >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< output_test_suite::output_test_row >::type{} );
    BOOST_CHECK( not std::is_move_constructible< output_test_suite::output_test_row >::type{} );
    BOOST_CHECK( not ( std::is_assignable< output_test_suite::output_test_row, output_test_suite::output_test_row >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< output_test_suite::output_test_row >::type{} );
  }
  // ----------
  const std::string test_row{ "2345 3245.00" };
  {
    output_test_suite::output_test_row arow;
    arow.buffer = test_row;
    {
      std::stringstream os;
      arow.labels( os );
      BOOST_REQUIRE_EQUAL( "INDEX MEAN", os.str() );
    }
    {
      std::stringstream os;
      arow.units( os );
      BOOST_REQUIRE_EQUAL( "ORDINAL RATE", os.str() );
    }
    {
      std::stringstream os;
      arow.row( os );
      BOOST_REQUIRE_EQUAL( "2345 3245.00", os.str() );
    }
    {
      std::unique_ptr< output_test_suite::output_test_row > rw;
      rw.reset( new output_test_suite::output_test_row );
      rw->buffer = "\n0 1.0";
      arow.merge( std::move( rw ) );
    }
    {
      std::stringstream os;
      arow.row( os );
      BOOST_REQUIRE_EQUAL( "2345 3245.00\n0 1.0", os.str() );
    }
  }
  {
    std::unique_ptr< output_test_suite::output_test_row > dtm;
    dtm.reset( new output_test_suite::output_test_row );
    dtm->buffer = test_row;
    output_test_suite::base_output_row_test( std::move( dtm ) );
  }

}

// Test output_row derived class
BOOST_AUTO_TEST_CASE( widom_row_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::widom_row >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::widom_row >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::widom_row >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::widom_row, observable::widom_row >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::widom_row >::type{} );
  }
  // ----------
  {
    observable::widom_row arow( 0 );
    {
      std::stringstream os;
      arow.labels( os );
      BOOST_REQUIRE_EQUAL( "INDEX ", os.str() );
    }
    {
      std::stringstream os;
      arow.units( os );
      BOOST_REQUIRE_EQUAL( "ORDINAL ", os.str() );
    }
    {
      std::stringstream os;
      arow.row( os );
      BOOST_REQUIRE_EQUAL( "0 ", os.str() );
    }
    {
      observable::widom::widom_datum dtm( 0 );
      dtm.count_ = 1;
      dtm.specie_count_.append( 4 );
      dtm.exp_potential_.append( 0.1 );
      dtm.conc_ = 0.1;
      dtm.excess_potential_ = 0.0;
      dtm.label_ = "Na";
      dtm.volume_ = 10000.0;
      arow.append( &dtm, &dtm + 1 );
    }
    {
      std::stringstream os;
      arow.labels( os );
      BOOST_CHECK_EQUAL( "INDEX Na_EXCESS0 Na_POTENTIAL Na_EXCESS Na_COUNT ", os.str() );
    }
    {
      std::stringstream os;
      arow.units( os );
      BOOST_CHECK_EQUAL( "ORDINAL ENERGY ENERGY ENERGY COUNT ", os.str() );
    }
    {
      std::stringstream os;
      arow.row( os );
      BOOST_CHECK_EQUAL( "0 0 2.30259 2.71173 1 ", os.str() );
    }
  }
  {
    //std::unique_ptr< observable::widom_row > dtm;
    //dtm.reset( new observable::widom_row );
    //dtm->buffer = test_row;
    //output_test_suite::base_output_row_test( std::move( dtm ) );
  }

}

//Test output_datum base class features.
void output_test_suite::base_output_datum_test(std::unique_ptr< observable::output_datum > data, int16_t usewith)
{
  const int16_t USE_SERIES{ 1 };
  const int16_t USE_TEXT{ 2 };
  const int16_t USE_ROW{ 4 };
  // Use serialization to make copies of the datum.
  std::string capture;
  {
    std::stringstream store;
    boost::archive::text_oarchive oa( store );
    oa << data;
    capture = store.str();
  }  
  const std::string label{ "label" };
  // Try with output_series object
  // -----------------------------
  {
    std::unique_ptr< observable::output_datum > data2;
    std::stringstream store( capture );
    boost::archive::text_iarchive ia( store );
    ia >> data2; 
     
    observable::output_series ser( label, false );
    if( 0 != ( USE_SERIES & usewith ) )
    {
      BOOST_CHECK_NO_THROW( ser.accept( std::move( data2 ) ) );
    }
    else
    {
      // Output error
      try
      {
        ser.accept( std::move( data2 ) );
        BOOST_ERROR( "expected \"XXX.accept( std::move( data2 ) )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "This output datum can not update an output_series object." ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"XXX.accept( std::move( data2 ) )\" was not expected type: " ) + err.what() );
      }
    }
  }
  // Try with output_text object
  // ---------------------------
  {
    std::unique_ptr< observable::output_datum > data2;
    std::stringstream store( capture );
    boost::archive::text_iarchive ia( store );
    ia >> data2; 
     
    observable::output_text txt( label, false );
    if( 0 != ( USE_TEXT & usewith ) )
    {
      BOOST_CHECK_NO_THROW( txt.accept( std::move( data2 ) ) );
    }
    else
    {
      // Output error
      try
      {
        txt.accept( std::move( data2 ) );
        BOOST_ERROR( "expected \"XXX.accept( std::move( data2 ) )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "This output datum can not update an output_text object." ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"XXX.accept( std::move( data2 ) )\" was not expected type: " ) + err.what() );
      }
    }
  }
  // Try with output_rows object
  // ---------------------------
  {
    std::unique_ptr< observable::output_datum > data2;
    std::stringstream store( capture );
    boost::archive::text_iarchive ia( store );
    ia >> data2; 
     
    observable::output_rows rws( label, false );
    if( 0 != ( USE_ROW & usewith ) )
    {
      BOOST_CHECK_NO_THROW( rws.accept( std::move( data2 ) ) );
    }
    else
    {
      // Output error
      try
      {
        rws.accept( std::move( data2 ) );
        BOOST_ERROR( "expected \"XXX.accept( std::move( data2 ) )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "This output datum can not update an output_rows object." ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"XXX.accept( std::move( data2 ) )\" was not expected type: " ) + err.what() );
      }
    }
  }

}

// Test output_dataset derived class
BOOST_AUTO_TEST_CASE( output_test_datum_test )
{
  //XX {
  //XX   // Static Lifetime method tests
  //XX   BOOST_CHECK( not std::is_default_constructible< observable::output_series >::type{} );
  //XX   BOOST_CHECK( not std::is_copy_constructible< observable::output_series >::type{} );
  //XX   BOOST_CHECK( not std::is_move_constructible< observable::output_series >::type{} );
  //XX   BOOST_CHECK( not ( std::is_assignable< observable::output_series, observable::output_series >::type{} ) );
  //XX   BOOST_CHECK( std::has_virtual_destructor< observable::output_series >::type{} );
  //XX }
  // ----------
  output_test_suite::output_test_datum::buffer = "EMPTY ";
  std::string canon( "EMPTY SERIES TEXT ROWS " );
  {
    std::unique_ptr< observable::output_datum > dtm( new output_test_suite::output_test_datum );
    output_test_suite::base_output_datum_test( std::move( dtm ), 7 );
    BOOST_CHECK_EQUAL( output_test_suite::output_test_datum::buffer, canon );
  }

}

// Test output_datum derived class
BOOST_AUTO_TEST_CASE( output_series_datum_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< observable::output_series_datum >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::output_series_datum >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::output_series_datum >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::output_series_datum, observable::output_series_datum >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::output_series_datum >::type{} );
  }
  // ----------
  observable::memory_sink sink;
  {
    std::unique_ptr< utility::estimate_array > arr( new utility::estimate_array( 2 ) );
    arr->append( { 0.0, 0.1 } );
    arr->append( { 0.1, 0.2 } );
    arr->append( { 0.2, 0.3 } );
    std::unique_ptr< observable::output_datum > dtm( new observable::output_series_datum( 0, std::move( arr ) ) );
    observable::output_series aset( "test1.dat", false );
    aset.push_back_field( std::move( std::unique_ptr< observable::output_field >( new observable::index_output( "INDEX", "ORDINAL" ) ) ) );
    aset.push_back_field( std::move( std::unique_ptr< observable::output_field >( new observable::mean_output( "MEAN", "VALUE" ) ) ) );
    aset.accept( std::move( dtm ) );
    aset.write( sink );
    BOOST_REQUIRE( sink.exists( "test1.dat" ) );
  }
  {
    std::unique_ptr< utility::estimate_array > arr( new utility::estimate_array( 2 ) );
    arr->append( { 0.0, 0.1 } );
    arr->append( { 0.1, 0.2 } );
    arr->append( { 0.2, 0.3 } );
    observable::output_series aset( "test2.dat", false );
    aset.push_back_field( std::move( std::unique_ptr< observable::output_field >( new observable::index_output( "INDEX", "ORDINAL" ) ) ) );
    aset.push_back_field( std::move( std::unique_ptr< observable::output_field >( new observable::mean_output( "MEAN", "VALUE" ) ) ) );
    aset.receive_data( 0, std::move( arr ) );
    aset.write( sink );
    BOOST_REQUIRE( sink.exists( "test2.dat" ) );
  }
  {
    std::string buf1, buf2;
    sink.read( "test1.dat", buf1 );
    sink.read( "test2.dat", buf2 );
    BOOST_CHECK_EQUAL( buf1, buf2 );
  }
  {
    std::unique_ptr< utility::estimate_array > arr( new utility::estimate_array( 2 ) );
    arr->append( { 0.0, 0.1 } );
    arr->append( { 0.1, 0.2 } );
    arr->append( { 0.2, 0.3 } );
    std::unique_ptr< observable::output_datum > dtm( new observable::output_series_datum( 0, std::move( arr ) ) );
    output_test_suite::base_output_datum_test( std::move( dtm ), 1 );
  }

}

// Test output_datum derived class
BOOST_AUTO_TEST_CASE( output_buffer_datum_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< observable::output_buffer_datum >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::output_buffer_datum >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::output_buffer_datum >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::output_buffer_datum, observable::output_buffer_datum >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::output_buffer_datum >::type{} );
  }
  // ----------
  std::string canon( "0 1.2\n1 1.1\n2 1.2" );
  observable::memory_sink sink;
  {
    std::unique_ptr< observable::output_datum > dtm( new observable::output_buffer_datum( canon ) );
    observable::output_text aset( "test1.dat", false );
    aset.set_field_labels( "INDEX MEAN" );
    aset.set_units( "ORDINAL VALUE" );
    aset.accept( std::move( dtm ) );
    aset.write( sink );
    BOOST_REQUIRE( sink.exists( "test1.dat" ) );
  }
  {
    observable::output_text aset( "test2.dat", false );
    aset.set_field_labels( "INDEX MEAN" );
    aset.set_units( "ORDINAL VALUE" );
    aset.receive_data( canon );
    aset.write( sink );
    BOOST_REQUIRE( sink.exists( "test2.dat" ) );
  }
  {
    std::string buf1, buf2;
    sink.read( "test1.dat", buf1 );
    sink.read( "test2.dat", buf2 );
    BOOST_CHECK_EQUAL( buf1, buf2 );
  }
  {
    std::unique_ptr< observable::output_datum > dtm( new observable::output_buffer_datum( canon ) );
    output_test_suite::base_output_datum_test( std::move( dtm ), 2 );
  }

}

// Test output_datum derived class
BOOST_AUTO_TEST_CASE( output_row_datum_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( std::is_default_constructible< observable::output_row_datum >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::output_row_datum >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::output_row_datum >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::output_row_datum, observable::output_row_datum >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::output_row_datum >::type{} );
  }
  // ----------
  observable::memory_sink sink;
  {
    std::unique_ptr< output_test_suite::output_test_row > rw( new output_test_suite::output_test_row );
    rw->buffer = "0 1.1";
    
    std::unique_ptr< observable::output_row_datum > dtm( new observable::output_row_datum( std::move( rw ), 0ul ) );
    observable::output_rows aset( "test1.dat", false );
    aset.accept( std::move( dtm ) );
    aset.write( sink );
    BOOST_REQUIRE( sink.exists( "test1.dat" ) );
  }
  {
    std::unique_ptr< output_test_suite::output_test_row > rw( new output_test_suite::output_test_row );
    rw->buffer = "0 1.1";
    observable::output_rows aset( "test2.dat", false );
    aset.receive_data( 0ul, std::move( rw ) );
    aset.write( sink );
    BOOST_REQUIRE( sink.exists( "test2.dat" ) );
  }
  {
    std::string buf1, buf2;
    sink.read( "test1.dat", buf1 );
    sink.read( "test2.dat", buf2 );
    BOOST_CHECK_EQUAL( buf1, buf2 );
  }
  {
    std::unique_ptr< output_test_suite::output_test_row > rw( new output_test_suite::output_test_row );
    rw->buffer = "0 1.1";
    std::unique_ptr< observable::output_row_datum > dtm( new observable::output_row_datum( std::move( rw ), 0ul ) );
    output_test_suite::base_output_datum_test( std::move( dtm ), 4 );
  }

}

//Test output_dataset base class features.
void output_test_suite::base_output_dataset_test(std::unique_ptr< observable::output_dataset > data)
{
  std::stringstream store;
  std::string emptr_str;
  std::string output;
  const std::string label{ data->label() };
  const std::string title{ data->title() };
  {
    const std::string test_title{ "Something or other." };
    data->set_title( emptr_str );
    BOOST_CHECK_EQUAL( data->title(), emptr_str );
  
    data->set_title( test_title );
    BOOST_CHECK_EQUAL( data->title(), test_title );
  
    data->set_title( title );
    BOOST_CHECK_EQUAL( data->title(), title );
  
    // Output
    observable::memory_sink sink;
    data->write( sink );
    sink.read( data->label(), output );
   
    boost::archive::text_oarchive oa( store );
    oa << data; 
  }
  {
    std::unique_ptr< observable::output_dataset > data2;
    boost::archive::text_iarchive ia( store );
    ia >> data2; 
    BOOST_CHECK_EQUAL( data2->label(), label );
    BOOST_CHECK_EQUAL( data2->title(), title );
    observable::memory_sink sink;
    data2->write( sink );
    std::string out;
    sink.read( data->label(), out );
    std::stringstream is1( output ), is2( out );
    std::size_t count = 0;
    for( std::string s1, s2; std::getline( is1, s1 ) and std::getline( is2, s2 ); )
    {
      ++count;
      if( count == 1 )
      {
        continue;
      }
      // std::cout << "[" << s1 << "][" << s2 << "]\n";
      BOOST_CHECK_EQUAL( s1, s2 );
    }
  }

}

// Test output_dataset derived class
BOOST_AUTO_TEST_CASE( output_series_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::output_series >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::output_series >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::output_series >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::output_series, observable::output_series >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::output_series >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  const std::string test_label{ "result123123.dat"};
  const std::string test_title{ "Something or other 23453245." };
  {
    observable::output_series aset( test_label, true );
    BOOST_CHECK_EQUAL( aset.label(), test_label );
    BOOST_CHECK_EQUAL( aset.title(), emptr_str );
    BOOST_CHECK_EQUAL( aset.size(), 0 );
    BOOST_CHECK_EQUAL( aset.empty(), true );
    BOOST_CHECK_EQUAL( aset.is_serial(), true );
    BOOST_CHECK( aset.begin() == aset.end() );
  
    aset.set_title( test_title );
    BOOST_CHECK_EQUAL( aset.title(), test_title );
    BOOST_CHECK_EQUAL( aset.size(), 0 );
    BOOST_CHECK_EQUAL( aset.empty(), true );
    BOOST_CHECK( aset.begin() == aset.end() );
  }
  {
    // Output error if no entries
    try
    {
      observable::output_series aset( test_label, true );
      aset.do_header( std::cout );
      BOOST_ERROR( "expected \"dsetcp.do_header( std::cout )\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Can not write output with no fields defined" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    try
    {
      observable::output_series aset( test_label, true );
      aset.do_series_body( std::cout );
      BOOST_ERROR( "expected \"dsetcp.do_body( std::cout )\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Can not write output with no fields defined" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    try
    {
      observable::output_series aset( test_label, false );
      aset.do_array_body( std::cout );
      BOOST_ERROR( "expected \"dsetcp.do_body( std::cout )\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Can not write output with no fields defined" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    observable::memory_sink sink;
    observable::output_series aset( test_label, true );
    try
    {
      aset.write( sink );
      BOOST_ERROR( "expected \"dsetcp.write( sink )\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Can not write output with no fields defined" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    std::unique_ptr< utility::estimate_array > arr( new utility::estimate_array( 2 ) );
    arr->append( { 0.0, 0.1 } );
    arr->append( { 0.1, 0.2 } );
    arr->append( { 0.2, 0.3 } );
    std::unique_ptr< observable::output_series > aset( new observable::output_series( test_label, true ) );
    aset->push_back_field( std::move( std::unique_ptr< observable::output_field >( new observable::index_output( "INDEX", "ORDINAL" ) ) ) );
    aset->push_back_field( std::move( std::unique_ptr< observable::output_field >( new observable::mean_output( "MEAN", "VALUE" ) ) ) );
    aset->receive_data( 0ul, std::move( arr ) );
    output_test_suite::base_output_dataset_test( std::move( aset ) );
  }
  {
    std::unique_ptr< utility::estimate_array > arr( new utility::estimate_array( 2 ) );
    arr->append( { 0.0, 0.1 } );
    arr->append( { 0.1, 0.2 } );
    arr->append( { 0.2, 0.3 } );
    std::unique_ptr< observable::output_series > aset( new observable::output_series( test_label, false ) );
    aset->push_back_field( std::move( std::unique_ptr< observable::output_field >( new observable::index_output( "INDEX", "ORDINAL" ) ) ) );
    aset->push_back_field( std::move( std::unique_ptr< observable::output_field >( new observable::mean_output( "MEAN", "VALUE" ) ) ) );
    aset->receive_data( 0ul, std::move( arr ) );
    output_test_suite::base_output_dataset_test( std::move( aset ) );
  }

}

// Test output_dataset derived class
BOOST_AUTO_TEST_CASE( output_text_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::output_text >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::output_text >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::output_text >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::output_text, observable::output_text >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::output_text >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  const std::string test_label{ "result123123.dat"};
  const std::string test_title{ "Something or other 23453245." };
  const std::string test_fields{ "INDEX PMEAN" };
  const std::string test_units{ "ORDINAL VALUE" };
  {
    observable::output_text aset( test_label, true );
    BOOST_CHECK_EQUAL( aset.label(), test_label );
    BOOST_CHECK_EQUAL( aset.title(), emptr_str );
    aset.set_title( test_title );
    BOOST_CHECK_EQUAL( aset.title(), test_title );
    aset.set_field_labels( test_fields );
    aset.set_units( test_units );
    BOOST_CHECK_EQUAL( aset.is_serial(), true );
  
    aset.set_title( test_title );
    BOOST_CHECK_EQUAL( aset.title(), test_title );
  }
  {
    // Output error if no entries
    try
    {
      std::stringstream os;
      observable::output_text aset( test_label, true );
      aset.set_title( test_title );
      aset.set_field_labels( test_fields );
      aset.set_units( test_units );
      std::string buf{ "0 23.4" };
      aset.receive_data( buf );
      aset.do_header( os );
      BOOST_CHECK_EQUAL( "# TITLE: \"Something or other 23453245.\" \n# FIELDS: INDEX PMEAN \n# UNITS: ORDINAL VALUE \n", os.str() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    observable::memory_sink sink;
    observable::output_text aset( test_label, true );
    aset.set_title( test_title );
    aset.set_field_labels( test_fields );
    aset.set_units( test_units );
    std::string buf{ "0 23.4" };
    aset.receive_data( buf );
    try
    {
      aset.write( sink );
      std::string out;
      sink.read( test_label, out );
      const std::string canon( "# TITLE: \"Something or other 23453245.\" \n# FIELDS: INDEX PMEAN \n# UNITS: ORDINAL VALUE " );
      BOOST_CHECK( out.find( canon ) < out.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    std::unique_ptr< observable::output_text > aset( new observable::output_text( test_label, true ) );
    aset->set_title( test_title );
    aset->set_field_labels( test_fields );
    aset->set_units( test_units );
    std::string buf{ "0 23.4" };
    aset->receive_data( buf );
    output_test_suite::base_output_dataset_test( std::move( aset ) );
  }
  {
    std::unique_ptr< observable::output_text > aset( new observable::output_text( test_label, false ) );
    aset->set_title( test_title );
    aset->set_field_labels( test_fields );
    aset->set_units( test_units );
    std::string buf{ "0 23.4" };
    aset->receive_data( buf );
    output_test_suite::base_output_dataset_test( std::move( aset ) );
  }

}

// Test output_dataset derived class
BOOST_AUTO_TEST_CASE( output_rows_test )
{
  {
    // Static Lifetime method tests
    BOOST_CHECK( not std::is_default_constructible< observable::output_row >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::output_row >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::output_row >::type{} );
    BOOST_CHECK( not ( std::is_assignable< observable::output_row, observable::output_row >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::output_row >::type{} );
  }
  // CTOR TESTS
  // ----------
  std::string emptr_str;
  const std::string test_label{ "result123123.dat"};
  const std::string test_title{ "Something or other 23453245." };
  {
    observable::output_rows aset( test_label, true );
    BOOST_CHECK_EQUAL( aset.label(), test_label );
    BOOST_CHECK_EQUAL( aset.title(), emptr_str );
    aset.set_title( test_title );
    BOOST_CHECK_EQUAL( aset.title(), test_title );
    BOOST_CHECK_EQUAL( aset.is_serial(), true );
  
    aset.set_title( test_title );
    BOOST_CHECK_EQUAL( aset.title(), test_title );
  }
  {
    observable::memory_sink sink;
    observable::output_rows aset( test_label, true );
    aset.set_title( test_title );
    std::unique_ptr< output_test_suite::output_test_row > rw( new output_test_suite::output_test_row );
    rw->buffer = "0 1.1";
    aset.receive_data( 0ul, std::move( rw ) );
    try
    {
      aset.write( sink );
      std::string out;
      sink.read( test_label, out );
      const std::string canon( "# TITLE: \"Something or other 23453245.\" \n# FIELDS: INDEX MEAN\n# UNITS: ORDINAL RATE" );
      //std::cout << "\"" << out << "\"\n";
      BOOST_CHECK( out.find( canon ) < out.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    std::unique_ptr< observable::output_rows > aset( new observable::output_rows( test_label, true ) );
    aset->set_title( test_title );
    std::unique_ptr< output_test_suite::output_test_row > rw( new output_test_suite::output_test_row );
    rw->buffer = "0 1.1";
    aset->receive_data( 0ul, std::move( rw ) );
    output_test_suite::base_output_dataset_test( std::move( aset ) );
  }
  {
    std::unique_ptr< observable::output_rows > aset( new observable::output_rows( test_label, false ) );
    aset->set_title( test_title );
    std::unique_ptr< output_test_suite::output_test_row > rw( new output_test_suite::output_test_row );
    rw->buffer = "0 1.1";
    aset->receive_data( 0ul, std::move( rw ) );
    output_test_suite::base_output_dataset_test( std::move( aset ) );
  }

}

//Test the floating point environment
BOOST_AUTO_TEST_CASE( test_archive_file )
{
  const std::string paxpath( "myar.pax" );
  const std::string zippath( "myar.zip" );
  {
     // main constructor with name that needs to
     // be changed.
     BOOST_CHECK_NO_THROW( observable::archive_file ar( paxpath ) );
     observable::archive_file ar( paxpath );
     BOOST_CHECK_EQUAL( ar.filename(), paxpath + ".zip" );
  }
  {
     // check filename conversion
     observable::archive_file ar( zippath );
     BOOST_CHECK_EQUAL( ar.filename(), zippath );
  }
  {
     // Standard tests
     observable::archive_file ar( zippath );
     output_test_suite::test_base_sink_method( ar );
  }

}

//Test the memory-sink class
BOOST_AUTO_TEST_CASE( test_memory_sink )
{
  const std::string root_path( "dummy_path" );
  {
     // main constructor.
     BOOST_CHECK_NO_THROW( observable::memory_sink ar( root_path ) );
  }
  {
     // Standard tests
     observable::memory_sink ar( root_path );
     BOOST_CHECK_EQUAL( ar.filename(), root_path );
     output_test_suite::test_base_sink_method( ar );
  }

}

//Test the floating point environment
BOOST_AUTO_TEST_CASE( test_gdbm_sink )
{
  const std::string paxpath( "myar.pax" );
  const std::string gdbmpath( "myar.dbm" );
  {
     // main constructor with name that needs to
     // be changed.
     BOOST_CHECK_NO_THROW( observable::gdbm_sink ar( paxpath ) );
     observable::gdbm_sink ar( paxpath );
     BOOST_CHECK_EQUAL( ar.filename(), paxpath + ".dbm" );
  }
  {
     // check filename no conversion
     observable::gdbm_sink ar( gdbmpath );
     BOOST_CHECK_EQUAL( ar.filename(), gdbmpath );
  }
  {
     // Standard tests
     observable::gdbm_sink ar( gdbmpath );
     output_test_suite::test_base_sink_method( ar );
  }

}

//Test generic operations on a sink object
void output_test_suite::test_base_sink_method(observable::base_sink & sink)
{
  const std::string path( "dat/amx.dat" );
  const std::string content( "a1b2c3d4e5" );
  const std::string content2( "a5b4c3d2e1" );
  // BASIC OPERATION
  {
    // UUID is non-empty string with 32 characters.
    BOOST_CHECK( not sink.uuid().empty() );
    BOOST_CHECK_EQUAL( sink.uuid().size(), 32ul );
    // write to file
    BOOST_CHECK_NO_THROW( sink.write( path, content ) );
    // check path now exists
    BOOST_CHECK_EQUAL( sink.exists( path ), true );
    // read from file
    std::string buffer;
    bool success;
    BOOST_CHECK_NO_THROW( success = sink.read( path, buffer ) );
    BOOST_CHECK_MESSAGE( success, "read after write failed" );
    BOOST_CHECK_EQUAL( content, buffer );
  
    // test of write twice to same path in file then read
    BOOST_CHECK_NO_THROW( sink.write( path, content ) );
    BOOST_CHECK_NO_THROW( sink.write( path, content2 ) );
    BOOST_CHECK_NO_THROW( success = sink.read( path, buffer ) );
    BOOST_CHECK_MESSAGE( success, "read after write failed" );
    BOOST_CHECK_EQUAL( content2, buffer );
  
    // test of append then read
    BOOST_CHECK_NO_THROW( sink.write( path, content ) );
    BOOST_CHECK_NO_THROW( sink.append( path, content2 ) );
    BOOST_CHECK_NO_THROW( success = sink.read( path, buffer ) );
    BOOST_CHECK_MESSAGE( success, "read failed" );
    BOOST_CHECK_EQUAL( content + content2, buffer );
  }
  // ERROR CONDITIONS that should have strong exception guarrantee
  {
    // OPERATIONS WITH EMPTY PATH
    const std::string emptypath;
    std::string buffer;
    {
      // Specification is for strong exception guarantee
      const std::string fncall { "sink.write( emptypath, content )" };
      try
      {
        sink.write( emptypath, content );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write to an empty location/path" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    {
      // Specification is for strong exception guarantee
      const std::string fncall { "sink.append( emptypath, content )" };
      try
      {
        sink.append( emptypath, content );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not append to an empty location/path" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    {
      // Specification is for strong exception guarantee
      const std::string fncall { "sink.exists( emptypath )" };
      try
      {
        sink.exists( emptypath );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not check for an empty location/path" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    {
      // Specification is for strong exception guarantee
      const std::string fncall { "sink.read( emptypath, buffer )" };
      try
      {
        sink.read( emptypath, buffer );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not read from an empty location/path" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
  
    // OPERATIONS WITH EMPTY WRITE BUFFER
    {
      // Specification is for strong exception guarantee
      const std::string fncall { "sink.write( path, emptypath )" };
      try
      {
        sink.write( path, emptypath );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Can not write without content" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
  }
  // TEST OF SERIALIZATION
  std::stringstream store;
  {
    // serialize
    boost::archive::text_oarchive oa( store );
    oa << sink;
    BOOST_CHECK_EQUAL( sink.exists( path ), true );
    std::string buffer;
    bool success;
    BOOST_CHECK_NO_THROW( success = sink.read( path, buffer ) );
    BOOST_CHECK_MESSAGE( success, "read failed" );
    BOOST_CHECK_EQUAL( content + content2, buffer );
    // Changes to sink made after serialization may or may not
    // be available after deserialization.
    BOOST_CHECK_NO_THROW( sink.write( path, content2 ) );
    BOOST_CHECK_NO_THROW( success = sink.read( path, buffer ) );
    BOOST_CHECK_MESSAGE( success, "read after write failed" );
    BOOST_CHECK_EQUAL( content2, buffer );
  }
  {
    boost::archive::text_iarchive ia( store );
    ia >> sink;
    BOOST_CHECK_EQUAL( sink.exists( path ), true );
    std::string buffer;
    bool success;
    BOOST_CHECK_NO_THROW( success = sink.read( path, buffer ) );
    BOOST_CHECK_MESSAGE( success, "read failed" );
    // Change made after serialization may or may not be
    // available. However, content should be one of two
    // well-defined values
    BOOST_CHECK( content + content2 == buffer or content2 == buffer );
  }
  
  
  

}


#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(output_test_suite::output_test_datum, "output_test_suite::output_test_datum");
BOOST_CLASS_EXPORT_GUID(output_test_suite::output_test_row, "output_test_suite::output_test_row");