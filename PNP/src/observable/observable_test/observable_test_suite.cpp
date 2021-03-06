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

#define BOOST_TEST_MODULE observable_test
#include <boost/test/unit_test.hpp>

#include "observable/observable_test/observable_test_suite.hpp"
#include "observable/acceptance_observable.hpp"
#include "core/constants.hpp"
#include "observable/d3_distribution.hpp"
#include "observable/density_zaxis.hpp"
#include "geometry/digitizer_3d.hpp"
#include "observable/metropolis_sampler.hpp"
#include "observable/output_file.hpp"
#include "observable/output_series.hpp"
#include "observable/rdf_sampler.hpp"
#include "observable/report_manager.hpp"
#include "observable/sampler_meta.hpp"
#include "observable/specie_count.hpp"
#include "observable/specie_region_count.hpp"
#include "core/strngs.hpp"
#include "observable/base_observable.hpp"
#include "observable/trial_observer.hpp"
#include "observable/widom.hpp"
#include "core/input_parameter_memo.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"

// Manuals
#include "core/input_document.hpp"
#include "core/input_delegater.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
#include "core/input_preprocess.hpp"
#include "evaluator/coulomb.hpp"
#include "geometry/cube_region.hpp"
#include "geometry/periodic_cube_region.hpp"
#include "observable/memory_sink.hpp"
#include "particle/change_set.hpp"
#include "particle/ensemble.hpp"
#include "utility/fuzzy_equals.hpp"
// -
//#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
//#include <type_traits>
// - 
std::vector< core::input_parameter_memo > observable_test_suite::mockup_params()
{
  std::vector< core::input_parameter_memo > params;
  core::input_parameter_memo tmp;
  tmp.set_name( "end" );
  params.push_back( std::move( tmp ) );
  return std::move( params );
}

// Mockup set of realistic species.
//
//  specie_count = 5
//  solute = "Na" (x2 particles) and "Cl" (x1 particle)
//  mobile = "CA" (x1 particle)
//  flexible = "CO" (x1 particle)
//  channel only = "OX" (x1 particle)
//  
boost::shared_ptr< particle::particle_manager > observable_test_suite::mockup_particle_manager()
{
  boost::shared_ptr< particle::particle_manager > pman( new particle::particle_manager );
  {
    particle::specie spc1;
    spc1.set_label( "CA" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.11 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( 1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::MOBILE );
    spc1.append_position( geometry::coordinate( 2.0, 2.0, 0.0 ), geometry::centroid( 3.0, 2.0, 2.0, 0.0 ) );
    pman->add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "CO" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::FLEXIBLE );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 2.0 ), geometry::centroid( 3.0, 0.0, 0.0, 2.0 ) );
    pman->add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "OX" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::CHANNEL_ONLY );
    spc1.append_position( geometry::coordinate( 2.0, 0.0, 2.0 ) );
    pman->add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "Na" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( 1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::SOLUTE );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 2.0, 0.0, 0.0 ) );
    pman->add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "Cl" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.2 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.3123 );
    spc1.set_type( particle::specie::SOLUTE );
    spc1.append_position( geometry::coordinate( 0.0, 2.0, 0.0 ) );
    pman->add_specie( spc1 );
  }
  pman->add_predefined_particles();
  return pman;

}

// Create geometry manager with PBC cube "cell" and 
// open cube "bulk" regions.
boost::shared_ptr< geometry::geometry_manager > observable_test_suite::mockup_geometry_manager()
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 5.0 ) );
  boost::shared_ptr< geometry::geometry_manager > gman( new geometry::geometry_manager( regn ) );
  {
    boost::shared_ptr< geometry::base_region > bulk( new geometry::cube_region( "bulk", 4.0, geometry::coordinate( 0.0, 0.0, 0.0 ), true ) );
    gman->add_region( bulk );
  }
  return gman;

}

// Mockup evaluator
//
//  size = 1
//  get_evaluator()[ 0 ].type_label = "coulomb"
//  
boost::shared_ptr< evaluator::evaluator_manager > observable_test_suite::mockup_evaluator_manager()
{
  boost::shared_ptr< evaluator::evaluator_manager > eman( new evaluator::evaluator_manager );
  std::map< std::string, std::string > param;
  core::input_preprocess reader;
  eman->add_evaluator( evaluator::coulomb::make_evaluator( mockup_params() ) );
  return eman;

}

// Series of tests to exercise base observable class operations.
//
// - serialize
// - get_label
// - description
// - write_document
void observable_test_suite::base_observable_method_test(boost::shared_ptr< observable::base_observable > obs, std::string label)
{
  std::stringstream store;
  std::string description;
  std::string document;
  {
    BOOST_CHECK_EQUAL( label, obs->get_label() );
    {
      std::stringstream desc;
      obs->description( desc );
      description = desc.str();
    }
    {
      std::stringstream doc;
      core::input_document idoc( 1ul );
      obs->write_document( idoc );
      idoc.write( doc );
      document = doc.str();
      BOOST_CHECK_LT( document.find( label ), document.size() );
    }
    boost::archive::text_oarchive oa( store );
    oa << obs;
  }
  {
    boost::shared_ptr< observable::base_observable > copy;
    boost::archive::text_iarchive ia( store );
    ia >> copy;
    BOOST_CHECK_EQUAL( label, copy->get_label() );
    {
      std::stringstream desc;
      copy->description( desc );
    }
    {
      std::stringstream doc;
      core::input_document idoc( 1ul );
      copy->write_document( idoc );
      idoc.write( doc );
      BOOST_CHECK_EQUAL( document, doc.str() );
    }
  }
  

}

BOOST_AUTO_TEST_CASE( sampler_definition_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::sampler_definition >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::sampler_definition >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::sampler_definition >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::sampler_definition, observable::sampler_definition >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::sampler_definition >::type{} );
  }
  {
    // default ctor
    observable::sampler_definition tmp( "acceptance", "description", []( std::vector< core::input_parameter_memo > const& a )->boost::shared_ptr< observable::base_observable >{
      return boost::shared_ptr< observable::base_observable >{};
    } );
  
    BOOST_CHECK( tmp.empty() );
    BOOST_CHECK_EQUAL( tmp.size(), 0ul );
    BOOST_CHECK_EQUAL( tmp.label(), "acceptance" );
    BOOST_CHECK_EQUAL( tmp.description(), "description" );
  }

}

BOOST_AUTO_TEST_CASE( tracked_definition_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::tracked_definition >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::tracked_definition >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::tracked_definition >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::tracked_definition, observable::tracked_definition >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::tracked_definition >::type{} );
  }
  {
    // default ctor
    observable::tracked_definition tmp( "acceptance", "description", []( std::vector< core::input_parameter_memo >const& a )->boost::shared_ptr< observable::tracked_observable >{
      return boost::shared_ptr< observable::tracked_observable >{};
    } );
  
    BOOST_CHECK( tmp.empty() );
    BOOST_CHECK_EQUAL( tmp.size(), 0ul );
    BOOST_CHECK_EQUAL( tmp.label(), "acceptance" );
    BOOST_CHECK_EQUAL( tmp.description(), "description" );
  }

}

BOOST_AUTO_TEST_CASE( sampler_meta_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::sampler_meta >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::sampler_meta >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::sampler_meta >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::sampler_meta, observable::sampler_meta >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::sampler_meta >::type{} );
  }
  {
    // default ctor
    boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
    observable::sampler_meta tmp( oman );
  
    //BOOST_CHECK( tmp.empty() );
    //BOOST_CHECK_EQUAL( tmp.size(), 0ul );
  }

}

// Attempt to insert two definitions with the same label
BOOST_AUTO_TEST_CASE( sampler_meta_double_definition_insertion_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::acceptance_observable::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "trial-rate" ) );
  
  try
  {
    observable::acceptance_observable::add_definition( *m );
    BOOST_ERROR( "expected exception from adding the same definition twice" );
  }
  catch( std::runtime_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Attempt to add more than one chooser factory for type \"trial-rate\"." ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( report_manager_lifetime_test )
{
  {
    // Static Lifetime method tests: no copy / non-virtual
    BOOST_CHECK( std::is_default_constructible< observable::report_manager >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::report_manager >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::report_manager >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::report_manager, observable::report_manager >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< observable::report_manager >::type{} );
  }
  {
    // default ctor
    observable::report_manager tmp;
  
    BOOST_CHECK( tmp.empty() );
    BOOST_CHECK( not tmp.has_sink() );
    BOOST_CHECK_EQUAL( tmp.size(), 0ul );
  }

}

//Methods tested
// build_input_delegate
// description
// empty
// get_sink
// get_sample
// get_tracked
// has_sample
// has_tracked
// set_sink
// serialize
// size
// write_document
//Tested via input tests
// add_sample/add_tracked
//Untested
// remove_sample/remove_tracked
//Methods not tested (all forwarding methods to sampler objects)
// on_trial_end
// on_sample
// on_report
// prepare

BOOST_AUTO_TEST_CASE( report_manager_methods_test )
{
  // Methods tested
  //  * build_input_delegate
  //  * description
  //  * empty
  //  * get_sink
  //  * get_sample
  //  * get_tracked
  //  * has_sink
  //  * has_sample
  //  * has_tracked
  //  * set_sink
  //  * serialize
  //  * size
  //  * write_document
  // Untested
  //  remove_sample/remove_tracked
  // Methods not tested (all forwarding methods to sampler objects)
  //  on_trial_end
  //  on_sample
  //  on_report
  //  prepare
  std::stringstream store;
  {
    boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
    BOOST_CHECK( oman->empty() );
    BOOST_CHECK_EQUAL( oman->size(), 0ul );
  
    {
      boost::shared_ptr< observable::base_sink > snk( new observable::memory_sink );
      BOOST_CHECK( not oman->has_sink() );
      oman->set_sink( snk );
      BOOST_CHECK( oman->has_sink() );
      BOOST_CHECK( &oman->get_sink() == snk.get() );
    }
  
    {
      core::input_delegater dg( 1 );
      observable::report_manager::build_input_delegater( oman, dg );
      BOOST_REQUIRE( dg.has_section( "sampler" ) );
      {
        core::input_help hlpr;
        dg.get_documentation( hlpr );
        {
          std::stringstream ss;
          hlpr.write( ss );
          const std::string hstr( ss.str() );
          BOOST_CHECK( hstr.find( "trial-rate" ) < hstr.size() );
          BOOST_CHECK( hstr.find( "population-zaxis" ) < hstr.size() );
          BOOST_CHECK( hstr.find( "3d-distribution" ) < hstr.size() );
          BOOST_CHECK( hstr.find( "metropolis" ) < hstr.size() );
          BOOST_CHECK( hstr.find( "rdf-specie" ) < hstr.size() );
          BOOST_CHECK( hstr.find( "specie-count" ) < hstr.size() );
          BOOST_CHECK( hstr.find( "specie-region-count" ) < hstr.size() );
          BOOST_CHECK( hstr.find( "trial-log" ) < hstr.size() );
          BOOST_CHECK( hstr.find( "widom" ) < hstr.size() );
          //std::cout << hstr << "\n";
        }
      }
  
      // Use Widom = tracked, specie_count = sample
      /////////////////////////////
      std::string canon_input( "sampler\niwidom 100\ntype widom\nspecie Na Cl\nend\n\nsampler\ntype specie-count\nend\n\n" );
      core::input_preprocess reader;
      reader.add_buffer( "dummy", canon_input );
      try
      {
        dg.read_input( reader );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
      }
    }
    BOOST_CHECK( not oman->empty() );
    BOOST_CHECK_EQUAL( oman->size(), 2ul );
    BOOST_CHECK( oman->has_tracked( "widom" ) );
    BOOST_CHECK( oman->has_sample( "specie-count" ) );
    boost::archive::text_oarchive oa( store );
    oa << oman;
  }
  {
    boost::shared_ptr< observable::report_manager > oman;
  
    boost::archive::text_iarchive ia( store );
    ia >> oman;
    BOOST_CHECK( not oman->empty() );
    BOOST_CHECK_EQUAL( oman->size(), 2ul );
    BOOST_CHECK( oman->has_tracked( "widom" ) );
    BOOST_CHECK( oman->has_sample( "specie-count" ) );
    BOOST_CHECK_EQUAL( oman->get_tracked( "widom" )->get_label(), "widom" );
    BOOST_CHECK_EQUAL( oman->get_sample( "specie-count" )->get_label(), "specie-count" );
  
    {
      std::stringstream ss;
      BOOST_REQUIRE( oman->has_sink() );
      oman->description( ss );
      const std::string dstr( ss.str() );
      //std::cout << dstr << "\n";
      BOOST_CHECK( dstr.find( "specie-count" ) < dstr.size() );
      BOOST_CHECK( dstr.find( "widom" ) < dstr.size() );
      BOOST_CHECK( dstr.find( "loop : 100" ) < dstr.size() );
    }
    {
      std::string canon( "sampler\niwidom 100\nspecie Na Cl\ntype widom\nend\n\nsampler\ntype specie-count\nend\n" );
      core::input_document wrtr( 1ul );
      oman->write_document( wrtr );
      std::stringstream idoc;
      wrtr.write( idoc );
      const std::string sdoc( idoc.str() );
      //std::cout << sdoc << "\n";
      BOOST_CHECK_LE( sdoc.find( canon ), sdoc.size() );
    }
  }
  
  

}

BOOST_AUTO_TEST_CASE( acceptance_observable_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::acceptance_observable >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::acceptance_observable >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::acceptance_observable >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::acceptance_observable, observable::acceptance_observable >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::acceptance_observable >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::tracked_observable > var( observable::acceptance_observable::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "trial-rate" );
  }

}

BOOST_AUTO_TEST_CASE( acceptance_observable_method_test )
{
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    observable::memory_sink sink;
    // factory ctor
    boost::shared_ptr< observable::tracked_observable > var( observable::acceptance_observable::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable::acceptance_observable *pvar = dynamic_cast< observable::acceptance_observable* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
  
    std::vector< particle::change_hash > dummy_trials;
    dummy_trials.push_back( { 0, 1, 1, 0 } ); // A 1
    dummy_trials.push_back( { 1, 1, 1, 0 } ); // B 0
    dummy_trials.push_back( { 0, 1, 1, 1 } ); // C 1
    dummy_trials.push_back( { 1, 1, 1, 0 } ); // B 0
    dummy_trials.push_back( { 0, 1, 1, 1 } ); // C 1
    dummy_trials.push_back( { 0, 1, 1, 0 } ); // A 0
    dummy_trials.push_back( { 0, 1, 1, 0 } ); // A 1
    dummy_trials.push_back( { 0, 1, 1, 1 } ); // C 0
    dummy_trials.push_back( { 1, 1, 1, 0 } ); // B 1
    dummy_trials.push_back( { 1, 1, 1, 0 } ); // B 0
    // Out first set of dummy trials
    {
      typedef boost::tuples::tuple< particle::change_hash, std::size_t, std::size_t, double, double > result_t;
      std::vector< result_t > dummy_results;
      dummy_results.push_back( result_t( { 0, 1, 1, 0 }, 2, 3, 0.66666666666667, 0.3 ) );
      dummy_results.push_back( result_t( { 1, 1, 1, 0 }, 1, 4, 0.25, 0.4 ) );
      dummy_results.push_back( result_t( { 0, 1, 1, 1 }, 2, 3, 0.66666666666667, 0.3 ) );
  
      std::size_t counter = 0;
      for( auto id : dummy_trials )
      {
        particle::change_set tmp;
        tmp.id( id );
        if( ( counter % 2 ) == 0 )
        {
          tmp.set_accept( true );
        }
        var->on_trial_end( tmp );
        ++counter;
      }
      for( auto const& res : dummy_results )
      {
        BOOST_REQUIRE( pvar->has_sample( boost::tuples::get< 0 >( res ) ) );
        BOOST_CHECK_EQUAL( pvar->get_sample( boost::tuples::get< 0 >( res ) ).first, boost::tuples::get< 1 >( res ) );
        BOOST_CHECK_EQUAL( pvar->get_sample( boost::tuples::get< 0 >( res ) ).second, boost::tuples::get< 2 >( res ) );
      }
      var->on_sample( *pman, *gman, *eman );
      for( auto const& res : dummy_results )
      {
        BOOST_REQUIRE( pvar->has_datum( boost::tuples::get< 0 >( res ) ) );
        BOOST_CHECK_CLOSE( pvar->get_datum( boost::tuples::get< 0 >( res ) ).mean( 0 ), boost::tuples::get< 3 >( res ), 0.00000001 );
        BOOST_CHECK_EQUAL( pvar->get_datum( boost::tuples::get< 0 >( res ) ).variance( 0 ), 0.0 );
        BOOST_CHECK_EQUAL( pvar->get_datum( boost::tuples::get< 0 >( res ) ).count(), 1ul );
        BOOST_CHECK_CLOSE( pvar->get_datum( boost::tuples::get< 0 >( res ) ).mean( 1 ), boost::tuples::get< 4 >( res ), 0.00000001 );
        BOOST_CHECK_EQUAL( pvar->get_datum( boost::tuples::get< 0 >( res ) ).variance( 1 ), 0.0 );
      }
    }
    {
      std::stringstream out;
      var->on_report( out, sink );
      // std::cout << out.str() << "\n";
      sink.write_dataset();
      BOOST_REQUIRE( sink.exists( "acceptance.dat" ) );
      // std::string buf;
      // sink.read( "acceptance.dat", buf );
      // std::cout << buf << "\n";
    }
    // Second set of dummy trials
    {
      typedef boost::tuples::tuple< particle::change_hash, std::size_t, std::size_t, double, double > result_t;
      std::vector< result_t > dummy_results;
      dummy_results.push_back( result_t( { 0, 1, 1, 0 }, 0, 3, 0, 0.3 ) );
      dummy_results.push_back( result_t( { 1, 1, 1, 0 }, 1, 4, 0.25, 0.4 ) );
      dummy_results.push_back( result_t( { 0, 1, 1, 1 }, 2, 3, 0.66666666666667, 0.3 ) );
      // A 0
      // B 1
      // C 0
      // B 0
      // C 1
      // A 0
      // A 0
      // C 1
      // B 0
      // B 0
  
      std::size_t counter = 0;
      for( auto id : dummy_trials )
      {
        particle::change_set tmp;
        tmp.id( id );
        if( ( counter % 3 ) == 1 )
        {
          tmp.set_accept( true );
        }
        var->on_trial_end( tmp );
        ++counter;
      }
      for( auto const& res : dummy_results )
      {
        BOOST_REQUIRE( pvar->has_sample( boost::tuples::get< 0 >( res ) ) );
        BOOST_CHECK_EQUAL( pvar->get_sample( boost::tuples::get< 0 >( res ) ).first, boost::tuples::get< 1 >( res ) );
        BOOST_CHECK_EQUAL( pvar->get_sample( boost::tuples::get< 0 >( res ) ).second, boost::tuples::get< 2 >( res ) );
      }
      var->on_sample( *pman, *gman, *eman );
      for( auto const& res : dummy_results )
      {
        BOOST_REQUIRE( pvar->has_datum( boost::tuples::get< 0 >( res ) ) );
        BOOST_CHECK_CLOSE( pvar->get_datum( boost::tuples::get< 0 >( res ) ).mean( 0 ), boost::tuples::get< 3 >( res ), 0.00000001 );
        BOOST_CHECK_EQUAL( pvar->get_datum( boost::tuples::get< 0 >( res ) ).variance( 0 ), 0.0 );
        BOOST_CHECK_EQUAL( pvar->get_datum( boost::tuples::get< 0 >( res ) ).count(), 1ul );
        BOOST_CHECK_CLOSE( pvar->get_datum( boost::tuples::get< 0 >( res ) ).mean( 1 ), boost::tuples::get< 4 >( res ), 0.00000001 );
        BOOST_CHECK_EQUAL( pvar->get_datum( boost::tuples::get< 0 >( res ) ).variance( 1 ), 0.0 );
      }
    }
    {
      std::string canon_log = "          Trial  Accept.av Accept.var   Trial.av  Trial.var\n   (0, 1, 1, 0) 0.00000000 0.00000000 0.30000000 0.00000000\n   (0, 1, 1, 1) 0.66666667 0.00000000 0.30000000 0.00000000\n   (1, 1, 1, 0) 0.25000000 0.00000000 0.40000000 0.00000000";
      std::stringstream out;
      var->on_report( out, sink );
      //std::cout << out.str() << "\n";
      BOOST_CHECK_LT( out.str().find( canon_log ), out.str().size() );
      sink.write_dataset();
      BOOST_REQUIRE( sink.exists( "acceptance.dat" ) );
      std::string buf;
      sink.read( "acceptance.dat", buf );
      //std::cout << buf << "\n";
  
      std::string canon_rep = "# TITLE: \"Trial acceptance and rate data.\" \n# FIELDS: Trial Accept.mean Accept.var Trial.mean Trial.var \n# UNITS: ID Rate Rate2 Rate Rate2 \n(0, 1, 1, 0) 0.333333 0.222222 0.3 0 \n(0, 1, 1, 1) 0.666667 0 0.3 0 \n(1, 1, 1, 0) 0.25 0 0.4 0";
      BOOST_CHECK_LT( buf.find( canon_rep ), buf.size() );
    }
    // Trial with new id key
    {
      particle::change_set tmp;
      tmp.id( { 1, 1, 1, 1 } );
      tmp.set_accept( true );
      try
      {
        var->on_trial_end( tmp );
        BOOST_ERROR( "expected \"var->on_trial_end( tmp )\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Attempt to extend acceptance sampler after defining the first report.\n\n** Increase reporting interval or trial probability. **\n" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"var->on_trial_end( tmp )\" was not expected type: " ) + err.what() );
      }
    }
  }
  
  

}

BOOST_AUTO_TEST_CASE( d3_distribution_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::d3_distribution >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::d3_distribution >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::d3_distribution >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::d3_distribution, observable::d3_distribution >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::d3_distribution >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::d3_distribution::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "3d-distribution" );
  }

}

// Test region list processing method.
//
// Fail cases tested
//
// Invalid specie Label : "Ca1,bulk"
//                      : "Ca,bulk,cell"
//
// Missing region : "Ca,bulk,Na"
// Duplicate specie : "Ca,bulk.Ca.cell"
// Missing label : " : cell"

BOOST_AUTO_TEST_CASE( d3_distribution_region_list_test )
{
  {
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "bulk";
    std::map< std::string, std::string > result{ observable::d3_distribution::process_region_list( " Ca : bulk" ) };
  
    BOOST_CHECK( canon == result );
  }
  {
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "bulk";
    std::map< std::string, std::string > result{ observable::d3_distribution::process_region_list( "Ca:bulk" ) };
  
    BOOST_CHECK( canon == result );
  }
  {
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "bulk";
    std::map< std::string, std::string > result{ observable::d3_distribution::process_region_list( "Ca bulk" ) };
  
    BOOST_CHECK( canon == result );
  }
  {
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "bulk";
    std::map< std::string, std::string > result{ observable::d3_distribution::process_region_list( "Ca,bulk" ) };
  
    BOOST_CHECK( canon == result );
  }
  {
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "bulk";
    canon[ "Na" ] = "cell";
    std::map< std::string, std::string > result{ observable::d3_distribution::process_region_list( "Ca:bulk \"Na\" \"cell\"" ) };
  
    BOOST_CHECK( canon == result );
  }
  {
    // Invalid specie
    std::map< std::string, std::string > canon;
    canon[ "Ca1" ] = "bulk";
    std::map< std::string, std::string > result;
    try
    {
      result = observable::d3_distribution::process_region_list( "Ca1,bulk" );
      BOOST_CHECK( canon == result );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Specie-region pair list  parameter \"region\" with value (Ca1,bulk) in section \"sampler\".\n** Specie label \"Ca1\" should have two exactly two letters. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    // Odd number of entries with bad specie name
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "bulk";
    std::map< std::string, std::string > result;
    try
    {
      result = observable::d3_distribution::process_region_list( "Ca,bulk,cell" );
      BOOST_CHECK( canon == result );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Specie-region pair list  parameter \"region\" with value (Ca,bulk,cell) in section \"sampler\".\n** Specie label \"cell\" should have two exactly two letters. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    // Odd number of entries
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "bulk";
    std::map< std::string, std::string > result;
    try
    {
      result = observable::d3_distribution::process_region_list( "Ca,bulk,Na" );
      BOOST_CHECK( canon == result );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Specie-region pair list  parameter \"region\" with value (Ca,bulk,Na) in section \"sampler\".\n** Specie label \"Na\" has no matching region name. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    // Duplicate specie
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "cell";
    std::map< std::string, std::string > result;
    try
    {
      result = observable::d3_distribution::process_region_list( "Ca,bulk,Ca,cell" );
      BOOST_CHECK( canon == result );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Specie-region pair list  parameter \"region\" with value (Ca,bulk,Ca,cell) in section \"sampler\".\n** Specie label \"Ca\" appears more than once in list **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }
  {
    // Missing specie
    std::map< std::string, std::string > canon;
    canon[ "Ca" ] = "cell";
    std::map< std::string, std::string > result;
    try
    {
      result = observable::d3_distribution::process_region_list( " , bulk" );
      BOOST_CHECK( canon == result );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Specie-region pair list  parameter \"region\" with value ( , bulk) in section \"sampler\".\n** Specie label \"bulk\" should have two exactly two letters. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
    }
  }

}

BOOST_AUTO_TEST_CASE( d3_distribution_method_test )
{
  const std::vector< std::string > filenames { { "d3df-CA.dat", "d3df-CO.dat", "d3df-OX.dat", "d3df-Na.dat", "d3df-Cl.dat" } };
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    std::vector< core::input_parameter_memo > params( 2 );
    params[ 0 ].set_name( "stepsize" );
    params[ 0 ].set_value( "2.0" );
    params[ 1 ] = observable_test_suite::mockup_params().back();
    boost::shared_ptr< observable::base_observable > var( observable::d3_distribution::make_sampler( params ) );
  
    observable::d3_distribution *pvar = dynamic_cast< observable::d3_distribution* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
  
    var->prepare( *pman, *gman, *eman, *rman );
    BOOST_REQUIRE_EQUAL( pvar->size(), pman->specie_count() );
  
    // first sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      for( auto filename : filenames )
      {
        BOOST_REQUIRE( not sink.exists( filename ) );
      }
      std::stringstream out;
      var->on_report( out, sink );
      //std::cout << out.str() << "\n";
    }
    // second sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
      //std::cout << out.str() << "\n";
    }
    // need to add/convert a particle
    {
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 2 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 1 );
      // convert a 3 to a 4
      const std::size_t idx = pman->get_ensemble().nth_specie_index( 3, 0 );
      particle::change_set exc;
      particle::change_atom a1, a2;
      a1.key = 3;
      a2.key = 4;
      a1.index = idx;
      a2.index = pman->get_ensemble().size();
      a1.old_position = pman->get_ensemble().position( idx );
      a1.do_old = true;
      a1.do_new = false;
      a2.new_position = pman->get_ensemble().position( idx );
      a2.do_new = true;
      a2.do_old = false;
      exc.add_atom( a1 );
      exc.add_atom( a2 );
      pman->commit( exc );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 1 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 2 );
  
      var->on_sample( *pman, *gman, *eman );
    }
    {
      std::stringstream out;
      const std::string uuid_head { "# UUID:" };
      const std::string label_head { "# FIELDS: XMID YMID ZMID P.mean P.var" };
      const std::string unit_head { "# UNITS: ANGSTROM ANGSTROM ANGSTROM Rate Rate2" };
      const std::string count_head { "# SAMPLE COUNT: 3" };
  
      const std::vector< std::string > canon_titles {
        "# TITLE: \"3D density distribution histogram for specie CA\"",
        "# TITLE: \"3D density distribution histogram for specie CO\"",
        "# TITLE: \"3D density distribution histogram for specie OX\"",
        "# TITLE: \"3D density distribution histogram for specie Na\"",
        "# TITLE: \"3D density distribution histogram for specie Cl\""
      };
      const std::vector< std::string > interesting_rows { {
          "3 3 1 1 0",
          "1 1 3 1 0",
          "3 1 3 1 0",
          "1 1 1 0.666667 0.333333",
          "1 1 1 0.333333 0.333333"
        }
      };
  
      var->on_report( out, sink );
      sink.write_dataset();
      //std::cout << out.str() << "\n";
      // should be no output on "out"
      BOOST_CHECK_EQUAL( out.str().size(), 0 );
      for( std::size_t idx = 0; idx != filenames.size(); ++idx )
      {
        std::string buf;
        BOOST_REQUIRE( sink.exists( filenames[ idx ] ) );
        sink.read( filenames[ idx ], buf );
        BOOST_CHECK_LT( buf.find( uuid_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( label_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( unit_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( count_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( canon_titles[ idx ] ), buf.size() );
        BOOST_CHECK_LT( buf.find( interesting_rows[ idx ] ), buf.size() );
        //std::cout << buf << "\n";
      }
    }
  }

}

BOOST_AUTO_TEST_CASE( density_zaxis_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::density_zaxis >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::density_zaxis >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::density_zaxis >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::density_zaxis, observable::density_zaxis >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::density_zaxis >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::density_zaxis::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "population-zaxis" );
  }

}

BOOST_AUTO_TEST_CASE( density_zaxis_method_test )
{
  const std::vector< std::string > filenames { { "gz-CA.dat", "gz-CO.dat", "gz-OX.dat", "gz-Na.dat", "gz-Cl.dat" } };
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    std::vector< core::input_parameter_memo > params( 2 );
    params[ 0 ].set_name( "stepsize" );
    params[ 0 ].set_value( "0.5" );
    params[ 1 ] = observable_test_suite::mockup_params().back();
    boost::shared_ptr< observable::base_observable > var( observable::density_zaxis::make_sampler( params ) );
  
    observable::density_zaxis *pvar = dynamic_cast< observable::density_zaxis* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
  
    var->prepare( *pman, *gman, *eman, *rman );
    BOOST_REQUIRE_EQUAL( pvar->size(), pman->specie_count() );
  
    // first sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
      //std::cout << out.str() << "\n";
      BOOST_CHECK_EQUAL( out.str().size(), 0 );
    }
    // second sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
      //std::cout << out.str() << "\n";
      BOOST_CHECK_EQUAL( out.str().size(), 0 );
    }
    // need to add/convert a particle
    {
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 2 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 1 );
      // convert a 3 to a 4
      const std::size_t idx = pman->get_ensemble().nth_specie_index( 3, 0 );
      particle::change_set exc;
      particle::change_atom a1, a2;
      a1.key = 3;
      a2.key = 4;
      a1.index = idx;
      a2.index = pman->get_ensemble().size();
      a1.old_position = pman->get_ensemble().position( idx );
      a1.do_old = true;
      a1.do_new = false;
      a2.new_position = pman->get_ensemble().position( idx );
      a2.do_new = true;
      a2.do_old = false;
      exc.add_atom( a1 );
      exc.add_atom( a2 );
      pman->commit( exc );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 1 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 2 );
  
      var->on_sample( *pman, *gman, *eman );
    }
    {
      std::stringstream out;
      const std::string uuid_head { "# UUID:" };
      const std::string label_head { "# FIELDS: ZMIN ZMID ZMAX P.mean P.var \n# UNITS: ANGSTROM ANGSTROM ANGSTROM Rate Rate2" };
      const std::string count_head { "# SAMPLE COUNT: 3" };
  
      const std::vector< std::string > canon_titles { {
          "# TITLE: \"Z-axial distribution histogram for specie 0\"",
          "# TITLE: \"Z-axial distribution histogram for specie 1\"",
          "# TITLE: \"Z-axial distribution histogram for specie 2\"",
          "# TITLE: \"Z-axial distribution histogram for specie 3\"",
          "# TITLE: \"Z-axial distribution histogram for specie 4\""
        }
      };
      const std::vector< std::string > interesting_rows { {
          "0 0.25 0.5 1 0",
          "2 2.25 2.5 1 0",
          "2 2.25 2.5 1 0",
          "0 0.25 0.5 1.66667 0.333333",
          "0 0.25 0.5 1.33333 0.333333"
        }
      };
  
      var->on_report( out, sink );
      //std::cout << out.str() << "\n";
      // should be no output on "out"
      BOOST_CHECK_EQUAL( out.str().size(), 0 );
      for( auto filename : filenames )
      {
        BOOST_REQUIRE( not sink.exists( filename ) );
      }
      sink.write_dataset();
      for( std::size_t idx = 0; idx != filenames.size(); ++idx )
      {
        std::string buf;
        BOOST_REQUIRE( sink.exists( filenames[ idx ] ) );
        sink.read( filenames[ idx ], buf );
        BOOST_CHECK_LT( buf.find( uuid_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( label_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( count_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( canon_titles[ idx ] ), buf.size() );
        BOOST_CHECK_LT( buf.find( interesting_rows[ idx ] ), buf.size() );
        //std::cout << buf << "\n";
      }
    }
  }

}

BOOST_AUTO_TEST_CASE( metropolis_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::metropolis_sampler >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::metropolis_sampler >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::metropolis_sampler >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::metropolis_sampler, observable::metropolis_sampler >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::metropolis_sampler >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::metropolis_sampler::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "metropolis" );
  }

}

BOOST_AUTO_TEST_CASE( metropolis_method_test )
{
  const std::string filename = "metropolis.dat";
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    boost::shared_ptr< observable::tracked_observable > var( observable::metropolis_sampler::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable::metropolis_sampler *pvar = dynamic_cast< observable::metropolis_sampler* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
  
    var->prepare( *pman, *gman, *eman, *rman );
    BOOST_CHECK_EQUAL( pvar->boltzmann_factor().count(), 0ul );
    BOOST_CHECK_EQUAL( pvar->energy_change().count(), 0ul );
    BOOST_CHECK_EQUAL( pvar->energy().count(), 0ul );
    BOOST_CHECK_EQUAL( pvar->energy().size(), 3000ul );
  
    // Energy of dummy trials
    std::vector< double > dummy_trials { 0.9, 1.3, 0.1, 2., 0.0, 4.2, 0.6  };
    // Out first set of dummy trials
    {
      for( auto energy : dummy_trials )
      {
        particle::change_set tmp;
        tmp.id( { 0ul, 1ul, 1ul, 0ul } );
        particle::change_atom at;
        at.energy_new = energy;
        tmp.add_atom( at );
        var->on_trial_end( tmp );
      }
      // first sample and report
      var->on_sample( *pman, *gman, *eman );
      std::stringstream out;
      var->on_report( out, sink );
    }
    // second sample and report
    {
      for( auto energy : dummy_trials )
      {
        particle::change_set tmp;
        tmp.id( { 0ul, 1ul, 1ul, 0ul } );
        particle::change_atom at;
        at.energy_new = energy * 1.1;
        tmp.add_atom( at );
        var->on_trial_end( tmp );
      }
      var->on_sample( *pman, *gman, *eman );
      {
        std::stringstream out;
        var->on_report( out, sink );
      }
    }
    // need to add/convert a particle
    {
      // convert a 3 to a 4
      const std::size_t idx = pman->get_ensemble().nth_specie_index( 3, 0 );
      particle::change_set exc;
      particle::change_atom a1, a2;
      a1.key = 3;
      a2.key = 4;
      a1.index = idx;
      a2.index = pman->get_ensemble().size();
      a1.old_position = pman->get_ensemble().position( idx );
      a1.do_old = true;
      a1.energy_old = 4.5;
      a1.do_new = false;
      a2.new_position = pman->get_ensemble().position( idx );
      a2.do_new = true;
      a1.energy_new = 4.5;
      a2.do_old = false;
      exc.add_atom( a1 );
      exc.add_atom( a2 );
      var->on_trial_end( exc );
      for( auto energy : dummy_trials )
      {
        particle::change_set tmp;
        tmp.id( { 0ul, 1ul, 1ul, 0ul } );
        particle::change_atom at;
        at.energy_new = energy * 0.9;
        tmp.add_atom( at );
        var->on_trial_end( tmp );
      }
  
      var->on_sample( *pman, *gman, *eman );
  
      std::stringstream out;
      const std::string canon_out { "metropolis : P_mean       = 0.49397 var = 0.132581\n           : -log(P_mean) = 0.705281 var = -2.02056\n----------------------------------------------------------------------" };
      const std::string uuid_head { "# UUID:" };
      const std::string count_head { "# SAMPLE COUNT: 3" };
      const std::string label_head { "# FIELDS: EMIN EMID EMAX P.mean P.var" };
  
      std::string canon_title { "# TITLE: \"Energy distribution histogram\"" };
      std::vector< std::string > interesting_rows {
        "0 0.005 0.01 0.333333 0.333333",
        "0.01 0.015 0.02 0.333333 0.333333",
        "0.02 0.025 0.03 0.333333 0.333333",
        "0.11 0.115 0.12 0.333333 0.333333",
        "0.13 0.135 0.14 0.333333 0.333333",
        "0.16 0.165 0.17 0.333333 0.333333",
        "0.23 0.235 0.24 0.333333 0.333333",
        "0.27 0.275 0.28 0.333333 0.333333",
        "0.31 0.315 0.32 0.333333 0.333333",
        "0.37 0.375 0.38 0.333333 0.333333",
        "0.4 0.405 0.41 0.333333 0.333333",
        "0.44 0.445 0.45 0.333333 0.333333",
        "0.51 0.515 0.52 0.333333 0.333333",
        "0.54 0.545 0.55 0.333333 0.333333",
        "0.58 0.585 0.59 0.333333 0.333333",
        "0.89 0.895 0.9 0.333333 0.333333",
        "0.9 0.905 0.91 0.333333 0.333333",
        "0.91 0.915 0.92 0.333333 0.333333",
        "1 1.005 1.01 1.33333 0.333333"
      };
      var->on_report( out, sink );
      BOOST_REQUIRE( not sink.exists( filename ) );
      sink.write_dataset();
      //std::cout << out.str() << "\n";
      BOOST_REQUIRE( sink.exists( filename ) );
      BOOST_CHECK_LT( out.str().find( canon_out ), out.str().size() );
      std::string buf; 
      sink.read( filename, buf );
      BOOST_CHECK_LT( buf.find( uuid_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( label_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( count_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_title ), buf.size() );
      for( auto interesting_row : interesting_rows )
      {
        BOOST_CHECK_LT( buf.find( interesting_row ), buf.size() );
      }
      //std::cout << buf << "\n";
    }
  }
  
  

}

BOOST_AUTO_TEST_CASE( rdf_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::rdf_sampler >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::rdf_sampler >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::rdf_sampler >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::rdf_sampler, observable::rdf_sampler >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::rdf_sampler >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::rdf_sampler::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "rdf-specie" );
  }
  {
    // static methods
    BOOST_CHECK_EQUAL( observable::rdf_sampler::default_bin_width(), 0.2 );
    BOOST_CHECK_EQUAL( observable::rdf_sampler::default_width(), 20.0 );
    BOOST_CHECK_EQUAL( observable::rdf_sampler::type_label_(), "rdf-specie" );
  }

}

BOOST_AUTO_TEST_CASE( rdf_method_test )
{
  const std::vector< std::string > labels { "CA", "CO", "OX", "Na", "Cl" };
  const std::vector< std::string > filenames { "rdf-CA-CA.dat", "rdf-CA-CO.dat", "rdf-CA-OX.dat", "rdf-CA-Na.dat", "rdf-CA-Cl.dat", "rdf-CO-CO.dat", "rdf-CO-OX.dat", "rdf-CO-Na.dat", "rdf-CO-Cl.dat", "rdf-OX-OX.dat", "rdf-OX-Na.dat", "rdf-OX-Cl.dat", "rdf-Na-Na.dat", "rdf-Na-Cl.dat", "rdf-Cl-Cl.dat" };
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    std::vector< core::input_parameter_memo > params( 3 );
    params[ 0 ].set_name( "stepsize" );
    params[ 0 ].set_value( "0.5" );
    params[ 1 ].set_name( "width" );
    params[ 1 ].set_value( "10.5" );
    params[ 2 ] = observable_test_suite::mockup_params().back();
    boost::shared_ptr< observable::base_observable > var( observable::rdf_sampler::make_sampler( params ) );
  
    observable::rdf_sampler *pvar = dynamic_cast< observable::rdf_sampler* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
    BOOST_REQUIRE_EQUAL( pvar->stepsize(), 0.5 );
    BOOST_REQUIRE_EQUAL( pvar->width(), 10.5 );
  
    var->prepare( *pman, *gman, *eman, *rman );
    BOOST_REQUIRE_EQUAL( pvar->size(), (pman->specie_count() * (pman->specie_count() + 1))/2 );
  
    // first sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
      // nothing written to output
      BOOST_REQUIRE_EQUAL( out.str().size(), 0 );
    }
    // second sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // need to add/convert a particle
    {
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 2 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 1 );
      // convert a 3 to a 4
      const std::size_t idx = pman->get_ensemble().nth_specie_index( 3, 0 );
      particle::change_set exc;
      particle::change_atom a1, a2;
      a1.key = 3;
      a2.key = 4;
      a1.index = idx;
      a2.index = pman->get_ensemble().size();
      a1.old_position = pman->get_ensemble().position( idx );
      a1.do_old = true;
      a1.do_new = false;
      a2.new_position = pman->get_ensemble().position( idx );
      a2.do_new = true;
      a2.do_old = false;
      exc.add_atom( a1 );
      exc.add_atom( a2 );
      pman->commit( exc );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 1 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 2 );
  
      var->on_sample( *pman, *gman, *eman );
    }
    {
      std::stringstream out;
      const std::string uuid_head { "# UUID:" };
      const std::string label_head { "# FIELDS: RMIN RMID RMAX P.mean P.var" };
      const std::string unit_head { "# UNITS: ANGSTROM ANGSTROM ANGSTROM Rate Rate2" };
      const std::string count_head { "# SAMPLE COUNT: 3" };
  
      std::vector< std::string > canon_titles { 
          "# TITLE: \"Radial distribution histogram for species CA and CA\"",
          "# TITLE: \"Radial distribution histogram for species CA and CO\"",
          "# TITLE: \"Radial distribution histogram for species CA and OX\"",
          "# TITLE: \"Radial distribution histogram for species CA and Na\"",
          "# TITLE: \"Radial distribution histogram for species CA and Cl\"",
          "# TITLE: \"Radial distribution histogram for species CO and CO\"",
          "# TITLE: \"Radial distribution histogram for species CO and OX\"",
          "# TITLE: \"Radial distribution histogram for species CO and Na\"",
          "# TITLE: \"Radial distribution histogram for species CO and Cl\"",
          "# TITLE: \"Radial distribution histogram for species OX and OX\"",
          "# TITLE: \"Radial distribution histogram for species OX and Na\"",
          "# TITLE: \"Radial distribution histogram for species OX and Cl\"",
          "# TITLE: \"Radial distribution histogram for species Na and Na\"",
          "# TITLE: \"Radial distribution histogram for species Na and Cl\"",
          "# TITLE: \"Radial distribution histogram for species Cl and Cl\""
      };
      std::vector< std::string > interesting_rows { 
          "0.22 0.47 0.72 0 0",
          "3.23 3.48 3.73 1 0",
          "2.73 2.98 3.23 1 0",
          "2.73 2.98 3.23 0.666667 0.333333",
          "2.81 3.06 3.31 0.333333 0.333333",
          "0.24 0.49 0.74 0 0",
          "1.74 1.99 2.24 1 0",
          "1.74 1.99 2.24 0.666667 0.333333",
          "1.82 2.07 2.32 0.333333 0.333333",
          "0.24 0.49 0.74 0 0",
          "2.74 2.99 3.24 0.666667 0.333333",
          "2.82 3.07 3.32 0.333333 0.333333 \n3.32 3.57 3.82 1 0",
          "1.74 1.99 2.24 0.666667 0.333333",
          "1.82 2.07 2.32 1 0 \n2.32 2.57 2.82 0 0 \n2.82 3.07 3.32 1 0",
          "1.9 2.15 2.4 0.333333 0.333333"
      };
      BOOST_CHECK_EQUAL( filenames.size(), canon_titles.size() );
      BOOST_CHECK_EQUAL( filenames.size(), interesting_rows.size() );
  
      var->on_report( out, sink );
      // should be no output on "out"
      BOOST_REQUIRE_EQUAL( out.str().size(), 0 );
      for( auto filename : filenames )
      {
        BOOST_REQUIRE( not sink.exists( filename ) );
      }
      sink.write_dataset();
      for( auto filename : filenames )
      {
        BOOST_REQUIRE( sink.exists( filename ) );
      }
      std::string buf;
      for( std::size_t idx = 0; idx != canon_titles.size(); ++idx )
      {
        sink.read( filenames[ idx ], buf );
        BOOST_CHECK_LT( buf.find( uuid_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( label_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( unit_head ), buf.size() );
        BOOST_CHECK_LT( buf.find( canon_titles[ idx ] ), buf.size() );
        BOOST_CHECK_LT( buf.find( interesting_rows[ idx ] ), buf.size() );
        //std::cout << buf << "\n";
      }
    }
  }

}

BOOST_AUTO_TEST_CASE( specie_count_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::specie_count >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::specie_count >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::specie_count >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::specie_count, observable::specie_count >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::specie_count >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::specie_count::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "specie-count" );
  }

}

BOOST_AUTO_TEST_CASE( specie_count_method_test )
{
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    std::vector< double > means( pman->specie_count() );
    for( std::size_t ispec = 0; ispec != pman->specie_count(); ++ispec )
    {
      means[ ispec ] = double( pman->get_ensemble().specie_count( ispec ) );
    }
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::specie_count::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable::specie_count *pvar = dynamic_cast< observable::specie_count* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
    
    var->prepare( *pman, *gman, *eman, *rman );
  
    // first sample and report
    {
      var->on_sample( *pman, *gman, *eman );
      BOOST_REQUIRE_LE( pvar->size(), pman->specie_count() );
      for( std::size_t idx = 0; idx != pvar->size(); ++idx )
      {
        const std::size_t ispec = pvar->specie_key( idx );
        BOOST_CHECK_CLOSE( pvar->mean( idx ), means[ ispec ], 0.00000001 );
        BOOST_CHECK_CLOSE( pvar->variance( idx ), 0.0, 0.00000001 );
      }
    }
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // second sample and report
    {
      var->on_sample( *pman, *gman, *eman );
      for( std::size_t idx = 0; idx != pvar->size(); ++idx )
      {
        const std::size_t ispec = pvar->specie_key( idx );
        BOOST_CHECK_CLOSE( pvar->mean( idx ), means[ ispec ], 0.00000001 );
        BOOST_CHECK_CLOSE( pvar->variance( idx ), 0.0, 0.00000001 );
      }
    }
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // need to add/convert a particle
    {
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 2 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 1 );
      // convert a 3 to a 4
      const std::size_t idx = pman->get_ensemble().nth_specie_index( 3, 0 );
      particle::change_set exc;
      particle::change_atom a1, a2;
      a1.key = 3;
      a2.key = 4;
      a1.index = idx;
      a2.index = pman->get_ensemble().size();
      a1.old_position = pman->get_ensemble().position( idx );
      a1.do_old = true;
      a1.do_new = false;
      a2.new_position = pman->get_ensemble().position( idx );
      a2.do_new = true;
      a2.do_old = false;
      exc.add_atom( a1 );
      exc.add_atom( a2 );
      pman->commit( exc );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 1 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 2 );
  
      var->on_sample( *pman, *gman, *eman );
  
      means[ 3 ] = 1;
      means[ 4 ] = 2;
      for( std::size_t idx = 0; idx != pvar->size(); ++idx )
      {
        const std::size_t ispec = pvar->specie_key( idx );
        BOOST_CHECK_CLOSE( pvar->mean( idx ), means[ ispec ], 0.00000001 );
        BOOST_CHECK_CLOSE( pvar->variance( idx ), 0.0, 0.00000001 );
      }
    }
    {
      std::stringstream out;
      const std::string canon_log_title = " SPC    <COUNT>        VAR       <[]>";
      const std::string canon_log_s3 = "   3    1.00000     0.0000  0.0752768";
      const std::string canon_log_s4 = "   4    2.00000     0.0000  0.1505535";
      const std::string canon_title = "# TITLE: \"Time series specie counts.\"";
      const std::string canon_labels = "# FIELDS: INDEX SAMPLES SPC SPC.VOL Count.mean Count.var";
      const std::string canon_units = "# UNITS: ORDINAL ORDINAL INDEX ANGSTROM3 Rate Rate2";
      const std::string canon_rep = "0 1 3 125 2 0  4 125 1 0  \n1 1 3 125 2 0  4 125 1 0  \n2 1 3 125 1 0  4 125 2 0";
      var->on_report( out, sink );
      //std::cout << out.str() << "\n";
      BOOST_CHECK_LT( out.str().find( canon_log_title ), out.str().size() );
      BOOST_CHECK_LT( out.str().find( canon_log_s3 ), out.str().size() );
      BOOST_CHECK_LT( out.str().find( canon_log_s4 ), out.str().size() );
      BOOST_REQUIRE( not sink.exists( "specie-count.dat" ) );
      sink.write_dataset();
      BOOST_REQUIRE( sink.exists( "specie-count.dat" ) );
      std::string buf;
      sink.read( "specie-count.dat", buf );
      BOOST_CHECK_LT( buf.find( canon_title ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_labels ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_units ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_rep ), buf.size() );
      //std::cout << buf << "\n";
    }
  }

}

BOOST_AUTO_TEST_CASE( specie_region_count_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::specie_region_count >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::specie_region_count >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::specie_region_count >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::specie_region_count, observable::specie_region_count >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::specie_region_count >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::specie_region_count::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "specie-region-count" );
  }

}

BOOST_AUTO_TEST_CASE( specie_region_count_method_test )
{
  const std::string filename( "specie-region-count.dat" );
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    std::vector< double > means( pman->specie_count() );
    std::vector< double > vars( pman->specie_count(), 0.0 );
    for( std::size_t ispec = 0; ispec != pman->specie_count(); ++ispec )
    {
      means[ ispec ] = double( pman->get_ensemble().specie_count( ispec ) );
    }
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::specie_region_count::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable::specie_region_count *pvar = dynamic_cast< observable::specie_region_count* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
  
    var->prepare( *pman, *gman, *eman, *rman );
    BOOST_REQUIRE_EQUAL( pvar->size(), pman->specie_count() * gman->region_count() );
  
    // first sample and report
    {
      var->on_sample( *pman, *gman, *eman );
      for( std::size_t ireg = 0; ireg != gman->region_count(); ++ireg )
      {
        const std::size_t offset = ireg * pman->specie_count();
        for( std::size_t ispec = 0; ispec != pman->specie_count(); ++ispec )
        {
          BOOST_CHECK_CLOSE( pvar->mean( offset + ispec ), means[ ispec ], 0.00000001 );
          BOOST_CHECK_CLOSE( pvar->variance( offset + ispec ), vars[ ispec ], 0.00000001 );
        }
      }
    }
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // second sample and report
    {
      var->on_sample( *pman, *gman, *eman );
      for( std::size_t ireg = 0; ireg != gman->region_count(); ++ireg )
      {
        const std::size_t offset = ireg * pman->specie_count();
        for( std::size_t ispec = 0; ispec != pman->specie_count(); ++ispec )
        {
          BOOST_CHECK_CLOSE( pvar->mean( offset + ispec ), means[ ispec ], 0.00000001 );
          BOOST_CHECK_CLOSE( pvar->variance( offset + ispec ), vars[ ispec ], 0.00000001 );
        }
      }
    }
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // need to add/convert a particle
    {
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 2 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 1 );
      // convert a 3 to a 4
      const std::size_t idx = pman->get_ensemble().nth_specie_index( 3, 0 );
      particle::change_set exc;
      particle::change_atom a1, a2;
      a1.key = 3;
      a2.key = 4;
      a1.index = idx;
      a2.index = pman->get_ensemble().size();
      a1.old_position = pman->get_ensemble().position( idx );
      a1.do_old = true;
      a1.do_new = false;
      a2.new_position = pman->get_ensemble().position( idx );
      a2.do_new = true;
      a2.do_old = false;
      exc.add_atom( a1 );
      exc.add_atom( a2 );
      pman->commit( exc );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 1 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 2 );
  
      var->on_sample( *pman, *gman, *eman );
  
      means[ 3 ] = 1;
      means[ 4 ] = 2;
      vars[ 3 ] = 0;
      vars[ 4 ] = 0;
      for( std::size_t ireg = 0; ireg != gman->region_count(); ++ireg )
      {
        const std::size_t offset = ireg * pman->specie_count();
        for( std::size_t ispec = 0; ispec != pman->specie_count(); ++ispec )
        {
          BOOST_CHECK_CLOSE( pvar->mean( offset + ispec ), means[ ispec ], 0.00000001 );
          BOOST_CHECK_CLOSE( pvar->variance( offset + ispec ), vars[ ispec ], 0.00000001 );
        }
      }
    }
    {
      std::stringstream out;
      const std::string canon_log = "REG SPC    <COUNT>        VAR       <[]>\n   0   0    1.00000     0.0000  0.0752768\n   0   1    1.00000     0.0000  0.0752768\n   0   2    1.00000     0.0000  0.0752768\n   0   3    1.00000     0.0000  0.0752768\n   0   4    2.00000     0.0000  0.1505535\n   1   0    1.00000     0.0000  0.0385417\n   1   1    1.00000     0.0000  0.0385417\n   1   2    1.00000     0.0000  0.0385417\n   1   3    1.00000     0.0000  0.0385417\n   1   4    2.00000     0.0000  0.0770834";
  
      const std::string canon_title = "# TITLE: \"Time series specie / region counts.\"";
      const std::string canon_labels = "# FIELDS: INDEX SAMPLES REG:SPC REG:SPC:VOL Count.mean Count.var";
      const std::string canon_units = "# UNITS: ORDINAL ORDINAL INDEX ANGSTROM3 Rate Rate2";
      std::string canon_rep1 = "0 1 0:0 125 1 0  0:1 125 1 0  0:2 125 1 0  0:3 125 2 0  0:4 125 1 0  1:0 64 1 0  1:1 64 1 0  1:2 64 1 0  1:3 64 2 0  1:4 64 1 0";
      std::string canon_rep2 = "1 1 0:0 125 1 0  0:1 125 1 0  0:2 125 1 0  0:3 125 2 0  0:4 125 1 0  1:0 64 1 0  1:1 64 1 0  1:2 64 1 0  1:3 64 2 0  1:4 64 1 0";
      std::string canon_rep3 = "2 1 0:0 125 1 0  0:1 125 1 0  0:2 125 1 0  0:3 125 1 0  0:4 125 2 0  1:0 64 1 0  1:1 64 1 0  1:2 64 1 0  1:3 64 1 0  1:4 64 2 0";
  
      var->on_report( out, sink );
      //std::cout << out.str() << "\n";
      BOOST_CHECK_LT( out.str().find( canon_log ), out.str().size() );
  
      BOOST_REQUIRE( not sink.exists( filename ) );
      sink.write_dataset();
      BOOST_REQUIRE( sink.exists( filename ) );
      std::string buf;
      sink.read( filename, buf );
      BOOST_CHECK_LT( buf.find( canon_title ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_labels ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_units ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_rep1 ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_rep2 ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_rep3 ), buf.size() );
      //std::cout << buf << "\n";
    }
  }
  

}

BOOST_AUTO_TEST_CASE( trial_observer_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::trial_observer >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::trial_observer >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::trial_observer >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::trial_observer, observable::trial_observer >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::trial_observer >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::trial_observer::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "trial-log" );
  }
  {
    // static methods
    BOOST_CHECK_EQUAL( observable::trial_observer::type_label_(), "trial-log" );
  }

}

BOOST_AUTO_TEST_CASE( trial_observer_method_test )
{
  const std::string filename { "trial-log.dat" };
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    boost::shared_ptr< observable::tracked_observable > var( observable::trial_observer::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable::trial_observer *pvar = dynamic_cast< observable::trial_observer* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
  
    var->prepare( *pman, *gman, *eman, *rman );
  
    std::vector< std::tuple< particle::change_hash, double, double, double, bool, bool > > dummy_trials;
    // ID, E[0], PROB[1], EXP[0], FAIL, ACCEPT
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 0, 1, 1, 0 ), 0.1, 0.5, 1.0, false, true ) ); // A 1 0
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 1, 1, 1, 0 ), 0.2, 1, 0.0, false, false ) ); // B 0 1
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 0, 1, 1, 1 ), 0.3, 1, 0.0, false, true ) ); // C 1 0
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 1, 1, 1, 0 ), 0.4, 1, 0.0, false, false ) ); // B 0 0
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 0, 1, 1, 1 ), 0.5, 2.5, -1.0, false, true ) ); // C 1 1
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 0, 1, 1, 0 ), 0.6, 1, 0, false, false ) ); // A 0 0
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 0, 1, 1, 0 ), 0.7, 0.5, 1.0, false, true ) ); // A 1 0
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 0, 1, 1, 1 ), 0.8, 1, 0, false, false ) ); // C 0 1
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 1, 1, 1, 0 ), 0.9, 1, 0, false, true ) ); // B 1 0
    dummy_trials.push_back( std::make_tuple( particle::change_hash( 1, 1, 1, 0 ), 0.1, 1, 0, true, false ) ); // B 0 0
    // Out first set of dummy trials
    {
      std::size_t counter = 0;
      for( auto id : dummy_trials )
      {
        particle::change_set tmp;
        tmp.id( std::get< 0 >( id ) );
        particle::change_atom atm;
        atm.energy_new = std::get< 1 >( id );
        tmp.add_atom( atm );
        tmp.update_probability_factor( std::get< 2 >( id ) );
        tmp.update_exponential_factor( std::get< 3 >( id ) );
        if( std::get< 4 >( id ) )
        {
          tmp.set_fail();
          tmp.set_accept( false );
        }
        else
        {
          tmp.set_accept( std::get< 5 >( id ) );
        }
  
        var->on_trial_end( tmp );
        ++counter;
      }
    }
  
    // first sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      BOOST_REQUIRE( not sink.exists( filename ) );
      std::stringstream out;
      var->on_report( out, sink );
      // nothing written to output
      BOOST_REQUIRE_EQUAL( out.str().size(), 0 );
      sink.write_dataset();
      BOOST_REQUIRE( sink.exists( filename ) );
      std::string buf;
      sink.read( filename, buf );
      const std::string ulog { "# UUID: " };
      const std::string blog_title { "# TITLE: \"Record of each trial\"" };
      const std::string blog_labels { "# FIELDS: CHANGE_ID ENERGY PROB_FACTOR EXP_FACTOR FAIL ACCEPT" };
      const std::string blog_units { "# UNITS: (A,B,C,D) ENERGY NUMBER NUMBER BOOLEAN BOOLEAN" };
      const std::string blog1 { "(0, 1, 1, 0) 0.1 0.5 1 0 1" };
      const std::string blog2 { "(1, 1, 1, 0) 0.2 1 0 0 0" };
      const std::string blog3 { "(0, 1, 1, 1) 0.3 1 0 0 1" };
      const std::string blog4 { "(1, 1, 1, 0) 0.4 1 0 0 0" };
      const std::string blog5 { "(0, 1, 1, 1) 0.5 2.5 -1 0 1" };
      const std::string blog6 { "(0, 1, 1, 0) 0.6 1 0 0 0" };
      const std::string blog7 { "(0, 1, 1, 0) 0.7 0.5 1 0 1" };
      const std::string blog8 { "(0, 1, 1, 1) 0.8 1 0 0 0" };
      const std::string blog9 { "(1, 1, 1, 0) 0.9 1 0 0 1" };
      const std::string blogA { "(1, 1, 1, 0) 0.1 1 0 1 0" };
   
      BOOST_CHECK( buf.find( ulog ) != std::string::npos );
      BOOST_CHECK( buf.find( blog_title ) != std::string::npos );
      BOOST_CHECK( buf.find( blog_labels ) != std::string::npos );
      BOOST_CHECK( buf.find( blog_units ) != std::string::npos );
      BOOST_CHECK( buf.find( blog1 ) != std::string::npos );
      BOOST_CHECK( buf.find( blog2 ) != std::string::npos );
      BOOST_CHECK( buf.find( blog3 ) != std::string::npos );
      BOOST_CHECK( buf.find( blog4 ) != std::string::npos );
      BOOST_CHECK( buf.find( blog5 ) != std::string::npos );
      BOOST_CHECK( buf.find( blog6 ) != std::string::npos );
      BOOST_CHECK( buf.find( blog7 ) != std::string::npos );
      BOOST_CHECK( buf.find( blog8 ) != std::string::npos );
      BOOST_CHECK( buf.find( blog9 ) != std::string::npos );
      BOOST_CHECK( buf.find( blogA ) != std::string::npos );
      //std::cout << buf << "\n";
    }
  }

}

BOOST_AUTO_TEST_CASE( widom_lifetime_test )
{
  {
    // Static Lifetime method tests: virtual pattern
  
    BOOST_CHECK( not std::is_default_constructible< observable::widom >::type{} );
    BOOST_CHECK( not std::is_copy_constructible< observable::widom >::type{} );
    BOOST_CHECK( not std::is_move_constructible< observable::widom >::type{} );
    BOOST_CHECK( ( not std::is_assignable< observable::widom, observable::widom >::type{} ) );
    BOOST_CHECK( std::has_virtual_destructor< observable::widom >::type{} );
  }
  {
    // factory ctor
    boost::shared_ptr< observable::base_observable > var( observable::widom::make_sampler( observable_test_suite::mockup_params() ) );
  
    observable_test_suite::base_observable_method_test( var, "widom" );
  }
  {
    // static methods
    BOOST_CHECK_EQUAL( observable::widom::type_label_(), "widom" );
  }

}

// Note: energy estimates are zero. Not sure if this is due to too few particles in
// the simulation giving zero energies.
BOOST_AUTO_TEST_CASE( widom_method_test )
{
  const std::vector< std::string > labels { "CA", "CO", "OX", "Na", "Cl" };
  const std::string filename { "widom.dat" };
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    std::vector< core::input_parameter_memo > params( 2 );
    params[ 0 ].set_name( "iwidom" );
    params[ 0 ].set_value( "10000" );
    params[ 1 ] = observable_test_suite::mockup_params().back();
    boost::shared_ptr< observable::base_observable > var( observable::widom::make_sampler( params ) );
  
    observable::widom *pvar = dynamic_cast< observable::widom* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
    BOOST_REQUIRE_EQUAL( pvar->loop_count(), 0ul );
    BOOST_REQUIRE_EQUAL( pvar->trials(), 10000ul );
  
    var->prepare( *pman, *gman, *eman, *rman );
    eman->prepare( *pman, *gman );
    BOOST_REQUIRE_EQUAL( pvar->size(), 2ul );
  
    // first sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // second sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // need to add/convert a particle
    {
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 2 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 1 );
      // convert a 3 to a 4
      const std::size_t idx = pman->get_ensemble().nth_specie_index( 3, 0 );
      particle::change_set exc;
      particle::change_atom a1, a2;
      a1.key = 3;
      a2.key = 4;
      a1.index = idx;
      a2.index = pman->get_ensemble().size();
      a1.old_position = pman->get_ensemble().position( idx );
      a1.do_old = true;
      a1.do_new = false;
      a2.new_position = pman->get_ensemble().position( idx );
      a2.do_new = true;
      a2.do_old = false;
      exc.add_atom( a1 );
      exc.add_atom( a2 );
      pman->commit( exc );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 1 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 2 );
  
      var->on_sample( *pman, *gman, *eman );
    }
    {
      std::stringstream out;
      const std::string out_title { " Results widom test particle method :" };
      const std::string out_head { "SPECIE    SAMPLES    <U> (E)     <U>VAR     EXCESS" };
      const std::string out_row1 { "CP  Na      10000" };
      const std::string out_row2 { "CP  Cl      10000" };
      const std::string uuid_head { "# UUID:" };
      const std::string label_head { "# FIELDS: INDEX Na_EXCESS0 Na_POTENTIAL Na_EXCESS Na_COUNT Cl_EXCESS0 Cl_POTENTIAL Cl_EXCESS Cl_COUNT" };
      const std::string units_head { "# UNITS: ORDINAL ENERGY ENERGY ENERGY COUNT ENERGY ENERGY ENERGY COUNT" };
      const std::string canon_title { "# TITLE: \"Series of Widom method chemical potential estimates.\"" };
      const std::string interesting_row { "3 0.123 -25.3801 -27.9667 10000 0.3123 -6.95822 -10.2379 10000" };
  
      var->on_report( out, sink );
      // should be output on "out"
      {
        const std::string outstr { out.str() };
        //std::cout << outstr << "\n";
        BOOST_CHECK_LT( outstr.find( out_title ), outstr.size() );
        BOOST_CHECK_LT( outstr.find( out_head ), outstr.size() );
        BOOST_CHECK_LT( outstr.find( out_row1 ), outstr.size() );
        BOOST_CHECK_LT( outstr.find( out_row2 ), outstr.size() );
      }
      BOOST_REQUIRE( not sink.exists( filename ) );
      sink.write_dataset();
      BOOST_REQUIRE( sink.exists( filename ) );
      std::string buf;
      sink.read( filename, buf );
      //std::cout << buf << "\n";
      BOOST_CHECK_LT( buf.find( uuid_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( label_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( units_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_title ), buf.size() );
      BOOST_CHECK_LT( buf.find( interesting_row ), buf.size() );
    }
  }

}

// Note: energy estimates are zero. Not sure if this is due to too few particles in
// the simulation giving zero energies.
BOOST_AUTO_TEST_CASE( widom_specie_list_test )
{
  const std::vector< std::string > labels { "CA", "CO", "OX", "Na", "Cl" };
  const std::string filename { "widom.dat" };
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    std::vector< core::input_parameter_memo > params( 3 );
    params[ 0 ].set_name( "iwidom" );
    params[ 0 ].set_value( "10000" );
    params[ 1 ].set_name( "specie" );
    params[ 1 ].set_value( "Na" );
    params[ 2 ] = observable_test_suite::mockup_params().back();
    boost::shared_ptr< observable::base_observable > var( observable::widom::make_sampler( params ) );
  
    observable::widom *pvar = dynamic_cast< observable::widom* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
    BOOST_REQUIRE_EQUAL( pvar->loop_count(), 0ul );
    BOOST_REQUIRE_EQUAL( pvar->trials(), 10000ul );
    BOOST_REQUIRE_EQUAL( pvar->specie_list(), "Na" );
  
    var->prepare( *pman, *gman, *eman, *rman );
    eman->prepare( *pman, *gman );
    BOOST_REQUIRE_EQUAL( pvar->size(), 1ul );
  
    // first sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // second sample and report
    var->on_sample( *pman, *gman, *eman );
    {
      std::stringstream out;
      var->on_report( out, sink );
    }
    // need to add/convert a particle
    {
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 2 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 1 );
      // convert a 3 to a 4
      const std::size_t idx = pman->get_ensemble().nth_specie_index( 3, 0 );
      particle::change_set exc;
      particle::change_atom a1, a2;
      a1.key = 3;
      a2.key = 4;
      a1.index = idx;
      a2.index = pman->get_ensemble().size();
      a1.old_position = pman->get_ensemble().position( idx );
      a1.do_old = true;
      a1.do_new = false;
      a2.new_position = pman->get_ensemble().position( idx );
      a2.do_new = true;
      a2.do_old = false;
      exc.add_atom( a1 );
      exc.add_atom( a2 );
      pman->commit( exc );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 3 ), 1 );
      BOOST_REQUIRE_EQUAL( pman->get_ensemble().specie_count( 4 ), 2 );
  
      var->on_sample( *pman, *gman, *eman );
    }
    {
      std::stringstream out;
      const std::string uuid_head { "# UUID:" };
      const std::string label_head { "# FIELDS: INDEX Na_EXCESS0 Na_POTENTIAL Na_EXCESS Na_COUNT" };
      const std::string units_head { "# UNITS: ORDINAL ENERGY ENERGY ENERGY COUNT" };
  
      const std::string canon_title { "# TITLE: \"Series of Widom method chemical potential estimates.\"" };
      const std::string interesting_row { "3 0.123 -24.1507 -26.7372 10000" };
  
      var->on_report( out, sink );
      // Output checked in widom_method_test
      //std::cout << out.str() << "\n";
      BOOST_REQUIRE( not sink.exists( filename ) );
      sink.write_dataset();
      BOOST_REQUIRE( sink.exists( filename ) );
      std::string buf;
      sink.read( filename, buf );
      //std::cout << buf << "\n";
      BOOST_CHECK_LT( buf.find( uuid_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( label_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( units_head ), buf.size() );
      BOOST_CHECK_LT( buf.find( canon_title ), buf.size() );
      BOOST_CHECK_LT( buf.find( interesting_row ), buf.size() );
    }
  }

}

// Note: energy estimates are zero. Not sure if this is due to too few particles in
// the simulation giving zero energies.
BOOST_AUTO_TEST_CASE( widom_specie_list_unknown_specie_value_test )
{
  const std::vector< std::string > labels { "CA", "CO", "OX", "Na", "Cl" };
  const std::string filename { "widom.dat" };
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    std::vector< core::input_parameter_memo > params( 3 );
    params[ 0 ].set_name( "iwidom" );
    params[ 0 ].set_value( "10000" );
    params[ 1 ].set_name( "specie" );
    params[ 1 ].set_value( "Ca:bulk" );
    params[ 2 ] = observable_test_suite::mockup_params().back();
    boost::shared_ptr< observable::base_observable > var( observable::widom::make_sampler( params ) );
  
    observable::widom *pvar = dynamic_cast< observable::widom* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
    BOOST_REQUIRE_EQUAL( pvar->loop_count(), 0ul );
    BOOST_REQUIRE_EQUAL( pvar->trials(), 10000ul );
    BOOST_REQUIRE_EQUAL( pvar->specie_list(), "Ca:bulk" );
  
    try
    {
      var->prepare( *pman, *gman, *eman, *rman );
      BOOST_ERROR( "expected \"var->prepare(...)\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Specie list parameter \"specie\" with value (Ca:bulk) in section \"sampler\".\n** Specie label \"Ca\" must match one of the defined species. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"var->prepare(...)\" was not expected type: " ) + err.what() );
    }
  }

}

// Note: energy estimates are zero. Not sure if this is due to too few particles in
// the simulation giving zero energies.
BOOST_AUTO_TEST_CASE( widom_specie_list_bad_specie_value_test )
{
  const std::vector< std::string > labels { "CA", "CO", "OX", "Na", "Cl" };
  const std::string filename { "widom.dat" };
  {
    boost::shared_ptr< particle::particle_manager > pman = observable_test_suite::mockup_particle_manager();
    boost::shared_ptr< evaluator::evaluator_manager > eman = observable_test_suite::mockup_evaluator_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = observable_test_suite::mockup_geometry_manager();
    boost::shared_ptr< observable::report_manager > rman( new observable::report_manager );
    observable::memory_sink sink;
    // factory ctor
    std::vector< core::input_parameter_memo > params( 3 );
    params[ 0 ].set_name( "iwidom" );
    params[ 0 ].set_value( "10000" );
    params[ 1 ].set_name( "specie" );
    params[ 1 ].set_value( "CA" );
    params[ 2 ] = observable_test_suite::mockup_params().back();
  
    boost::shared_ptr< observable::base_observable > var( observable::widom::make_sampler( params ) );
  
    observable::widom *pvar = dynamic_cast< observable::widom* >( var.get() );
  
    BOOST_REQUIRE( pvar != nullptr );
    BOOST_REQUIRE_EQUAL( pvar->loop_count(), 0ul );
    BOOST_REQUIRE_EQUAL( pvar->trials(), 10000ul );
    BOOST_REQUIRE_EQUAL( pvar->specie_list(), "CA" );
  
    try
    {
      var->prepare( *pman, *gman, *eman, *rman );
      BOOST_ERROR( "expected \"var->prepare(...)\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Specie list parameter \"specie\" with value (CA) in section \"sampler\".\n** Specie \"CA\" must be a solute type specie. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"var->prepare(...)\" was not expected type: " ) + err.what() );
    }
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_unknown_sampler_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::acceptance_observable::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\ntype freeze\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Sampler subtype parameter \"type\" with value (freeze) in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>type freeze<<\n** A value from the list (trial-rate) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Attempt to insert two definitions with the same label
BOOST_AUTO_TEST_CASE( sampler_input_acceptance_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::acceptance_observable::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "trial-rate" ) );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\ntype trial-rate\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_CHECK( oman->has_tracked( "trial-rate" ) );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
  }

}

// Input file with unknown parameter type
BOOST_AUTO_TEST_CASE( sampler_input_acceptance_with_param_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::acceptance_observable::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\ntype trial-rate\nwidth 0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"width\" in section \"sampler\" subtype \"trial-rate\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n** Parameter \"width\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown parameter type
BOOST_AUTO_TEST_CASE( sampler_input_acceptance_duplicate_type )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::acceptance_observable::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\ntype trial-rate\ntype trial-rate\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"type\" appears more than once in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>type trial-rate<<\n** Parameter \"type\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown parameter type
BOOST_AUTO_TEST_CASE( sampler_input_acceptance_duplicate_invalid_type )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::acceptance_observable::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\ntype trial-rate\ntype accept\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"type\" appears more than once in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>type accept<<\n** Parameter \"type\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// 
BOOST_AUTO_TEST_CASE( sampler_input_density_zaxis_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::density_zaxis::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "population-zaxis" ) );
    dg.add_input_delegate( m );
  }
  // valid
  ////////
  std::string canon_input( "sampler\ntype population-zaxis\nstepsize 0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_CHECK( oman->has_sample( "population-zaxis" ) );
    auto obs = oman->get_sample( "population-zaxis" );
    BOOST_CHECK( obs );
    observable::density_zaxis *pobs = dynamic_cast< observable::density_zaxis* >( obs.get() );
    BOOST_REQUIRE( pobs != nullptr );
    BOOST_CHECK_CLOSE( pobs->stepsize(), 0.5, 0.00000000001 );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
  }

}

// 
BOOST_AUTO_TEST_CASE( sampler_input_density_zaxis_default_stepsize_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::density_zaxis::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "population-zaxis" ) );
    dg.add_input_delegate( m );
  }
  // valid
  ////////
  std::string canon_input( "sampler\ntype population-zaxis\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_CHECK( oman->has_sample( "population-zaxis" ) );
    auto obs = oman->get_sample( "population-zaxis" );
    BOOST_CHECK( obs );
    observable::density_zaxis *pobs = dynamic_cast< observable::density_zaxis* >( obs.get() );
    BOOST_REQUIRE( pobs != nullptr );
    BOOST_CHECK_CLOSE( pobs->stepsize(), observable::density_zaxis::default_stepsize(), 0.00000000001 );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
  }

}

BOOST_AUTO_TEST_CASE( sampler_input_density_zaxis_bad_param_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::density_zaxis::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown parameter
  //////////////////////////////
  std::string canon_input( "sampler\ntype population-zaxis\nstep-size 0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"step-size\" in section \"sampler\" subtype \"population-zaxis\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n** Parameter \"step-size\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( sampler_input_density_zaxis_param_no_value_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::density_zaxis::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown parameter
  //////////////////////////////
  std::string canon_input( "sampler\ntype population-zaxis\nstepsize# 0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "population-zaxis parameter \"stepsize\" in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>stepsize# 0.5<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( sampler_input_density_zaxis_param_bad_value_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::density_zaxis::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown parameter
  //////////////////////////////
  std::string canon_input( "sampler\ntype population-zaxis\nstepsize half\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "population-zaxis parameter \"stepsize\" with value (half) in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>stepsize half<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// 
BOOST_AUTO_TEST_CASE( sampler_input_d3_distribution_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::d3_distribution::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "3d-distribution" ) );
    dg.add_input_delegate( m );
  }
  // valid
  ////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype 3d-distribution\nstepsize 0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_CHECK( oman->has_sample( "3d-distribution" ) );
    auto obs = oman->get_sample( "3d-distribution" );
    BOOST_CHECK( obs );
    observable::d3_distribution *pobs = dynamic_cast< observable::d3_distribution* >( obs.get() );
    BOOST_REQUIRE( pobs != nullptr );
    BOOST_CHECK_CLOSE( pobs->stepsize(), 0.5, 0.00000000001 );
    BOOST_CHECK_EQUAL( pobs->region_map().size(), 1ul );
    BOOST_CHECK( pobs->region_map().find( "Ca" ) != pobs->region_map().end() );
    BOOST_CHECK_EQUAL( pobs->region_map().find( "Ca" )->second, "bulk" );
  }
  catch( std::exception const& err )
  {
    const std::string msg( err.what() );
    std::cout << msg << "\n";
    BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_d3_bad_param )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::d3_distribution::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype 3d-distribution\nstepsize 0.5\nwave 0.4\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"wave\" in section \"sampler\" subtype \"3d-distribution\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n** Parameter \"wave\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_d3_stepsize_no_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::d3_distribution::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype 3d-distribution\nstepsize #0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "3d-distribution parameter \"stepsize\" in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>stepsize #0.5<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_d3_stepsize_bad_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::d3_distribution::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype 3d-distribution\nstepsize half\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "3d-distribution parameter \"stepsize\" with value (half) in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>stepsize half<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_d3_stepsize_duplicate )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::d3_distribution::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype 3d-distribution\nstepsize 0.5\nstepsize 1.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"stepsize\" appears more than once in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>stepsize 1.5<<\n** Parameter \"stepsize\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_d3_region_no_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::d3_distribution::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion \ntype 3d-distribution\nstepsize 0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "3D distribution 3d-distribution parameter \"region\" in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>region <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Tested malformed list, now need to test what happens when list is non-empty
// string but an empty list, eg. " : ";
BOOST_AUTO_TEST_CASE( sampler_input_d3_region_bad_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::d3_distribution::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion \" : \"\ntype 3d-distribution\nstepsize 0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Volume selection parameter \"region\" with value (\" : \") in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>region \" : \"<<\n** Unable to convert value into a list of region labels. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_d3_region_duplicate )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::d3_distribution::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype 3d-distribution\nstepsize 0.5\nregion Na cell\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"region\" appears more than once in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>region Na cell<<\n** Parameter \"region\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// 
BOOST_AUTO_TEST_CASE( sampler_input_metropolis_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::metropolis_sampler::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "metropolis" ) );
    dg.add_input_delegate( m );
  }
  // valid
  ////////
  std::string canon_input( "sampler\ntype metropolis\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_REQUIRE_EQUAL( oman->size(), 1ul );
    BOOST_REQUIRE( oman->has_tracked( "metropolis" ) );
    auto obs = oman->get_tracked( "metropolis" );
    BOOST_CHECK( obs );
    observable::metropolis_sampler *pobs = dynamic_cast< observable::metropolis_sampler* >( obs.get() );
    BOOST_REQUIRE( pobs != nullptr );
    BOOST_CHECK_EQUAL( pobs->boltzmann_factor().count(), 0ul );
    BOOST_CHECK_EQUAL( pobs->energy_change().count(), 0ul );
    BOOST_CHECK_EQUAL( pobs->energy().count(), 0ul );
  }
  catch( std::exception const& err )
  {
    const std::string msg( err.what() );
    std::cout << msg << "\n";
    BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_metropolis_bad_param )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::metropolis_sampler::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype metropolis\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"region\" in section \"sampler\" subtype \"metropolis\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n** Parameter \"region\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// 
BOOST_AUTO_TEST_CASE( sampler_input_rdf_sampler_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::rdf_sampler::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "rdf-specie" ) );
    dg.add_input_delegate( m );
  }
  // valid
  ////////
  std::string canon_input( "sampler\nwidth 6.0\ntype rdf-specie\nstepsize 0.5\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_CHECK( oman->has_sample( "rdf-specie" ) );
    auto obs = oman->get_sample( "rdf-specie" );
    BOOST_CHECK( obs );
    observable::rdf_sampler *pobs = dynamic_cast< observable::rdf_sampler* >( obs.get() );
    BOOST_REQUIRE( pobs != nullptr );
    BOOST_CHECK_CLOSE( pobs->stepsize(), 0.5, 0.00000000001 );
    BOOST_CHECK_CLOSE( pobs->width(), 6.0, 0.0000000001 );
  }
  catch( std::exception const& err )
  {
    const std::string msg( err.what() );
    std::cout << msg << "\n";
    BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_rdf_sampler_bad_param )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::rdf_sampler::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype rdf-specie\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"region\" in section \"sampler\" subtype \"rdf-specie\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n** Parameter \"region\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_rdf_sampler_width_no_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::rdf_sampler::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nwidth #Ca:bulk\ntype rdf-specie\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Radial Distribution function total width parameter \"width\" in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>width #Ca:bulk<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_rdf_sampler_width_bad_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::rdf_sampler::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nwidth Ca:bulk\ntype rdf-specie\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Radial Distribution function total width parameter \"width\" with value (Ca:bulk) in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>width Ca:bulk<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_rdf_sampler_stepsize_no_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::rdf_sampler::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nstepsize #Ca:bulk\ntype rdf-specie\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Radial Distribution histgram bin width parameter \"stepsize\" in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>stepsize #Ca:bulk<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_rdf_sampler_stepsize_bad_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::rdf_sampler::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nstepsize Ca:bulk\ntype rdf-specie\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Radial Distribution histgram bin width parameter \"stepsize\" with value (Ca:bulk) in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>stepsize Ca:bulk<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// 
BOOST_AUTO_TEST_CASE( sampler_input_specie_count_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::specie_count::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "specie-count" ) );
    dg.add_input_delegate( m );
  }
  // valid
  ////////
  std::string canon_input( "sampler\ntype specie-count\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_CHECK( oman->has_sample( "specie-count" ) );
    auto obs = oman->get_sample( "specie-count" );
    BOOST_CHECK( obs );
    observable::specie_count *pobs = dynamic_cast< observable::specie_count* >( obs.get() );
    BOOST_REQUIRE( pobs != nullptr );
  }
  catch( std::exception const& err )
  {
    const std::string msg( err.what() );
    std::cout << msg << "\n";
    BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_specie_count_bad_param )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::specie_count::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype specie-count\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"region\" in section \"sampler\" subtype \"specie-count\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n** Parameter \"region\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// 
BOOST_AUTO_TEST_CASE( sampler_input_widom_test )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::widom::add_definition( *m );
    BOOST_CHECK( not m->empty() );
    BOOST_CHECK( m->has_type( "widom" ) );
    dg.add_input_delegate( m );
  }
  // valid
  ////////
  std::string canon_input( "sampler\niwidom 100\ntype widom\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_REQUIRE( oman->has_tracked( "widom" ) );
    auto obs = oman->get_tracked( "widom" );
    BOOST_REQUIRE( obs );
    observable::widom *pobs = dynamic_cast< observable::widom* >( obs.get() );
    BOOST_REQUIRE( pobs != nullptr );
    BOOST_CHECK_EQUAL( pobs->trials(), 100ul );
    //BOOST_CHECK_CLOSE( pobs->width(), 6.0, 0.0000000001 );
  }
  catch( std::exception const& err )
  {
    const std::string msg( err.what() );
    std::cout << msg << "\n";
    BOOST_ERROR( "expected no exception from \"dg.read_input( reader )\"" );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_widom_bad_param )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::widom::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nregion Ca:bulk\ntype widom\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"region\" in section \"sampler\" subtype \"widom\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n** Parameter \"region\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_widom_iwidom_no_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::widom::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\niwidom #Ca:bulk\ntype widom\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Widom insertion counts/cycle parameter \"iwidom\" in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>iwidom #Ca:bulk<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_widom_iwidom_bad_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::widom::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\niwidom Ca:bulk\ntype widom\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Widom insertion counts/cycle parameter \"iwidom\" with value (Ca:bulk) in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>iwidom Ca:bulk<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Input file with unknown sampler type
BOOST_AUTO_TEST_CASE( sampler_input_widom_specie_no_value )
{
  boost::shared_ptr< observable::report_manager > oman( new observable::report_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< observable::sampler_meta > m( new observable::sampler_meta( oman ) );
    observable::widom::add_definition( *m );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "sampler\nspecie #Ca:bulk\ntype widom\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Specie label list parameter \"specie\" in section \"sampler\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>specie #Ca:bulk<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

