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

#define BOOST_TEST_MODULE geometry_test
#include <boost/test/unit_test.hpp>

#include "geometry/geometry_test/geometry_test.hpp"
#include "geometry/centroid.hpp"
#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"
#include "geometry/cube_region.hpp"
#include "geometry/cubic_grid.hpp"
#include "geometry/cylinder_region.hpp"
#include "geometry/digitizer_3d.hpp"
#include "utility/fuzzy_equals.hpp"
#include "geometry/geometry_manager.hpp"
#include "geometry/membrane_cell_inserter.hpp"
#include "geometry/open_cylinder_region.hpp"
#include "geometry/open_split_cylinder_region.hpp"
#include "geometry/periodic_cube_region.hpp"
#include "geometry/region_meta.hpp"
#include "geometry/tubular_grid.hpp"
#include "geometry/base_region.hpp"
#include "geometry/grid.hpp"

// Manuals
#include "core/constants.hpp"
#include "core/input_delegater.hpp"
#include "core/input_document.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
//#include "utility/XXmulti_array.hpp"
//#include "utility/XXrandom.hpp"
//#include "utility/XXmachine.hpp"
//#include "utility/XXmathutil.hpp"
//#include "utility/XXutility.hpp"
// - 
//-
// Check if a region calculates the correct volume for
// itself both for the full volume and for the volume
// accessible to a particle of the given radius.
void geometry_test::calculate_volume_test(const geometry::base_region & reg, double full_volume, double radius, double reduced_volume)
{
  double full;
  double access;
  
  BOOST_REQUIRE_NO_THROW( full = reg.volume( 0.0 ) );
  BOOST_REQUIRE_NO_THROW( access = reg.volume( radius ) );
  BOOST_CHECK_EQUAL( full, full_volume );
  BOOST_CHECK_EQUAL( access, reduced_volume );

}

// Test serialization from base pointer.
void geometry_test::region_serialization_test(boost::shared_ptr< geometry::base_region > regn)
{
  std::stringstream ss;
  std::string label;
  double volume;
  double radius = 0.01;
  bool use_radius = true;
  geometry::coordinate big, small;
  {
    // Cubic grid main ctor
    BOOST_CHECK_NO_THROW( label = regn->label() );
    BOOST_CHECK_NO_THROW( use_radius = regn->fits( radius ) );
    BOOST_REQUIRE( use_radius or regn->fits( 0.0 ) ); // something must fit!
    BOOST_CHECK_NO_THROW( volume = regn->volume( (use_radius ? radius : 0.0 ) ) );
    BOOST_CHECK_NO_THROW( regn->extent( small, big, (use_radius ? radius : 0.0 ) ) );
  
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << regn );
  }
  {
    boost::shared_ptr< geometry::base_region > var2;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var2 );
    BOOST_CHECK_EQUAL( var2->label(), label );
    BOOST_CHECK_EQUAL( var2->fits( radius ), use_radius );
    double newvol;
    BOOST_CHECK_NO_THROW( newvol = var2->volume( (use_radius ? radius : 0.0 ) ) );
    BOOST_CHECK_EQUAL( newvol, volume );
    geometry::coordinate newbig, newsmall;
    BOOST_CHECK_NO_THROW( var2->extent( newsmall, newbig, (use_radius ? radius : 0.0 ) ) );
    BOOST_CHECK_EQUAL( newsmall, small );
    BOOST_CHECK_EQUAL( newbig, big );
  }

}

// Test result of generating a new position. If fits is always
//  true then set bigradius to a negative number to avoid certain
//  tests.
// 
//  + fits(okradius) & (bigradius > 0 and not fits(bigradius))
//  + is_inside(inside) & not is_inside(outside)
//  + is_inside(new_position(rand, okradius))
//  + bigradius > 0 and throw on new_position(rand, bigradius)
//  + is_inside(newpos) == new_position_offset(rand, inside, off, okradius)
//  + throw on new_position_offset(rand, outside, off, okradius)
//  + throw on new_position_offset(rand, inside, off, bigradius)
void geometry_test::region_new_position_test(const geometry::base_region & regn, const geometry::coordinate & inside, const geometry::coordinate & outside, double okradius, double bigradius)
{
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rand( generator );
  const double offset( 1.0 );
  // region = regn
  BOOST_REQUIRE( regn.fits( okradius ) );
  BOOST_REQUIRE( ( ( bigradius < 0.0 ) or ( not regn.fits( bigradius ) ) ) );
  BOOST_REQUIRE( regn.is_inside( inside, okradius ) );
  BOOST_REQUIRE( not regn.is_inside( outside, okradius ) );
  
  // Test methods that should throw first
  if( bigradius > 0.0 )
  {
    BOOST_CHECK( not regn.is_inside( outside, bigradius ) );
    BOOST_CHECK( not regn.is_inside( inside, bigradius ) );
    try
    {
      geometry::coordinate pos;
      // new_position is constant so should have strong exception
      // guarantee for regn. If rand is changed is undefined but should
      // be in well-defined state. Value of pos is undefined.
      pos = regn.new_position( rand, bigradius );
      BOOST_ERROR( ("expected \"regn.new_position( rand, bigradius )\" exception not thrown") );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Particle can not fit in region" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( ("exception thrown by \"regn.new_position( rand, bigradius )\" was not expected type: ") ) + err.what() );
    }
    try
    {
      geometry::coordinate pos = inside;
      // new_position_offset is constant so should have strong exception
      // guarantee for regn. If rand is changed is undefined but should
      // be in well-defined state. Value of pos is undefined.
      regn.new_position_offset( rand, pos, offset, bigradius );
      BOOST_ERROR( ("expected \"regn.new_position_offset( rand, pos, offset, bigradius )\" exception not thrown") );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Particle can not fit in region" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( ("exception thrown by \"regn.new_position_offset( rand, pos, offset, bigradius )\" was not expected type: ") ) + err.what() );
    }
    try
    {
      geometry::coordinate pos = outside;
      // new_position is constant so should have strong exception exception
      // guarantee for regn. If rand is changed is undefined but should be
      // in well-defined state. Value of pos is undefined.
      regn.new_position_offset( rand, pos, offset, bigradius );
      BOOST_ERROR( ("expected \"regn.new_position_offset( rand, pos, offset, bigradius )\" exception not thrown") );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Particle can not fit in region" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( ("exception thrown by \"regn.new_position_offset( rand, pos, offset, bigradius )\" was not expected type: ") ) + err.what() );
    }
  }
  {
    try
    {
      geometry::coordinate pos = outside;
      // new_position is constant so should have strong exception exception
      // guarantee for regn. If rand is changed is undefined but should be
      // in well-defined state. Value of pos is undefined.
      regn.new_position_offset( rand, pos, offset, okradius );
      BOOST_ERROR( ("expected \"regn.new_position_offset( rand, pos, offset, okradius )\" exception not thrown") );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Particle must start inside region" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( ("exception thrown by \"regn.new_position_offset( rand, pos, offset, okradius )\" was not expected type: ") ) + err.what() );
    }
  }
  for( std::size_t count = 0; count != 1000; ++count )
  {
    {
      geometry::coordinate pos;
      BOOST_REQUIRE_NO_THROW( pos = regn.new_position( rand, okradius ) );
      BOOST_CHECK( regn.is_inside( pos, okradius ) );
    }
    {
      geometry::coordinate pos( inside );
      bool result;
      BOOST_REQUIRE_NO_THROW( result = regn.new_position_offset( rand, pos, offset, okradius ) );
      BOOST_CHECK_EQUAL( result, regn.is_inside( pos, okradius ) );
    }
  }
  
  

}

// Test result of generating a gridder. Ideal count values are
// very low to large. Ideal test spacing is from small to a
// value such that regn.fits(space/2) is false (if possible).
// All positions should be inside the regn, and if closed should
// be a minimum of spacing/2 from an edge.
void geometry_test::region_make_gridder_test(const geometry::base_region & regn, std::size_t min_count, std::size_t max_count, double min_space, double max_space, bool is_closed)
{
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rand( generator );
  // region = regn
  
  // check count grid generation
  {
    if( min_count > max_count )
    {
      std::swap( max_count, min_count );
    }
    const std::size_t increment = ( min_count + 10 > max_count ? 1 : ( max_count - min_count ) / 10 );
  
    for( std::size_t count = min_count; count <= max_count; count += increment )
    {
      boost::shared_ptr< geometry::grid_generator > grid;
      grid = regn.make_gridder( rand, count );
      BOOST_CHECK_LE( count, grid->size() );
      // All points are inside the region
      {
        geometry::coordinate pos;
        if( is_closed )
        {
          const double hspace { grid->spacing() / 2 };
  //#if ( defined(DEBUG) and DEBUG>0 )
  //        std::stringstream ss;
  //        grid->dump( ss );
  //#endif
          while( grid->next( pos ) )
          {
            if ( not regn.is_inside( pos, hspace ) )
            {
              geometry::coordinate p1, p2;
              regn.extent( p1, p2, hspace );
              std::cerr << "Point(" << grid->size() << ") [" << pos << "] with radius " << hspace;
              std::cerr << " is outside region with bounds [" << p1;
              std::cerr << "][" << p2 << "]\n";
  //#if ( defined(DEBUG) and DEBUG>0 )
  //            std::cerr << ss.str();
  //#endif
            }
            BOOST_REQUIRE( regn.is_inside( pos, hspace ) );
  //#if ( defined(DEBUG) and DEBUG>0 ) 
  //          ss.str( "" );
  //          grid->dump( ss );
  //#endif
          }
        }
        else
        {
          while( grid->next( pos ) )
          {
            if ( not regn.is_inside( pos, 0.0 ) )
            {
              geometry::coordinate p1, p2;
              regn.extent( p1, p2, 0.0 );
              std::cerr << "Point(" << grid->size() << ") [" << pos << "] with zero radius";
              std::cerr << " is outside region with bounds [" << p1;
              std::cerr << "][" << p2 << "]\n";
            }
            BOOST_REQUIRE( regn.is_inside( pos, 0.0 ) );
          }
        }
      }
    }
  }
  
  // check spacing grid generation
  {
    if( min_space > max_space )
    {
      std::swap( min_space, max_space );
    }
    const double increment = ( min_space == max_space ? 1.0 : ( max_space - min_space ) / 10.0 );
  
    double space = 0.0;
    for( space = min_space; space <= max_space; space += increment )
    {
      if( regn.fits( space/2 ) )
      {
        boost::shared_ptr< geometry::grid_generator > grid;
        BOOST_CHECK_NO_THROW( grid = regn.make_gridder( space, rand ) );
        BOOST_CHECK_LE( space, grid->spacing() );
        {
          geometry::coordinate pos;
          if( is_closed )
          {
            const double hspace { space / 2 };
            while( grid->next( pos ) )
            {
              if ( not regn.is_inside( pos, hspace ) )
              {
                geometry::coordinate p1, p2;
                regn.extent( p1, p2, hspace );
                std::cerr << "Point(" << grid->size() << ")[" << pos << "] with radius " << hspace;
                std::cerr << " is outside region with bounds [" << p1;
                std::cerr << "][" << p2 << "]\n";
              }
              BOOST_REQUIRE( regn.is_inside( pos, hspace ) );
            }
          }
          else
          {
  //#if ( defined(DEBUG) and DEBUG>0 )
  //          std::stringstream ss;
  //          grid->dump( ss );
  //#endif
            while( grid->next( pos ) )
            {
              if ( not regn.is_inside( pos, 0.0 ) )
              {
                geometry::coordinate p1, p2;
                regn.extent( p1, p2, 0.0 );
                std::cerr << "Point(" << grid->size() << ") [" << pos << "] with zero radius";
                std::cerr << " is outside region with bounds [" << p1;
                std::cerr << "][" << p2 << "]\n";
  //#if ( defined(DEBUG) and DEBUG>0 )
  //              std::cerr << ss.str();
  //#endif
              }
              BOOST_REQUIRE( regn.is_inside( pos, 0.0 ) );
  //#if ( defined(DEBUG) and DEBUG>0 ) 
  //            ss.str( "" );
  //            grid->dump( ss );
  //#endif
            }
          }
        }
      }
      else
      {
        try
        {
          regn.make_gridder( space, rand );
          BOOST_ERROR( ( "expected \"regn.make_gridder( space, rand )\" exception not thrown" ) );
        }
        catch( std::runtime_error const& err )
        {
          const std::string msg( err.what() );
          //std::cout << msg << "\n";
          BOOST_CHECK( msg.find( "Requested inter-node spacing is too large for region" ) < msg.size() );
        }
        catch( std::exception const& err )
        {
          BOOST_ERROR( std::string( ( "exception thrown by \"regn.make_gridder( space, rand )\" was not expected type: " ) ) + err.what() );
        }
      }
    }
  }
  
  

}

//Test the functionality of the base class
void geometry_test::test_grid_generator(boost::shared_ptr< geometry::grid_generator > gridder)
{
  // Store generated coordinates
  std::vector< geometry::coordinate > postns;
  // Initial grid point count
  const std::size_t count { gridder->size() };
  // Test serialization
  std::stringstream store;
  {
     boost::archive::text_oarchive oa(store);
     // write class instance to archive
     BOOST_REQUIRE_NO_THROW( oa << gridder );
  }
  
  auto sqr = [](geometry::coordinate a, geometry::coordinate b){
    const double dx{ a.x - b.x }, dy{ a.y - b.y }, dz{ a.z - b.z };
    return dx * dx + dy * dy + dz * dz;
  };
  const double spc_sqr{ std::pow( gridder->spacing(), 2 ) };
  // Test grid next
  for ( std::size_t idx = 0; idx != count; ++idx )
  {
     // Not empty before first 'count' calls to gridder->next
     BOOST_CHECK( not gridder->empty() );
     // Non zero size before first 'count' calls to gridder->next
     BOOST_CHECK_LT( 0, gridder->size() );
  
     const std::size_t remain { gridder->size() };
     geometry::coordinate pnt;
  
     // Next return is not false for first 'count' - 1 calls to gridder->next
     // and false for the 'count'th call.
     BOOST_REQUIRE_MESSAGE( gridder->next( pnt ), "gridder.next() is false before expected end point" );
  
     // Size decreases by one during call to gridder->next
     BOOST_CHECK_EQUAL( remain - 1, gridder->size() );
  
     // New generated position is...
     for (auto const& p2 : postns )
     {
        // not the same as any previously generated position
        BOOST_REQUIRE_NE( p2, pnt );
        // not closer than spacing from any previous position
        BOOST_REQUIRE_LE( spc_sqr, sqr( pnt, p2 ) );
     }
  
     // store generated position.
     postns.push_back( pnt );
  }
  {
     geometry::coordinate pnt;
     BOOST_REQUIRE_MESSAGE( not gridder->next( pnt), "gridder.next() is true after expected end point" );
  }
  // Gridder is empty after 'count' calls
  BOOST_CHECK( gridder->empty() );
  // Gridder has zero size after 'count' calls
  BOOST_CHECK_EQUAL(0, gridder->size());
  
  // Test deserialization
  {
     boost::shared_ptr< geometry::grid_generator > g;
     boost::archive::text_iarchive oa(store);
     // read class instance from archive
     BOOST_REQUIRE_NO_THROW( oa >> g );
  
     BOOST_REQUIRE_EQUAL( count, g->size() );
     for ( std::size_t idx = 0; idx != count; ++idx )
     {
        // Not empty before first 'count' calls to gridder->next
        BOOST_CHECK( not g->empty() );
        // Non zero size before first 'count' calls to gridder->next
        BOOST_CHECK_LT( 0, g->size() );
  
        const std::size_t remain { g->size() };
        geometry::coordinate pnt;
  
        // Next return is not false for first 'count' - 1 calls to gridder->next
        // and false for the 'count'th call.
        BOOST_REQUIRE_MESSAGE( g->next( pnt), "gridder->next() is false before count calls" );
        // Expect copy to return points in same order as original
        BOOST_CHECK_EQUAL( pnt, postns[idx] );
  
        // Size decreases by one during call to gridder->next
        BOOST_CHECK_EQUAL( remain - 1, g->size() );
     }
  
  }
  

}

BOOST_AUTO_TEST_CASE( coordinate_test )
{
  auto test_coord = [](geometry::coordinate const& p1, double x, double y, double z)
  {
    BOOST_CHECK_EQUAL(p1.x, x);
    BOOST_CHECK_EQUAL(p1.y, y);
    BOOST_CHECK_EQUAL(p1.z, z);
  };
  
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< geometry::coordinate >::type{} );
    BOOST_CHECK( std::is_copy_constructible< geometry::coordinate >::type{} );
    BOOST_CHECK( std::is_move_constructible< geometry::coordinate >::type{} );
    BOOST_CHECK( (std::is_assignable< geometry::coordinate, geometry::coordinate >::type{}) );
    BOOST_CHECK( not std::has_virtual_destructor< geometry::coordinate >::type{} );
  }
  
  // Constructor tests:
  std::stringstream store;
  {
    geometry::coordinate p1;
    test_coord(p1, 0.0, 0.0, 0.0);
  
    geometry::coordinate p2 (0.1, 0.3, 0.2);
    test_coord(p2, 0.1, 0.3, 0.2);
  
    geometry::coordinate p3 {0.3, 0.2, 0.1};
    test_coord(p3, 0.3, 0.2, 0.1);
  
    test_coord({0.1,0.2,0.3},0.1,0.2,0.3);
  
    BOOST_CHECK(not p1.equivalent(p2));
    BOOST_CHECK( p1 != p2 );
  
    boost::archive::text_oarchive oa(store);
    // write class instances to archive
    oa << p1 << p2 << p3;
  
    p1 = p2;
    BOOST_CHECK(p1.equivalent(p2));
    BOOST_CHECK( p1 == p2 );
  
    {
      std::stringstream lstore;
      lstore << p1;
      p3.write( lstore );
      {
        geometry::coordinate p4, p5;
  
        BOOST_CHECK( not p1.equivalent( p4 ) );
        BOOST_CHECK( p3 != p5 );
  
        lstore >> p4;
        p5.read( lstore ); 
    
        BOOST_CHECK( p1.equivalent(p4) );
        BOOST_CHECK( p3 == p5 );
      }
    }
  }
  {
    geometry::coordinate P1, P2, P3;
    boost::archive::text_iarchive ia(store);
    // read class instances from archive
    ia >> P1 >> P2 >> P3;
    test_coord(P1, 0.0, 0.0, 0.0);
    test_coord(P2, 0.1, 0.3, 0.2);
    test_coord(P3, 0.3, 0.2, 0.1);
  }
  // Test read
  {
     geometry::coordinate var;
     const static std::string canon{"0.1 0.2 0.3\n2.0\n1.0 0.3 0.5 0.1\n1.1 " };
     std::stringstream ss( canon );
     var.read(ss);
     test_coord( var, 0.1, 0.2, 0.3 );
     ss >> var;
     test_coord( var, 2.0, 1.0, 0.3 );
     var.read(ss);
     test_coord( var, 0.5, 0.1, 1.1 );
  }

}

BOOST_AUTO_TEST_CASE( centroid_test )
{
  auto test_coord = [](geometry::centroid const& p1, double r, double x, double y, double z)
  {
    BOOST_CHECK_EQUAL(p1.r, r);
    BOOST_CHECK_EQUAL(p1.x, x);
    BOOST_CHECK_EQUAL(p1.y, y);
    BOOST_CHECK_EQUAL(p1.z, z);
  };
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< geometry::centroid >::type{} );
    BOOST_CHECK( std::is_copy_constructible< geometry::centroid >::type{} );
    BOOST_CHECK( std::is_move_constructible< geometry::centroid >::type{} );
    BOOST_CHECK( (std::is_assignable< geometry::centroid, geometry::centroid >::type{}) );
    BOOST_CHECK( not std::has_virtual_destructor< geometry::centroid >::type{} );
  } 
  // Constructor tests:
  std::stringstream store;
  {
     // default ctor
    geometry::centroid p1;
    test_coord(p1, 0.0, 0.0, 0.0, 0.0);
    BOOST_CHECK( p1.position() == geometry::coordinate( 0.0, 0.0, 0.0 ) );
  
    // param ctor
    geometry::centroid p2(1.0, 0.1, 0.3, 0.2);
    test_coord(p2, 1.0, 0.1, 0.3, 0.2);
    BOOST_CHECK( p2.position() == geometry::coordinate( 0.1, 0.3, 0.2 ) );
  
    // initializer list
    geometry::centroid p3 {1.0, 0.3, 0.2, 0.1};
    test_coord(p3, 1.0, 0.3, 0.2, 0.1);
    BOOST_CHECK( p3.position() == geometry::coordinate( 0.3, 0.2, 0.1 ) );
  
    // initializer list
    test_coord({1.0, 0.1, 0.2, 0.3}, 1.0, 0.1, 0.2, 0.3 );
  
    // copy
    geometry::centroid p4( p3 );
    test_coord(p4, 1.0, 0.3, 0.2, 0.1);
  
    // move
    geometry::centroid p5( std::move( geometry::centroid( p3 ) ) );
    test_coord(p5, 1.0, 0.3, 0.2, 0.1);
  
    // assignment
    p5 = p2;
    test_coord(p5, 1.0, 0.1, 0.3, 0.2);
  
    BOOST_CHECK( not p1.equivalent( p2 ) );
    BOOST_CHECK( p1 != p2 );
  
    boost::archive::text_oarchive oa(store);
    // write class instances to archive
    oa << p1 << p2 << p3;
  
    p1 = p2;
    BOOST_CHECK( p1.equivalent( p2 ) );
    BOOST_CHECK( p1 == p2 );
  
    {
      std::stringstream lstore;
      lstore << p1 << " ";
      p3.write( lstore );
      {
      geometry::centroid p4, p5;
  
      BOOST_CHECK( not p1.equivalent( p4 ) );
      BOOST_CHECK( p3 != p5 );
  
      p4.read( lstore );
      lstore >> p5; 
    
      BOOST_CHECK( p1.equivalent( p4 ) );
      BOOST_CHECK( p3 ==  p5 );
      }
    }
  }
  {
    geometry::centroid P1, P2, P3;
    boost::archive::text_iarchive ia( store );
    // read class instances from archive
    ia >> P1 >> P2 >> P3;
    test_coord( P1, 0.0, 0.0, 0.0, 0.0 );
    test_coord( P2, 1.0, 0.1, 0.3, 0.2 );
    test_coord( P3, 1.0, 0.3, 0.2, 0.1 );
  }
  // Test read
  {
     geometry::centroid var;
     const static std::string canon{"1.0 0.1 0.2 0.3\n2.0\n1.0 0.3 0.5 0.1\n1.1 2.0 1.0" };
     std::stringstream ss( canon );
     var.read(ss);
     test_coord( var, 1.0, 0.1, 0.2, 0.3 );
     ss >> var;
     test_coord( var, 2.0, 1.0, 0.3, 0.5 );
     var.read(ss);
     test_coord( var, 0.1, 1.1, 2.0, 1.0 );
  }

}

//Simple compliance test for 2D circle grid
BOOST_AUTO_TEST_CASE( circle_grid_lifetime_test )
{
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< geometry::circle_grid >::type {} );
    BOOST_CHECK( std::is_copy_constructible< geometry::circle_grid >::type {} );
    BOOST_CHECK( std::is_move_constructible< geometry::circle_grid >::type {} );
    BOOST_CHECK( ( std::is_assignable< geometry::circle_grid, geometry::circle_grid >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< geometry::circle_grid >::type {} );
  }
  std::stringstream log;
  {
    // Circle grid default ctor
    geometry::circle_grid var1;
    // Default should have no size
    BOOST_REQUIRE_EQUAL( var1.size(), 0 );
    // Should throw error
    geometry::coordinate pnt;
    try
    {
      var1.set_xy( pnt, 0 );
      BOOST_ERROR( ( "expected \"var1.set_xy( pnt, 0 )\" exception not thrown" ) );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Index out of range" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( ( "exception thrown by \"var1.set_xy( pnt, 0 )\" was not expected type: " ) ) + err.what() );
    }
  }
  {
    // Circle grid copy and serialization test
    const double radius{ 3.0 };
    const double space{ 1.0 };
    std::stringstream ss;
    {
      geometry::circle_grid var1( radius, space );
      BOOST_CHECK_EQUAL( var1.size(), 21 );
      {
        geometry::circle_grid var2( var1 );
        BOOST_CHECK_EQUAL( var2.size(), 21 );
        BOOST_CHECK_EQUAL( var1.size(), 21 );
      }
      {
        geometry::circle_grid var3;
        BOOST_CHECK_EQUAL( var3.size(), 0 );
        var3 = var1;
        BOOST_CHECK_EQUAL( var3.size(), 21 );
        BOOST_CHECK_EQUAL( var1.size(), 21 );
      }
      {
        geometry::circle_grid var4( var1 );
        geometry::circle_grid var5( std::move( var4 ) );
        BOOST_CHECK_EQUAL( var5.size(), 21 );
      }
  
      boost::archive::text_oarchive oa( ss );
      // write class instance to archive
      oa << var1;
      BOOST_CHECK_EQUAL( var1.size(), 21 );
    }
    {
      geometry::circle_grid var1;
      BOOST_CHECK_EQUAL( var1.size(), 0 );
      boost::archive::text_iarchive ia( ss );
      // read class instance from archive
      ia >> var1;
      BOOST_CHECK_EQUAL( var1.size(), 21 );
    }
  }

}

//Simple compliance test for 2D circle grid
BOOST_AUTO_TEST_CASE( circle_grid_methods_test )
{
  {
    // Circle grid real ctor
    const double radius{ 3.0 };
    const double space{ 1.0 };
    geometry::circle_grid var1( radius, space );
    BOOST_CHECK_EQUAL( var1.size(), 21 );
  
    const double r_edge( std::pow( radius - space, 2 ) );
    const double r_max( std::pow( radius - ( space/2 ), 2 ) );
    const std::size_t size( var1.size() );
    std::size_t countr1 = 0; // count of elements near r
    for( std::size_t idx = 0; idx != size; ++idx )
    {
      BOOST_CHECK_LT( 0, var1.size() );
      geometry::coordinate pnt;
      var1.set_xy( pnt, idx );
      const double r( pnt.x*pnt.x + pnt.y*pnt.y );
      BOOST_CHECK_LE( r, r_max );
      if( r >= r_edge ) ++countr1;
    }
    BOOST_CHECK_EQUAL( size, var1.size() );
    // Should throw error
    try
    {
      geometry::coordinate pnt;
      var1.set_xy( pnt, size );
      BOOST_ERROR( ( "expected \"var1.set_xy( pnt, size )\" exception not thrown" ) );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Index out of range" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( ( "exception thrown by \"var1.set_xy( pnt, size )\" was not expected type: " ) ) + err.what() );
    }
    BOOST_CHECK_LT( 0, countr1 );
    const std::size_t mod4{ countr1 % 4 };
    const std::size_t mod8{ countr1 % 8 };
    BOOST_CHECK_MESSAGE( mod4 == 0, "4 fold symmetry error: 0 != countr1 % 4" );
    BOOST_WARN_MESSAGE( mod8 == 0, "8 fold symmetry error: 0 != countr1 % 8" );
  }
  {
    std::vector< geometry::coordinate > canon;
    // Circle grid copy and serialization test
    const double radius{ 3.0 };
    const double space{ 1.0 };
    std::stringstream ss;
    {
      geometry::circle_grid var1 = geometry::circle_grid( radius, space );
      BOOST_CHECK_EQUAL( var1.size(), 21 );
  
      boost::archive::text_oarchive oa( ss );
      // write class instance to archive
      oa << var1;
  
      const std::size_t size( var1.size() );
      canon.resize( size );
      for( std::size_t idx = 0; idx != size; ++idx )
      {
        var1.set_xy( canon[ idx ], idx );
        BOOST_CHECK_LT( 0, var1.size() );
      }
    }
    {
      geometry::circle_grid var3;
      BOOST_CHECK_EQUAL( var3.size(), 0 );
      boost::archive::text_iarchive ia( ss );
      // read class instance from archive
      ia >> var3;
      BOOST_CHECK_EQUAL( var3.size(), 21 );
      BOOST_REQUIRE_EQUAL( var3.size(), canon.size() );
      for( std::size_t idx = 0; idx != var3.size(); ++idx )
      {
        geometry::coordinate pnt;
        var3.set_xy( pnt, idx );
        BOOST_CHECK_EQUAL( pnt, canon[idx] );
      }
  
    }
    {
      // circle grid that should be used in tubular grid
      geometry::circle_grid var1( 8.5, 0.5 );
      BOOST_CHECK_EQUAL( var1.size(), 861 );
    }
  }

}

// compliance test for 3D cubic grid
BOOST_AUTO_TEST_CASE( cubic_grid_lifetime_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::cubic_grid >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::cubic_grid >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::cubic_grid >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::cubic_grid, geometry::cubic_grid >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::cubic_grid >::type {} );
  }
  std::stringstream ss;
  {
    boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
    utility::random_distribution rgen( generator );
    // Cubic grid ctor from count
    boost::shared_ptr< geometry::cubic_grid > var1( geometry::cubic_grid::make_grid( { 0.0, 0.0, 0.0 }, 30.0, rgen, 216 ) );
    BOOST_CHECK_EQUAL( var1->size(), 216 );
  
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << var1 );
    BOOST_CHECK_EQUAL( var1->size(), 216 );
  
    // Cubic grid ctor from spacing
    boost::shared_ptr< geometry::cubic_grid > var2( geometry::cubic_grid::make_grid( { 0.0, 0.0, 0.0 }, 30.0, var1->spacing(), rgen ) );
    BOOST_CHECK_EQUAL( var2->size(), 216 );
    BOOST_CHECK_EQUAL( var2->spacing(), var1->spacing() );
  }
  {
    boost::shared_ptr< geometry::cubic_grid > var1;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var1 );
    BOOST_CHECK_EQUAL( var1->size(), 216 );
  }
  

}

//Simple compliance test for cubic grid.
BOOST_AUTO_TEST_CASE( cubic_grid_test )
{
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgen( generator );
  std::vector< geometry::coordinate > canon;
  // Cubic grid 30.0, 216
  // 216**1/3 => 6
  // 30/6 => 5 (spacing)
  // Edge spacing => 2.5
  // Min == 2.5; Max == 27.5
  {
    // cubic grid ctor(length, numpart, ranf)
    boost::shared_ptr< geometry::grid_generator > var1( geometry::cubic_grid::make_grid( { 0.0, 0.0, 0.0 }, 30.0, rgen, 216 ) );
    BOOST_CHECK_EQUAL( var1->size(), 216 );
    BOOST_CHECK( not var1->empty() );
    // Test base class behavior
    geometry_test::test_grid_generator( var1 );
  }
  {
    // Test all coordinates are in the expected
    // range.
    boost::shared_ptr< geometry::cubic_grid > var1( geometry::cubic_grid::make_grid( { 0.0, 0.0, 0.0 }, 30.0, rgen, 216 ) );
  
    boost::shared_ptr< geometry::cubic_grid > var2;
    {
      std::stringstream ss;
      boost::archive::text_oarchive oa( ss );
      // write class instance to archive
      oa << var1;
      boost::archive::text_iarchive ia( ss );
      // read class instance from archive
      BOOST_CHECK_NO_THROW( ia >> var2 );
      BOOST_CHECK_EQUAL( var2->size(), 216 );
    }
    const std::size_t size { var1->size() };
    canon.resize( size );
    for( std::size_t idx = 0; idx != size; ++idx )
    {
      geometry::coordinate pnt;
      geometry::coordinate pnt2;
      var1->next( pnt );
      var2->next( pnt2 );
      BOOST_CHECK_EQUAL( pnt, pnt2 );
      BOOST_CHECK_EQUAL( var1->size(), ( size - idx - 1 ) );
      BOOST_CHECK( not var1->empty() or ( size == idx + 1 ) );
      BOOST_CHECK_LE( pnt.x, 27.5 );
      BOOST_CHECK_LE( pnt.y, 27.5 );
      BOOST_CHECK_LE( pnt.z, 27.5 );
      BOOST_CHECK_LE( 2.5, pnt.x );
      BOOST_CHECK_LE( 2.5, pnt.y );
      BOOST_CHECK_LE( 2.5, pnt.z );
      canon[idx] = pnt;
    }
    // Check every valid point in the grid is generated
    // by converting grid position into an index
    std::vector< bool > check( size, false );
    std::vector< size_t > indices( size );
    for( std::size_t ix = 0; ix != size; ++ix )
    {
      geometry::coordinate pnt( canon[ix] );
      std::size_t idx( ( ( ( pnt.x - 2.5 ) * 6 + pnt.y - 2.5 ) * 6 + ( pnt.z - 2.5 ) ) / 5.0 );
      // Check generated index is less than size
      BOOST_CHECK_LT( idx, size );
      indices[ix] = idx;
      // Check generated index has not already seen
      BOOST_CHECK( not check[idx] );
      check[idx] = true;
    }
    {
      std::sort( indices.begin(), indices.end() );
      for( std::size_t ix = 0; ix != size; ++ix )
      {
        // Check generated index runs from 0 to 'size'
        BOOST_CHECK_EQUAL( ix, indices[ix] );
      }
    }
    // Check for missing (false) indices.
    BOOST_CHECK( check.end() == std::find( check.begin(), check.end(), false ) );
  }
  

}

BOOST_AUTO_TEST_CASE( digitizer_3d_lifetime_test )
{
  {
    // Static Lifetime method tests: canonical pattern
  
    BOOST_CHECK( std::is_default_constructible< geometry::digitizer_3d >::type{} );
    BOOST_CHECK( std::is_copy_constructible< geometry::digitizer_3d >::type{} );
    BOOST_CHECK( std::is_move_constructible< geometry::digitizer_3d >::type{} );
    BOOST_CHECK( ( std::is_assignable< geometry::digitizer_3d, geometry::digitizer_3d >::type{} ) );
    BOOST_CHECK( not std::has_virtual_destructor< geometry::digitizer_3d >::type{} );
  }
  const geometry::coordinate pzero{ 0.0, 0.0, 0.0 };
  const geometry::coordinate pminusone{ -1.0, -1.0, -1.0 };
  const geometry::coordinate pone{ 1.0, 1.0, 1.0 };
  
  {
    // Default ctor
    geometry::digitizer_3d var;
    BOOST_CHECK_EQUAL( var.minimum(), pzero );
    BOOST_CHECK_EQUAL( var.maximum(), pzero );
    BOOST_CHECK_EQUAL( var.size(), 0ul );
    for( int x = -10; x != 11; ++x )
    {
      const geometry::coordinate pos{ double( x )/5.0, double( x )/4.0, double( x )/6.0 };
      BOOST_REQUIRE( not var.in_range( pos ) );
    }
  }
  std::stringstream store;
  {
    // Min/Max/Count ctor
    geometry::digitizer_3d var( pminusone, pone, 0.2 );
    BOOST_CHECK_EQUAL( var.minimum(), pminusone );
    BOOST_CHECK_EQUAL( var.maximum(), pone );
    BOOST_CHECK_EQUAL( var.size(), 1000ul );
    const geometry::coordinate bwidth{ var.bin_width() };
    BOOST_CHECK( utility::feq( bwidth.x, 0.2 ) );
    BOOST_CHECK( utility::feq( bwidth.y, 0.2 ) );
    BOOST_CHECK( utility::feq( bwidth.z, 0.2 ) );
  
    boost::archive::text_oarchive oa(store);
    // write class instances to archive
    oa << var;
  
    {
      // copy
      geometry::digitizer_3d var2( var );
      BOOST_CHECK_EQUAL( var2.minimum(), pminusone );
      BOOST_CHECK_EQUAL( var2.maximum(), pone );
      BOOST_CHECK_EQUAL( var2.size(), 1000ul );
      const geometry::coordinate bwidth{ var2.bin_width() };
      BOOST_CHECK( utility::feq( bwidth.x, 0.2 ) );
      BOOST_CHECK( utility::feq( bwidth.y, 0.2 ) );
      BOOST_CHECK( utility::feq( bwidth.z, 0.2 ) );
      BOOST_CHECK( var == var2 );
    }
    {
      // assign
      geometry::digitizer_3d var2;
      BOOST_CHECK_EQUAL( var2.minimum(), pzero );
      BOOST_CHECK_EQUAL( var2.maximum(), pzero );
      var2 = var;
      BOOST_CHECK_EQUAL( var2.minimum(), pminusone );
      BOOST_CHECK_EQUAL( var2.maximum(), pone );
      BOOST_CHECK_EQUAL( var2.size(), 1000ul );
      const geometry::coordinate bwidth{ var2.bin_width() };
      BOOST_CHECK( utility::feq( bwidth.x, 0.2 ) );
      BOOST_CHECK( utility::feq( bwidth.y, 0.2 ) );
      BOOST_CHECK( utility::feq( bwidth.z, 0.2 ) );
      BOOST_CHECK( var == var2 );
    }
    {
      // move
      geometry::digitizer_3d var1( var );
      BOOST_CHECK_EQUAL( var1.minimum(), pminusone );
      BOOST_CHECK_EQUAL( var1.maximum(), pone );
      geometry::digitizer_3d var2( std::move( var1 ) );
      BOOST_CHECK_EQUAL( var2.minimum(), pminusone );
      BOOST_CHECK_EQUAL( var2.maximum(), pone );
      BOOST_CHECK_EQUAL( var2.size(), 1000ul );
      const geometry::coordinate bwidth{ var2.bin_width() };
      BOOST_CHECK( utility::feq( bwidth.x, 0.2 ) );
      BOOST_CHECK( utility::feq( bwidth.y, 0.2 ) );
      BOOST_CHECK( utility::feq( bwidth.z, 0.2 ) );
      BOOST_CHECK( var == var2 );
    }
  }
  {
    geometry::digitizer_3d var;
   
    boost::archive::text_iarchive ia(store);
    // read class instances from archive
    ia >> var;
    BOOST_CHECK_EQUAL( var.minimum(), pminusone );
    BOOST_CHECK_EQUAL( var.maximum(), pone );
    BOOST_CHECK_EQUAL( var.size(), 1000ul );
    const geometry::coordinate bwidth{ var.bin_width() };
    BOOST_CHECK( utility::feq( bwidth.x, 0.2 ) );
    BOOST_CHECK( utility::feq( bwidth.y, 0.2 ) );
    BOOST_CHECK( utility::feq( bwidth.z, 0.2 ) );
  } 
  

}

BOOST_AUTO_TEST_CASE( digitizer_3d_method_test )
{
  {
    // Min/Max/step ctor
    geometry::digitizer_3d var( {-1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0 }, 0.2 );
    BOOST_CHECK_EQUAL( var.size(), 1000ul );
    const geometry::coordinate bwidth{ var.bin_width() };
    BOOST_CHECK( utility::feq( bwidth.x, 0.2 ) );
    BOOST_CHECK( utility::feq( bwidth.y, 0.2 ) );
    BOOST_CHECK( utility::feq( bwidth.z, 0.2 ) );
    for( int x = -10; x != 11; ++x )
    {
      geometry::coordinate pos{ double( x )/5.0, double( x )/4.0, double( x )/6.0 };
      if( not var.in_range( pos ) )
      {
        if( -4 <= x and x < 4 )
        {
          std::stringstream ss;
          ss << pos << " should be in range!";
          BOOST_ERROR( ss.str() );
        }
      }
      else
      {
        if( -4 > x or 4 >= 5 )
        {
          std::stringstream ss;
          ss << pos << " should NOT be in range!";
          BOOST_ERROR( ss.str() );
        }
      }
    }
  }

}

//Simple compliance test for tubular grid
BOOST_AUTO_TEST_CASE( tubular_grid_lifetime_test )
{
  boost::shared_ptr< boost::mt19937> generator(new boost::mt19937);
  std::stringstream log;
  utility::random_distribution rgen(generator);
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::tubular_grid >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::tubular_grid >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::tubular_grid >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::tubular_grid, geometry::tubular_grid >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::tubular_grid >::type {} );
  }
  std::stringstream ss;
  {
    boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
    utility::random_distribution rgen( generator );
    // Cubic grid main ctor
    boost::shared_ptr< geometry::tubular_grid > var1( new geometry::tubular_grid( 31.0, 8.5, 2.5, rgen ) );
    BOOST_CHECK_EQUAL( var1->size(), 300 );
  
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << var1 );
    BOOST_CHECK_EQUAL( var1->size(), 300 );
  }
  {
    boost::shared_ptr< geometry::tubular_grid > var1;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var1 );
    BOOST_CHECK_EQUAL( var1->size(), 300 );
  }

}

//Simple compliance test for tubular grid
BOOST_AUTO_TEST_CASE( tubular_grid_test )
{
  boost::shared_ptr< boost::mt19937> generator(new boost::mt19937);
  std::stringstream log;
  utility::random_distribution rgen(generator);
  // Cylindrical grid 
  // HLENGTH 31.0,
  // RADIUS  8.5
  // SPACING 0.5
  // ZGRID = (31 - .5) / .5 = 61
  // XYGRID = Y \   X
  //             \        2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x
  //              \ 0     0.5   1     1.5   2     2.5   3     3.5   4     4.5   5     5.5   6     6.5   7     7.5   8
  //               \---------------------------------------------------------------------------------------------------
  //    33    0    |0     0.5   1     1.5   2     2.5   3     3.5   4     4.5   5     5.5   6     6.5   7     7.5   8.0
  // 2x 33    0.5  |0.5   0.7   1.1   1.6   2.1   2.5   3.0   3.5   4.0   4.5   5.0   5.5   6.0   6.5   7.0   7.5   8.0
  // 2x 33    1    |1     1.1   1.4   1.8   2.2   2.7   3.2   3.6   4.1   4.6   5.1   5.6   6.1   6.6   7.1   7.6   8.1
  // 2x 33    1.5  |1.5   1.6   1.8   2.1   2.5   2.9   3.4   3.8   4.3   4.7   5.2   5.7   6.2   6.7   7.2   7.6   8.1
  // 2x 33    2    |2     2.1   2.2   2.5   2.8   3.2   3.6   4.0   4.5   4.9   5.4   5.9   6.3   6.8   7.3   7.8   8.2
  // 2x 31    2.5  |2.5   2.5   2.7   2.9   3.2   3.5   3.9   4.3   4.7   5.1   5.6   6.0   6.5   7.0   7.4   7.9
  // 2x 31    3    |3     3.0   3.2   3.4   3.6   3.9   4.2   4.6   5.0   5.4   5.8   6.3   6.7   7.2   7.6   8.1
  // 2x 29    3.5  |3.5   3.5   3.6   3.8   4.0   4.3   4.6   4.9   5.3   5.7   6.1   6.5   6.9   7.4   7.8
  // 2x 29    4    |4     4.0   4.1   4.3   4.5   4.7   5.0   5.3   5.7   6.0   6.4   6.8   7.2   7.6   8.1
  // 2x 27    4.5  |4.5   4.5   4.6   4.7   4.9   5.1   5.4   5.7   6.0   6.4   6.7   7.1   7.5   7.9
  // 2x 27    5    |5     5.0   5.1   5.2   5.4   5.6   5.8   6.1   6.4   6.7   7.1   7.4   7.8   8.2
  // 2x 25    5.5  |5.5   5.5   5.6   5.7   5.9   6.0   6.3   6.5   6.8   7.1   7.4   7.8   8.1
  // 2x 23    6    |6     6.0   6.1   6.2   6.3   6.5   6.7   6.9   7.2   7.5   7.8   8.1
  // 2x 21    6.5  |6.5   6.5   6.6   6.7   6.8   7.0   7.2   7.4   7.6   7.9   8.2
  // 2x 17    7    |7     7.0   7.1   7.2   7.3   7.4   7.6   7.8   8.1
  // 2x 13    7.5  |7.5   7.5   7.6   7.6   7.8   7.9   8.1
  // 2x 5     8    |8     8.0   8.1   8.1   8.2 
  // ======
  //    861
  //
  {
    // Tubular grid ctor (with spacing)
     boost::shared_ptr< geometry::grid_generator > var1( new geometry::tubular_grid( 31.0, 8.5, 2.5 ) );
  
    BOOST_CHECK( not var1->empty() );
    // Test base class behavior
    geometry_test::test_grid_generator( var1 );
  }
  {
    // Tubular grid ctor (with target grid point)
    boost::shared_ptr< geometry::grid_generator > var1( new geometry::tubular_grid( 31.0, 8.5, 500 ) );
  
    BOOST_CHECK_LE( 500, var1->size() );
    BOOST_CHECK_LE( 0.0, var1->spacing() );
    BOOST_CHECK( not var1->empty() );
  }
  
  {
    // Tubular grid test: constructed with defined spacing
     const double length{ 31.0 };
     const double radius{ 8.5 };
     const double spacing{ 0.5 };
    geometry::tubular_grid var1 (length, radius, spacing);
    BOOST_CHECK_EQUAL(var1.size(), 861*62);
    // Should have some grid points within 3/2*'spacing' of edge
    const double r_edge{ std::pow(radius - (3*spacing/2), 2) };
    const double z1_edge{ (length - 3*spacing)/2 };
    const double z2_edge{ -z1_edge };
    // should have no grid points within ('spacing'/2) of edge
    const double r_max{ std::pow(radius - spacing/2.0, 2) };
    const double z1_max{ (length - spacing)/ 2.0 };
    const double z2_max{ -z1_max };
    const double z_min{ spacing / 2.0 };
    // Number of grid points
    const std::size_t size{ var1.size() };
    std::set< double > zvalues;
  
    std::size_t counter_spacing_r1 = 0; // count of elements near r, when z>0
    std::size_t counter_spacing_r2 = 0; // count of elements near r, when z<0
    std::size_t counter_spacing_z1 = 0; // count of elements near +z
    std::size_t counter_spacing_z2 = 0; // count of elements near -z
  
    for (std::size_t idx = 0; idx != size; ++idx)
    {
      BOOST_CHECK_LT(0, var1.size());
      geometry::coordinate pnt;
      var1.next(pnt);
      const double r(pnt.x*pnt.x + pnt.y*pnt.y);
      BOOST_CHECK_LE( r, r_max );
      BOOST_CHECK_LE( pnt.z, z1_max );
      BOOST_CHECK_LE( z2_max, pnt.z );
      BOOST_CHECK_LE( z_min, std::abs(pnt.z) );
  
      if (r > r_edge and pnt.z > 0.0) ++counter_spacing_r1;
      if (r > r_edge and pnt.z < 0.0) ++counter_spacing_r2;
      if (pnt.z > z1_edge) ++counter_spacing_z1;
      if (pnt.z < z2_edge) ++counter_spacing_z2;
      zvalues.insert( pnt.z );
    }
    log << "ZMIN " << z_min << " ZMAX " << z1_max << " ZEDGE " << z1_edge << " SPC " << spacing << "\n";
    for (double zz : zvalues )
    {
       log << zz << " ";
    }
    log << "\n";
    BOOST_CHECK_EQUAL( 0, var1.size() );
    BOOST_CHECK_LT( 0, counter_spacing_r1 );
    BOOST_CHECK_EQUAL( counter_spacing_r1, counter_spacing_r2 );
    // r count should be multiple of 4*60
    const std::size_t mod_60{ counter_spacing_r1 % 62 };
    BOOST_CHECK_MESSAGE( 0 == mod_60, "0 == (counter_spacing_r1 % 62)" );
    const std::size_t mod_120{ counter_spacing_r1 % (2*62) };
    BOOST_CHECK_MESSAGE( 0 == mod_120, "2 FOLD SYMMETRY FAILURE: 0 == (counter_spacing_r1 % (2*62))" );
    const std::size_t mod_240{ counter_spacing_r1 % (4*62) };
    BOOST_CHECK_MESSAGE( 0 == mod_240, "4 FOLD SYMMETRY FAILURE: 0 == (counter_spacing_r1 % (4*62))" );
    const std::size_t mod_480{ counter_spacing_r1 % (8*62) };
    BOOST_WARN_MESSAGE( 0 == mod_480, "8 FOLD SYMMETRY FAILURE: 0 == (counter_spacing_r1 % (4*62))" );
  
  
    BOOST_CHECK_LT( 0, counter_spacing_z1 );
    BOOST_CHECK_EQUAL( counter_spacing_z1, counter_spacing_z2 );
    // z count should be 861
    BOOST_CHECK_EQUAL( 861, counter_spacing_z1 );
  }
  {
    // Tubular grid: constructed with target grid point minimum count
     const double length{ 31.0 };
     const double radius{ 8.5 };
     geometry::tubular_grid var1( length, radius, 500);
  
    std::set< double > zvalues;
  
    // Should have no points within (spacing/2) of edge
    const double r_max( std::pow(radius - var1.spacing()/2.0, 2) );
    const double z1_max( (length - var1.spacing())/2.0 );
    const double z2_max( -z1_max );
    const double z_min( var1.spacing()/2.0 );
    // Should have some points within 1.5*spacing of edge
    const double r_edge( std::pow(radius - (3*var1.spacing()/2), 2) );
    const double z1_edge( (length - 3*var1.spacing())/2 );
    const double z2_edge( -z1_edge );
  
    std::size_t counter_ntarget_r1 = 0; // count of elements near r, when z>0
    std::size_t counter_ntarget_r2 = 0; // count of elements near r, when z<0
    std::size_t counter_ntarget_z1 = 0; // count of elements near +z
    std::size_t counter_ntarget_z2 = 0; // count of elements near -z
  
    geometry::coordinate pnt;
    while ( var1.next( pnt ) )
    {
      const double r( pnt.x*pnt.x + pnt.y*pnt.y );
      // Test that no grid points get closer than 'spacing'/2 from edge
      BOOST_CHECK_LE( r, r_max );
      BOOST_CHECK_LE( pnt.z, z1_max );
      BOOST_CHECK_LE( z2_max, pnt.z );
      BOOST_CHECK_LE( z_min, std::abs( pnt.z ) );// no points nearer to z=0.0 than spacing/2
  
      if ( r > r_edge and pnt.z > 0.0 ) ++counter_ntarget_r1;
      if ( r > r_edge and pnt.z < 0.0 ) ++counter_ntarget_r2;
      if ( pnt.z > z1_edge ) ++counter_ntarget_z1;
      if ( pnt.z < z2_edge ) ++counter_ntarget_z2;
      zvalues.insert( pnt.z );
    }
    log << "ZMIN " << z_min << " ZMAX " << z1_max << " ZEDGE " << z1_edge << " SPC " << var1.spacing() << "\n";
    for (double zz : zvalues )
    {
       log << zz << " ";
    }
    log << "\n";
    BOOST_CHECK_EQUAL( 0, var1.size() );
    // Test that some  grid points get closer than 'spacing' from edge
    // and that there are the same number on each side of z=0
    BOOST_CHECK_LT( 0, counter_ntarget_r1 );
    BOOST_CHECK_EQUAL( counter_ntarget_r1, counter_ntarget_r2 );
    BOOST_CHECK_LT( 0, counter_ntarget_z1 );
    BOOST_CHECK_EQUAL( counter_ntarget_z1, counter_ntarget_z2 );
    const std::size_t mod_2{ counter_ntarget_r2 % 2 };
    BOOST_CHECK_MESSAGE( 0 == mod_2, "2 FOLD SYMMETRY FAILURE: 0 == (counter_ntarget_r2 % 2)" );
    const std::size_t mod_4{ counter_ntarget_r2 % 4 };
    BOOST_CHECK_MESSAGE( 0 == mod_4, "4 FOLD SYMMETRY FAILURE: 0 == (counter_ntarget_r2 % 4)" );
    const std::size_t mod_8{ counter_ntarget_r2 % 8 };
    BOOST_WARN_MESSAGE( 0 == mod_8, "8 FOLD SYMMETRY FAILURE: 0 == (counter_ntarget_r2 % 8)" );
  }

}

//Simple compliance test for tubular grid
BOOST_AUTO_TEST_CASE( split_tube_grid_lifetime_test )
{
  boost::shared_ptr< boost::mt19937> generator(new boost::mt19937);
  std::stringstream log;
  utility::random_distribution rgen(generator);
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::split_tube_grid >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::split_tube_grid >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::split_tube_grid >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::split_tube_grid, geometry::split_tube_grid >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::split_tube_grid >::type {} );
  }
  std::stringstream ss;
  {
    boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
    utility::random_distribution rgen( generator );
    // Cubic grid main ctor
    boost::shared_ptr< geometry::split_tube_grid > var1( new geometry::split_tube_grid( 10.0, 31.0, 8.5, 2.5, rgen ) );
    BOOST_CHECK_EQUAL( var1->size(), 300 );
  
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << var1 );
    BOOST_CHECK_EQUAL( var1->size(), 300 );
  }
  {
    boost::shared_ptr< geometry::split_tube_grid > var1;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var1 );
    BOOST_CHECK_EQUAL( var1->size(), 300 );
  }

}

//Simple compliance test for tubular grid
BOOST_AUTO_TEST_CASE( split_tube_grid_test )
{
  boost::shared_ptr< boost::mt19937> generator(new boost::mt19937);
  std::stringstream log;
  utility::random_distribution rgen(generator);
  const double offset( 10.0 );
  // Cylindrical grid 
  // HLENGTH 31.0,
  // RADIUS  8.5
  // SPACING 0.5
  // ZGRID = (31 - .5) / .5 = 61
  // XYGRID = Y \   X
  //             \        2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x    2x
  //              \ 0     0.5   1     1.5   2     2.5   3     3.5   4     4.5   5     5.5   6     6.5   7     7.5   8
  //               \---------------------------------------------------------------------------------------------------
  //    33    0    |0     0.5   1     1.5   2     2.5   3     3.5   4     4.5   5     5.5   6     6.5   7     7.5   8.0
  // 2x 33    0.5  |0.5   0.7   1.1   1.6   2.1   2.5   3.0   3.5   4.0   4.5   5.0   5.5   6.0   6.5   7.0   7.5   8.0
  // 2x 33    1    |1     1.1   1.4   1.8   2.2   2.7   3.2   3.6   4.1   4.6   5.1   5.6   6.1   6.6   7.1   7.6   8.1
  // 2x 33    1.5  |1.5   1.6   1.8   2.1   2.5   2.9   3.4   3.8   4.3   4.7   5.2   5.7   6.2   6.7   7.2   7.6   8.1
  // 2x 33    2    |2     2.1   2.2   2.5   2.8   3.2   3.6   4.0   4.5   4.9   5.4   5.9   6.3   6.8   7.3   7.8   8.2
  // 2x 31    2.5  |2.5   2.5   2.7   2.9   3.2   3.5   3.9   4.3   4.7   5.1   5.6   6.0   6.5   7.0   7.4   7.9
  // 2x 31    3    |3     3.0   3.2   3.4   3.6   3.9   4.2   4.6   5.0   5.4   5.8   6.3   6.7   7.2   7.6   8.1
  // 2x 29    3.5  |3.5   3.5   3.6   3.8   4.0   4.3   4.6   4.9   5.3   5.7   6.1   6.5   6.9   7.4   7.8
  // 2x 29    4    |4     4.0   4.1   4.3   4.5   4.7   5.0   5.3   5.7   6.0   6.4   6.8   7.2   7.6   8.1
  // 2x 27    4.5  |4.5   4.5   4.6   4.7   4.9   5.1   5.4   5.7   6.0   6.4   6.7   7.1   7.5   7.9
  // 2x 27    5    |5     5.0   5.1   5.2   5.4   5.6   5.8   6.1   6.4   6.7   7.1   7.4   7.8   8.2
  // 2x 25    5.5  |5.5   5.5   5.6   5.7   5.9   6.0   6.3   6.5   6.8   7.1   7.4   7.8   8.1
  // 2x 23    6    |6     6.0   6.1   6.2   6.3   6.5   6.7   6.9   7.2   7.5   7.8   8.1
  // 2x 21    6.5  |6.5   6.5   6.6   6.7   6.8   7.0   7.2   7.4   7.6   7.9   8.2
  // 2x 17    7    |7     7.0   7.1   7.2   7.3   7.4   7.6   7.8   8.1
  // 2x 13    7.5  |7.5   7.5   7.6   7.6   7.8   7.9   8.1
  // 2x 5     8    |8     8.0   8.1   8.1   8.2 
  // ======
  //    861
  //
  {
    // Tubular grid ctor (with spacing)
     boost::shared_ptr< geometry::grid_generator > var1( new geometry::split_tube_grid( offset, 31.0, 8.5, 2.5 ) );
  
    BOOST_CHECK( not var1->empty() );
    // Test base class behavior
    geometry_test::test_grid_generator( var1 );
  }
  {
    // Tubular grid ctor (with target grid point)
    boost::shared_ptr< geometry::grid_generator > var1( new geometry::split_tube_grid( offset, 31.0, 8.5, 500 ) );
  
    BOOST_CHECK_LE( 500, var1->size() );
    BOOST_CHECK_LE( 0.0, var1->spacing() );
    BOOST_CHECK( not var1->empty() );
  }
  
  {
    // Tubular grid test: constructed with defined spacing
     const double length{ 31.0 };
     const double radius{ 8.5 };
     const double spacing{ 0.5 };
    geometry::split_tube_grid var1 (offset, length, radius, spacing);
    BOOST_CHECK_EQUAL(var1.size(), 861*62);
    // Should have some grid points within 3/2*'spacing' of edge
    const double r_edge{ std::pow(radius - (3*spacing/2), 2) };
    const double z1_edge{ (length - 3*spacing)/2 + offset };
    const double z2_edge{ -z1_edge };
    // should have no grid points within ('spacing'/2) of edge
    const double r_max{ std::pow(radius - spacing/2.0, 2) };
    const double z1_max{ (length - spacing)/ 2.0 + offset };
    const double z2_max{ -z1_max };
    const double z_min{ spacing / 2.0 + offset };
    // Number of grid points
    const std::size_t size{ var1.size() };
    std::set< double > zvalues;
  
    std::size_t counter_spacing_r1 = 0; // count of elements near r, when z>0
    std::size_t counter_spacing_r2 = 0; // count of elements near r, when z<0
    std::size_t counter_spacing_z1 = 0; // count of elements near +z
    std::size_t counter_spacing_z2 = 0; // count of elements near -z
  
    for (std::size_t idx = 0; idx != size; ++idx)
    {
      BOOST_CHECK_LT(0, var1.size());
      geometry::coordinate pnt;
      var1.next(pnt);
      const double r(pnt.x*pnt.x + pnt.y*pnt.y);
      BOOST_CHECK_LE( r, r_max );
      BOOST_CHECK_LE( pnt.z, z1_max );
      BOOST_CHECK_LE( z2_max, pnt.z );
      BOOST_CHECK_LE( z_min, std::abs(pnt.z) );
  
      if (r > r_edge and pnt.z > 0.0) ++counter_spacing_r1;
      if (r > r_edge and pnt.z < 0.0) ++counter_spacing_r2;
      if (pnt.z > z1_edge) ++counter_spacing_z1;
      if (pnt.z < z2_edge) ++counter_spacing_z2;
      zvalues.insert( pnt.z );
    }
    log << "ZMIN " << z_min << " ZMAX " << z1_max << " ZEDGE " << z1_edge << " SPC " << spacing << "\n";
    for (double zz : zvalues )
    {
       log << zz << " ";
    }
    log << "\n";
    BOOST_CHECK_EQUAL( 0, var1.size() );
    BOOST_CHECK_LT( 0, counter_spacing_r1 );
    BOOST_CHECK_EQUAL( counter_spacing_r1, counter_spacing_r2 );
    // r count should be multiple of 4*60
    const std::size_t mod_60{ counter_spacing_r1 % 62 };
    BOOST_CHECK_MESSAGE( 0 == mod_60, "0 == (counter_spacing_r1 % 62)" );
    const std::size_t mod_120{ counter_spacing_r1 % (2*62) };
    BOOST_CHECK_MESSAGE( 0 == mod_120, "2 FOLD SYMMETRY FAILURE: 0 == (counter_spacing_r1 % (2*62))" );
    const std::size_t mod_240{ counter_spacing_r1 % (4*62) };
    BOOST_CHECK_MESSAGE( 0 == mod_240, "4 FOLD SYMMETRY FAILURE: 0 == (counter_spacing_r1 % (4*62))" );
    const std::size_t mod_480{ counter_spacing_r1 % (8*62) };
    BOOST_WARN_MESSAGE( 0 == mod_480, "8 FOLD SYMMETRY FAILURE: 0 == (counter_spacing_r1 % (4*62))" );
  
  
    BOOST_CHECK_LT( 0, counter_spacing_z1 );
    BOOST_CHECK_EQUAL( counter_spacing_z1, counter_spacing_z2 );
    // z count should be 861
    BOOST_CHECK_EQUAL( 861, counter_spacing_z1 );
  }
  {
    // Tubular grid: constructed with target grid point minimum count
     const double length{ 31.0 };
     const double radius{ 8.5 };
     geometry::split_tube_grid var1( offset, length, radius, 500);
  
    std::set< double > zvalues;
  
    // Should have no points within (spacing/2) of edge
    const double r_max( std::pow(radius - var1.spacing()/2.0, 2) );
    const double z1_max( (length - var1.spacing())/2.0 + offset );
    const double z2_max( -z1_max );
    const double z_min( var1.spacing()/2.0 + offset );
    // Should have some points within 1.5*spacing of edge
    const double r_edge( std::pow(radius - (3*var1.spacing()/2), 2) );
    const double z1_edge( (length - 3*var1.spacing())/2 + offset );
    const double z2_edge( -z1_edge );
  
    std::size_t counter_ntarget_r1 = 0; // count of elements near r, when z>0
    std::size_t counter_ntarget_r2 = 0; // count of elements near r, when z<0
    std::size_t counter_ntarget_z1 = 0; // count of elements near +z
    std::size_t counter_ntarget_z2 = 0; // count of elements near -z
  
    geometry::coordinate pnt;
    while ( var1.next( pnt ) )
    {
      const double r( pnt.x*pnt.x + pnt.y*pnt.y );
      // Test that no grid points get closer than 'spacing'/2 from edge
      BOOST_CHECK_LE( r, r_max );
      BOOST_CHECK_LE( pnt.z, z1_max );
      BOOST_CHECK_LE( z2_max, pnt.z );
      BOOST_CHECK_LE( z_min, std::abs( pnt.z ) );// no points nearer to z=0.0 than spacing/2
  
      if ( r > r_edge and pnt.z > 0.0 ) ++counter_ntarget_r1;
      if ( r > r_edge and pnt.z < 0.0 ) ++counter_ntarget_r2;
      if ( pnt.z > z1_edge ) ++counter_ntarget_z1;
      if ( pnt.z < z2_edge ) ++counter_ntarget_z2;
      zvalues.insert( pnt.z );
    }
    log << "ZMIN " << z_min << " ZMAX " << z1_max << " ZEDGE " << z1_edge << " SPC " << var1.spacing() << "\n";
    for (double zz : zvalues )
    {
       log << zz << " ";
    }
    log << "\n";
    BOOST_CHECK_EQUAL( 0, var1.size() );
    // Test that some  grid points get closer than 'spacing' from edge
    // and that there are the same number on each side of z=0
    BOOST_CHECK_LT( 0, counter_ntarget_r1 );
    BOOST_CHECK_EQUAL( counter_ntarget_r1, counter_ntarget_r2 );
    BOOST_CHECK_LT( 0, counter_ntarget_z1 );
    BOOST_CHECK_EQUAL( counter_ntarget_z1, counter_ntarget_z2 );
    const std::size_t mod_2{ counter_ntarget_r2 % 2 };
    BOOST_CHECK_MESSAGE( 0 == mod_2, "2 FOLD SYMMETRY FAILURE: 0 == (counter_ntarget_r2 % 2)" );
    const std::size_t mod_4{ counter_ntarget_r2 % 4 };
    BOOST_CHECK_MESSAGE( 0 == mod_4, "4 FOLD SYMMETRY FAILURE: 0 == (counter_ntarget_r2 % 4)" );
    const std::size_t mod_8{ counter_ntarget_r2 % 8 };
    BOOST_WARN_MESSAGE( 0 == mod_8, "8 FOLD SYMMETRY FAILURE: 0 == (counter_ntarget_r2 % 8)" );
  }

}

//Simple compliance test for cell inserter
BOOST_AUTO_TEST_CASE( membrane_cell_inserter_lifetime_test )
{
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< geometry::membrane_cell_inserter >::type {} );
    BOOST_CHECK( std::is_copy_constructible< geometry::membrane_cell_inserter >::type {} );
    BOOST_CHECK( std::is_move_constructible< geometry::membrane_cell_inserter >::type {} );
    BOOST_CHECK( ( std::is_assignable< geometry::membrane_cell_inserter, geometry::membrane_cell_inserter >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< geometry::membrane_cell_inserter >::type {} );
  }
  std::stringstream store;
  const double sphere_radius{ 1.0 };
  const double cell_hlength{ 30.0 };
  const double cell_radius{ 30.0 };
  const double channel_hlength{ 10.0 };
  const double channel_radius{ 3.0 };
  const double arc_radius{ 5.0 };
  {
  // Circle grid default ctor
    geometry::membrane_cell_inserter var1{ sphere_radius,
                                           cell_radius,
                                           cell_hlength,
                                           channel_radius,
                                           channel_hlength,
                                           arc_radius };
    // Default should have no size
    BOOST_CHECK_EQUAL( var1.radius(), sphere_radius );
    BOOST_CHECK_EQUAL( var1.reduced_cell_radius(), cell_radius - sphere_radius );
    BOOST_CHECK_EQUAL( var1.cell_offset(), channel_hlength + sphere_radius );
    BOOST_CHECK_EQUAL( var1.reduced_channel_radius(), channel_radius - sphere_radius );
    BOOST_CHECK_EQUAL( var1.vestibule_outer_radius(), channel_radius + arc_radius );
    BOOST_CHECK_EQUAL( var1.vestibule_radius(), sphere_radius + arc_radius );
    BOOST_CHECK_EQUAL( var1.vestibule_offset(), channel_hlength - arc_radius );
    // Volumes
    BOOST_CHECK_EQUAL( var1.compartment_factor(), (std::pow( var1.reduced_cell_radius(), 2 ) * core::constants::pi() * ( cell_hlength - 2 * sphere_radius ) ) );
    BOOST_CHECK_EQUAL( var1.inner_channel_factor(),  (std::pow( var1.reduced_channel_radius(), 2 ) * core::constants::pi() * ( channel_hlength - arc_radius ) ) );
    BOOST_CHECK_EQUAL( var1.vestibule_factor(), var1.volume_of_rotation( channel_radius + arc_radius, sphere_radius + arc_radius ) );
  
    {
      geometry::membrane_cell_inserter var2( var1 );
      BOOST_CHECK( var1 == var2 );
    }
    {
      geometry::membrane_cell_inserter var3;
      var3 = var1;
      BOOST_CHECK( var1 == var3 );
    }
    {
      geometry::membrane_cell_inserter var3( var1 );
      geometry::membrane_cell_inserter var5( std::move( var3 ) );
      BOOST_CHECK( var1 == var5 );
    }
  
    boost::archive::text_oarchive oa( store );
    // write class instance to archive
    oa << var1;
    BOOST_CHECK_EQUAL( var1.radius(), sphere_radius );
  }
  {
    geometry::membrane_cell_inserter var1;
    boost::archive::text_iarchive ia( store );
    // read class instance from archive
    ia >> var1;
    BOOST_CHECK_EQUAL( var1.radius(), sphere_radius );
    BOOST_CHECK_EQUAL( var1.reduced_cell_radius(), cell_radius - sphere_radius );
    BOOST_CHECK_EQUAL( var1.cell_offset(), channel_hlength + sphere_radius );
    BOOST_CHECK_EQUAL( var1.reduced_channel_radius(), channel_radius - sphere_radius );
    BOOST_CHECK_EQUAL( var1.vestibule_outer_radius(), channel_radius + arc_radius );
    BOOST_CHECK_EQUAL( var1.vestibule_radius(), sphere_radius + arc_radius );
    BOOST_CHECK_EQUAL( var1.vestibule_offset(), channel_hlength - arc_radius );
    // Volumes
    BOOST_CHECK_EQUAL( var1.compartment_factor(), (std::pow( var1.reduced_cell_radius(), 2 ) * core::constants::pi() * ( cell_hlength - 2 * sphere_radius ) ) );
    BOOST_CHECK_EQUAL( var1.inner_channel_factor(),  (std::pow( var1.reduced_channel_radius(), 2 ) * core::constants::pi() * ( channel_hlength - arc_radius ) ) );
    BOOST_CHECK_EQUAL( var1.vestibule_factor(), var1.volume_of_rotation( channel_radius + arc_radius, sphere_radius + arc_radius ) );
  }

}

// Should text volume of rotation. generate_position tested in geom_dist_exec.
BOOST_AUTO_TEST_CASE( membrane_cell_inserter_methods_test )
{

}

BOOST_AUTO_TEST_CASE( periodic_cube_region_lifetime_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::periodic_cube_region >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::periodic_cube_region >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::periodic_cube_region >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::periodic_cube_region, geometry::periodic_cube_region >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::periodic_cube_region >::type {} );
  }
  std::stringstream ss;
  {
    // Cubic grid main ctor
    boost::shared_ptr< geometry::periodic_cube_region > var1( new geometry::periodic_cube_region( "bulk", 10.0 ) );
    BOOST_CHECK_EQUAL( var1->length(), 10.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK_EQUAL( var1->volume( 1.0 ), 1000.0 );
  
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << var1 );
  }
  {
    boost::shared_ptr< geometry::periodic_cube_region > var1;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var1 );
    BOOST_CHECK_EQUAL( var1->length(), 10.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK_EQUAL( var1->volume( 1.0 ), 1000.0 );
  }

}

BOOST_AUTO_TEST_CASE( periodic_cube_region_method_test )
{
  const std::string label( "bulk" );
  // Cubic grid main ctor
  boost::shared_ptr< geometry::periodic_cube_region > var1( new geometry::periodic_cube_region( label, 10.0 ) );
  BOOST_CHECK_EQUAL( var1->length(), 10.0 );
  
  {
    geometry::coordinate inside( 0.0, 10.0, 10.0 );
    geometry::coordinate outside( 0.0, -0.1, 10.0 );
    double okradius( 1.0 );
    double bigradius( 10.1 );
    // tests fits, is_inside, new_position and new_position_offset
    geometry_test::region_new_position_test( *var1, inside, outside, okradius, bigradius );
    geometry_test::region_make_gridder_test( *var1, 20, 10000, 0.1, 9.0, true );
    geometry_test::region_serialization_test( var1 );
  }
  // Test volume (should be the same regardless of size)
  {
    BOOST_CHECK_EQUAL( var1->volume( 1.0 ), 1000.0 );
    BOOST_CHECK_EQUAL( var1->volume( 0.0 ), 1000.0 );
    // should throw on non-fitting radius
    const std::string fncall{ "var1->volume( 100.0 )" };
    try
    {
      var1->volume( 100.0 );
      BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Sphere must fit in space to be able to calculate the volume" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
    }
  }
  // label / set_label
  {
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK_NO_THROW( var1->set_label( "100" ) );
    BOOST_CHECK_EQUAL( var1->label(), "100" );
    BOOST_CHECK_NO_THROW( var1->set_label( label ) );
    BOOST_CHECK_EQUAL( var1->label(), label );
  }
  // Calculate distances
  {
    // Uses wrapping
    geometry::coordinate_set pnts;
    std::vector< double > target, result;
    pnts.resize( 10 );
    // First five points effectively wrap to origin
    pnts.set_x( 0, 0.0 );
    pnts.set_y( 0, 0.0 );
    pnts.set_z( 0 , 0.0 );
    pnts.set_x( 1, 10.0 );
    pnts.set_y( 1, 0.0 );
    pnts.set_z( 1 , 0.0 );
    pnts.set_x( 2, 0.0 );
    pnts.set_y( 2, 10.0 );
    pnts.set_z( 2 , 0.0 );
    pnts.set_x( 3, 0.0 );
    pnts.set_y( 3, 0.0 );
    pnts.set_z( 3 , 10.0 );
    pnts.set_x( 4, 10.0 );
    pnts.set_y( 4, 10.0 );
    pnts.set_z( 4 , 10.0 );
    pnts.set_x( 5, 1.0 );
    pnts.set_y( 5, 0.0 );
    pnts.set_z( 5 , 0.0 );
    pnts.set_x( 6, 0.0 );
    pnts.set_y( 6, 1.0 );
    pnts.set_z( 6 , 1.0 );
    pnts.set_x( 7, 1.0 );
    pnts.set_y( 7, 1.0 );
    pnts.set_z( 7 , 1.0 );
    pnts.set_x( 8, 2.0 );
    pnts.set_y( 8, 2.0 );
    pnts.set_z( 8 , 2.0 );
    pnts.set_x( 9, 5.0 );
    pnts.set_y( 9, 5.0 );
    pnts.set_z( 9 , 5.0 );
  
    target.resize( 10 );
    target[0] = 5.0;
    target[1] = 5.0;
    target[2] = 5.0;
    target[3] = 5.0;
    target[4] = 5.0;
    target[5] = std::sqrt( 26.0 );
    target[6] = std::sqrt( 17.0 );
    target[7] = std::sqrt( 18.0 );
    target[8] = std::sqrt( 17.0 );
    target[9] = std::sqrt( 50.0 );
  
    geometry::geometry_manager man( var1 );
  
    {
      geometry::coordinate newpos( 0.0, 5.0, 0.0 );
      man.calculate_distances( newpos, pnts, result, 0, 10 );
      BOOST_REQUIRE_EQUAL( result.size(), 10 );
      for( std::size_t ii = 0; ii != 10; ++ii )
      {
        BOOST_CHECK( utility::feq( result[ii], target[ii] ) );
        geometry::coordinate otherpos( pnts.x( ii ), pnts.y( ii ), pnts.z( ii ) );
        BOOST_CHECK( utility::feq( man.calculate_distance_squared( newpos, otherpos ), std::pow( target[ii], 2 ) ) );
      }
    }
    {
      // Same position as before but wrapped in Y direction
      geometry::coordinate newpos( 0.0, 15.0, 0.0 );
      man.calculate_distances( newpos, pnts, result, 0, 10 );
      BOOST_REQUIRE_EQUAL( result.size(), 10 );
      for( std::size_t ii = 0; ii != 10; ++ii )
      {
        BOOST_CHECK( utility::feq( result[ii], target[ii] ) );
        geometry::coordinate otherpos( pnts.x( ii ), pnts.y( ii ), pnts.z( ii ) );
        BOOST_CHECK( utility::feq( man.calculate_distance_squared( newpos, otherpos ), std::pow( target[ii], 2 ) ) );
      }
    }
    {
      // Same position as before but wrapped in X direction
      geometry::coordinate newpos( 10.0, 5.0, 0.0 );
      man.calculate_distances( newpos, pnts, result, 0, 10 );
      BOOST_REQUIRE_EQUAL( result.size(), 10 );
      for( std::size_t ii = 0; ii != 10; ++ii )
      {
        BOOST_CHECK( utility::feq( result[ii], target[ii] ) );
        geometry::coordinate otherpos( pnts.x( ii ), pnts.y( ii ), pnts.z( ii ) );
        BOOST_CHECK( utility::feq( man.calculate_distance_squared( newpos, otherpos ), std::pow( target[ii], 2 ) ) );
      }
    }
    {
      // Same position as before but wrapped in Z direction
      geometry::coordinate newpos( 0.0, 5.0, 10.0 );
      man.calculate_distances( newpos, pnts, result, 0, 10 );
      BOOST_REQUIRE_EQUAL( result.size(), 10 );
      for( std::size_t ii = 0; ii != 10; ++ii )
      {
        BOOST_CHECK( utility::feq( result[ii], target[ii] ) );
        geometry::coordinate otherpos( pnts.x( ii ), pnts.y( ii ), pnts.z( ii ) );
        BOOST_CHECK( utility::feq( man.calculate_distance_squared( newpos, otherpos ), std::pow( target[ii], 2 ) ) );
      }
    }
  }
  // change volume
  {
    geometry::geometry_manager man( var1 );
    BOOST_CHECK_EQUAL( var1->volume( 0.0 ), 1000.0 );
  
    BOOST_REQUIRE_NO_THROW( man.change_volume( 900.0, 3.0 ) );
    BOOST_CHECK( utility::feq( var1->volume( 3.0 ), 900.0 ) );
    BOOST_CHECK_EQUAL( var1->volume( 3.0 ), 900.0 );
  
    const std::string fncall{ "man.change_volume( 729.0, 5.0 )" };
    try
    {
      man.change_volume( 729.0, 5.0 );
      BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Object can not fit in volume after volume change" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
    }
    // test make_gridder after volume change
    geometry_test::region_make_gridder_test( *var1, 1, 500, 0.5, 11.0, true );
  }
  

}

BOOST_AUTO_TEST_CASE( cube_region_lifetime_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::cube_region >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::cube_region >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::cube_region >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::cube_region, geometry::cube_region >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::cube_region >::type {} );
  }
  std::stringstream ss;
  geometry::coordinate origin( 1.0, -2.0, 3.0 );
  {
    // Cubic grid main ctor
    boost::shared_ptr< geometry::cube_region > var1( new geometry::cube_region( "bulk", 10.0, origin, true ) );
    BOOST_CHECK_EQUAL( var1->length(), 10.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK( var1->is_open() );
    BOOST_CHECK_EQUAL( var1->origin(), origin );
  
    geometry_test::region_serialization_test( var1 );
  
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << var1 );
  }
  {
    boost::shared_ptr< geometry::cube_region > var1;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var1 );
    BOOST_CHECK_EQUAL( var1->length(), 10.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK( var1->is_open() );
    BOOST_CHECK_EQUAL( var1->origin(), origin );
  }

}

// Test of methods when cube is set to open
BOOST_AUTO_TEST_CASE( cube_region_method_open_test )
{
  const std::string label( "bulk" );
  const geometry::coordinate origin( 1.0, -1.0, -2.0 );
  // OPEN cube
  {
  // Cubic grid main ctor
    boost::shared_ptr< geometry::cube_region > var1( new geometry::cube_region( label, 10.0, origin, true ) );
    BOOST_CHECK_EQUAL( var1->length(), 10.0 );
  
    {
      geometry::coordinate inside( 1.0, 8.0, 8.0 );
      geometry::coordinate outside( 1.0, -1.1, 8.0 );
      double okradius( 1.0 );
      double bigradius( -1.0 ); // negative number -> any radius fits
      // tests fits, is_inside, new_position and new_position_offset
      geometry_test::region_new_position_test( *var1, inside, outside, okradius, bigradius );
      geometry_test::region_make_gridder_test( *var1, 20, 10000, 0.1, 9.0, false );
    }
    // Test volume (cube is open so should be the same regardless of size)
    {
      BOOST_CHECK_EQUAL( var1->volume( 1.0 ), 1000.0 );
      BOOST_CHECK_EQUAL( var1->volume( 0.0 ), 1000.0 );
      // Any radius fits
      BOOST_CHECK_EQUAL( var1->volume( 100.0 ), 1000.0 );
    }
    // label / set_label
    {
      BOOST_CHECK_EQUAL( var1->label(), "bulk" );
      BOOST_CHECK_NO_THROW( var1->set_label( "100" ) );
      BOOST_CHECK_EQUAL( var1->label(), "100" );
      BOOST_CHECK_NO_THROW( var1->set_label( label ) );
      BOOST_CHECK_EQUAL( var1->label(), label );
    }
    // Calculate distances
    {
      geometry::coordinate_set pnts;
      std::vector< double > target, result;
      pnts.resize( 10 );
      pnts.set_x( 0, 0.0 );
      pnts.set_y( 0, 0.0 );
      pnts.set_z( 0, 0.0 );
      pnts.set_x( 1, 10.0 );
      pnts.set_y( 1, 0.0 );
      pnts.set_z( 1, 0.0 );
      pnts.set_x( 2, 0.0 );
      pnts.set_y( 2, 10.0 );
      pnts.set_z( 2, 0.0 );
      pnts.set_x( 3, 0.0 );
      pnts.set_y( 3, 0.0 );
      pnts.set_z( 3, 10.0 );
      pnts.set_x( 4, 10.0 );
      pnts.set_y( 4, 10.0 );
      pnts.set_z( 4, 10.0 );
      pnts.set_x( 5, 1.0 );
      pnts.set_y( 5, 0.0 );
      pnts.set_z( 5, 0.0 );
      pnts.set_x( 6, 0.0 );
      pnts.set_y( 6, 1.0 );
      pnts.set_z( 6, 1.0 );
      pnts.set_x( 7, 1.0 );
      pnts.set_y( 7, 1.0 );
      pnts.set_z( 7, 1.0 );
      pnts.set_x( 8, 2.0 );
      pnts.set_y( 8, 2.0 );
      pnts.set_z( 8, 2.0 );
      pnts.set_x( 9, 5.0 );
      pnts.set_y( 9, 5.0 );
      pnts.set_z( 9, 5.0 );
  
      target.resize( 10 );
      geometry::coordinate newpos( 0.0, 5.0, 0.0 );
      target[0] = 5.0;
      target[1] = std::sqrt( 125.0 );
      target[2] = 5.0;
      target[3] = std::sqrt( 125.0 );
      target[4] = std::sqrt( 225.0 );
      target[5] = std::sqrt( 26.0 );
      target[6] = std::sqrt( 17.0 );
      target[7] = std::sqrt( 18.0 );
      target[8] = std::sqrt( 17.0 );
      target[9] = std::sqrt( 50.0 );
  
      geometry::geometry_manager man( var1 );
  
      man.calculate_distances( newpos, pnts, result, 0, 10 );
      BOOST_REQUIRE_EQUAL( result.size(), 10 );
      for( std::size_t ii = 0; ii != 10; ++ii )
      {
        BOOST_CHECK( utility::feq( result[ii], target[ii] ) );
        geometry::coordinate otherpos( pnts.x( ii ), pnts.y( ii ), pnts.z( ii ) );
        BOOST_CHECK( utility::feq( man.calculate_distance_squared( newpos, otherpos ), std::pow( target[ii], 2 ) ) );
      }
    }
  
    // change volume
    geometry::geometry_manager man( var1 );
    BOOST_CHECK_EQUAL( var1->volume( 0.0 ), 1000.0 );
  
    BOOST_REQUIRE_NO_THROW( man.change_volume( 900.0, 3.0 ) );
    BOOST_CHECK( utility::feq( var1->volume( 3.0 ), 900.0 ) );
    // No radius should be too large for open cube.
    BOOST_CHECK_NO_THROW( man.change_volume( 729.0, 5.0 ) );
  
    // make_gridder after volume change
    geometry_test::region_make_gridder_test( *var1, 1, 500, 0.5, 3.0, false );
  }
  
  
  

}

// Test of methods when cube is set to closed
BOOST_AUTO_TEST_CASE( cube_region_method_closed_test )
{
  const std::string label( "bulk" );
  const geometry::coordinate origin( 1.0, -1.0, -2.0 );
  // OPEN cube
  {
  // Cubic grid main ctor
    boost::shared_ptr< geometry::cube_region > var1( new geometry::cube_region( label, 10.0, origin, false ) );
    BOOST_CHECK_EQUAL( var1->length(), 10.0 );
  
    {
      geometry::coordinate inside( 2.0, 7.0, 5.0 );
      geometry::coordinate outside( 2.0, -0.1, 5.0 );
      double okradius( 1.0 );
      double bigradius( 10.1 );
      // tests fits, is_inside, new_position and new_position_offset
      geometry_test::region_new_position_test( *var1, inside, outside, okradius, bigradius );
      geometry_test::region_make_gridder_test( *var1, 20, 10000, 0.1, 9.0, true );
    }
    // Test volume (cube is open so should be the same regardless of size)
    {
      BOOST_CHECK_EQUAL( var1->volume( 1.0 ), std::pow( 8, 3 ) );
      BOOST_CHECK_EQUAL( var1->volume( 0.0 ), 1000.0 );
      // should throw on non-fitting radius
      const std::string fncall
      { "var1->volume( 10.1 )"
      };
      try
      {
        var1->volume( 10.1 );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Sphere must fit in space to be able to calculate the volume" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    // label / set_label
    {
      BOOST_CHECK_EQUAL( var1->label(), "bulk" );
      BOOST_CHECK_NO_THROW( var1->set_label( "100" ) );
      BOOST_CHECK_EQUAL( var1->label(), "100" );
      BOOST_CHECK_NO_THROW( var1->set_label( label ) );
      BOOST_CHECK_EQUAL( var1->label(), label );
    }
    // Calculate distances
    {
      geometry::coordinate_set pnts;
      std::vector< double > target, result;
      pnts.resize( 10 );
      pnts.set_x( 0, 0.0 );
      pnts.set_y( 0, 0.0 );
      pnts.set_z( 0, 0.0 );
      pnts.set_x( 1, 10.0 );
      pnts.set_y( 1, 0.0 );
      pnts.set_z( 1, 0.0 );
      pnts.set_x( 2, 0.0 );
      pnts.set_y( 2, 10.0 );
      pnts.set_z( 2, 0.0 );
      pnts.set_x( 3, 0.0 );
      pnts.set_y( 3, 0.0 );
      pnts.set_z( 3, 10.0 );
      pnts.set_x( 4, 10.0 );
      pnts.set_y( 4, 10.0 );
      pnts.set_z( 4, 10.0 );
      pnts.set_x( 5, 1.0 );
      pnts.set_y( 5, 0.0 );
      pnts.set_z( 5, 0.0 );
      pnts.set_x( 6, 0.0 );
      pnts.set_y( 6, 1.0 );
      pnts.set_z( 6, 1.0 );
      pnts.set_x( 7, 1.0 );
      pnts.set_y( 7, 1.0 );
      pnts.set_z( 7, 1.0 );
      pnts.set_x( 8, 2.0 );
      pnts.set_y( 8, 2.0 );
      pnts.set_z( 8, 2.0 );
      pnts.set_x( 9, 5.0 );
      pnts.set_y( 9, 5.0 );
      pnts.set_z( 9, 5.0 );
  
      target.resize( 10 );
      geometry::coordinate newpos( 0.0, 5.0, 0.0 );
      target[0] = 5.0;
      target[1] = std::sqrt( 125.0 );
      target[2] = 5.0;
      target[3] = std::sqrt( 125.0 );
      target[4] = std::sqrt( 225.0 );
      target[5] = std::sqrt( 26.0 );
      target[6] = std::sqrt( 17.0 );
      target[7] = std::sqrt( 18.0 );
      target[8] = std::sqrt( 17.0 );
      target[9] = std::sqrt( 50.0 );
  
      geometry::geometry_manager man( var1 );
  
      man.calculate_distances( newpos, pnts, result, 0, 10 );
      BOOST_REQUIRE_EQUAL( result.size(), 10 );
      for( std::size_t ii = 0; ii != 10; ++ii )
      {
        BOOST_CHECK( utility::feq( result[ii], target[ii] ) );
        geometry::coordinate otherpos( pnts.x( ii ), pnts.y( ii ), pnts.z( ii ) );
        BOOST_CHECK( utility::feq( man.calculate_distance_squared( newpos, otherpos ), std::pow( target[ii], 2 ) ) );
      }
    }
    // change volume
    geometry::geometry_manager man( var1 );
    BOOST_CHECK_EQUAL( var1->volume( 0.0 ), 1000.0 );
  
    BOOST_REQUIRE_NO_THROW( man.change_volume( 900.0, 3.0 ) );
    BOOST_CHECK( utility::feq( var1->volume( 3.0 ), 900.0 ) );
  
    BOOST_REQUIRE_NO_THROW( man.change_volume( 729.0, 5.0 ) );
    BOOST_CHECK( utility::feq( var1->volume( 5.0 ), 729.0 ) );
  
    // make_gridder after volume change
    geometry_test::region_make_gridder_test( *var1, 1, 500, 0.5, 3.0, true );
  }
  // make_gridder: see cube_region_method_open_test as gridder generation is
  // always independent of open/closedness.

}

BOOST_AUTO_TEST_CASE( cylinder_region_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::cylinder_region >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::cylinder_region >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::cylinder_region >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::cylinder_region, geometry::cylinder_region >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::cylinder_region >::type {} );
  }
  std::stringstream ss;
  {
    // Cubic grid main ctor
    boost::shared_ptr< geometry::cylinder_region > var1( new geometry::cylinder_region( "bulk", 10.0, 5.0 ) );
    BOOST_CHECK_EQUAL( var1->radius(), 10.0 );
    BOOST_CHECK_EQUAL( var1->half_length(), 5.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK( utility::feq( var1->volume( 0.0 ), 1000*core::constants::pi() ) );
    BOOST_CHECK_CLOSE( var1->volume( 1.0 ), 9*9*8*core::constants::pi(), 0.00000001 );
    BOOST_CHECK( utility::feq( var1->volume( 1.0 ), 9*9*8*core::constants::pi() ) );
  
    {
      geometry::coordinate inside( 1.0, 7.0, 2.9 );
      geometry::coordinate outside( 9.9, -1.1, 3.9 );
      double okradius( 1.0 );
      double bigradius( 10.1 );
      geometry_test::region_serialization_test( var1 );
      geometry_test::region_new_position_test( *var1, inside, outside, okradius, bigradius );
      geometry_test::region_make_gridder_test( *var1, 20, 10000, 0.1, 9.0, true );
    }
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << var1 );
  }
  {
    boost::shared_ptr< geometry::cylinder_region > var1;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var1 );
    BOOST_CHECK_EQUAL( var1->radius(), 10.0 );
    BOOST_CHECK_EQUAL( var1->half_length(), 5.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK( utility::feq( var1->volume( 0.0 ), 1000*core::constants::pi() ) );
  }

}

BOOST_AUTO_TEST_CASE( open_cylinder_region_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::open_cylinder_region >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::open_cylinder_region >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::open_cylinder_region >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::open_cylinder_region, geometry::open_cylinder_region >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::open_cylinder_region >::type {} );
  }
  std::stringstream ss;
  {
    // Cubic grid main ctor
    boost::shared_ptr< geometry::open_cylinder_region > var1( new geometry::open_cylinder_region( "bulk", 10.0, 5.0 ) );
    BOOST_CHECK_EQUAL( var1->radius(), 10.0 );
    BOOST_CHECK_EQUAL( var1->half_length(), 5.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK( utility::feq( var1->volume( 0.0 ), 1000*core::constants::pi() ) );
    BOOST_CHECK( utility::feq( var1->volume( 1.0 ), 10*9*9*core::constants::pi() ) );
  
    {
      geometry::coordinate inside( 1.0, 8.0, 4.9 );
      geometry::coordinate outside( 9.9, -1.1, 3.0 );
      double okradius( 1.0 );
      double bigradius( 10.1 );
      geometry_test::region_serialization_test( var1 );
      geometry_test::region_new_position_test( *var1, inside, outside, okradius, bigradius );
      geometry_test::region_make_gridder_test( *var1, 20, 10000, 0.1, 9.0, false );
    }
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << var1 );
  }
  {
    boost::shared_ptr< geometry::open_cylinder_region > var1;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var1 );
    BOOST_CHECK_EQUAL( var1->radius(), 10.0 );
    BOOST_CHECK_EQUAL( var1->half_length(), 5.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK( utility::feq( var1->volume( 0.0 ), 1000*core::constants::pi() ) );
  }

}

BOOST_AUTO_TEST_CASE( open_split_cylinder_region_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::open_split_cylinder_region >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::open_split_cylinder_region >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::open_split_cylinder_region >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::open_split_cylinder_region, geometry::open_split_cylinder_region >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::open_split_cylinder_region >::type {} );
  }
  std::stringstream ss;
  {
    // Cubic grid main ctor
    boost::shared_ptr< geometry::open_split_cylinder_region > var1( new geometry::open_split_cylinder_region( "bulk", 10.0, 5.0, 4.0 ) );
    BOOST_CHECK_EQUAL( var1->radius(), 10.0 );
    BOOST_CHECK_EQUAL( var1->half_length(), 5.0 );
    BOOST_CHECK_EQUAL( var1->offset(), 4.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK( utility::feq( var1->volume( 0.0 ), 1000*core::constants::pi() ) );
    BOOST_CHECK( utility::feq( var1->volume( 1.0 ), 1000*core::constants::pi() ) );
  
    {
      geometry::coordinate inside( 5.9, 8.0, 8.9 );
      geometry::coordinate outside( 7.9, -1.1, 3.9 );
      double okradius( 1.0 );
      double bigradius( -1.0 );
      geometry_test::region_serialization_test( var1 );
      geometry_test::region_new_position_test( *var1, inside, outside, okradius, bigradius );
      geometry_test::region_make_gridder_test( *var1, 20, 10000, 0.1, 9.0, false );
    }
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    BOOST_REQUIRE_NO_THROW( oa << var1 );
  }
  {
    boost::shared_ptr< geometry::open_split_cylinder_region > var1;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    BOOST_REQUIRE_NO_THROW( ia >> var1 );
    BOOST_CHECK_EQUAL( var1->radius(), 10.0 );
    BOOST_CHECK_EQUAL( var1->half_length(), 5.0 );
    BOOST_CHECK_EQUAL( var1->offset(), 4.0 );
    BOOST_CHECK_EQUAL( var1->label(), "bulk" );
    BOOST_CHECK( utility::feq( var1->volume( 0.0 ), 1000*core::constants::pi() ) );
  }

}

BOOST_AUTO_TEST_CASE( geometry_manager_lifetime_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::geometry_manager >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::geometry_manager >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::geometry_manager >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::geometry_manager, geometry::geometry_manager >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< geometry::geometry_manager >::type {} );
  }
  {
    boost::shared_ptr< geometry::cube_region > regn( new geometry::cube_region( "cell", 1.0, { 0.0, 0.0, 0.0 }, false ) );
    // main ctor
    boost::shared_ptr< geometry::geometry_manager > dobj( new geometry::geometry_manager( regn ) );
    BOOST_CHECK_EQUAL( dobj->region_count(), 1 );
    BOOST_CHECK_EQUAL( dobj->system_region().label(), "cell" );
  }

}

// Tested:
//   add_region
//   get_region
//   locate_region
//   region_count
//   region_key
//   serialize
//   system_region
//   build_input_delegater
//Not tested as they simply forward to system region (see
//region methods for tests of these methods)
//   calculate_distances
//   calculate_distance_squared
//   change_volume
// Not directly tested:
//   write_document

BOOST_AUTO_TEST_CASE( geometry_manager_method_test )
{
  std::stringstream store;
  // Test some basic geometry_manager methods
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    BOOST_CHECK_EQUAL( mngr->region_count(), 1ul );
    BOOST_CHECK_EQUAL( mngr->region_key( "cell" ), 0ul );
    BOOST_CHECK_EQUAL( mngr->get_region( 0 ).label(), "cell" );
    BOOST_CHECK_EQUAL( mngr->system_region().label(), "cell" );
  
    core::input_delegater dg( 1 );
    geometry::geometry_manager::build_input_delegater( mngr, dg );
    {
      core::input_help hlpr;
      dg.get_documentation( hlpr );
      {
        std::stringstream ss;
        hlpr.write( ss );
        BOOST_CHECK( ss.str().find( "name" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "type" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "open" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "origin" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "width" ) < ss.str().size() );
      }
    }
  
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    BOOST_CHECK_EQUAL( mngr->region_count(), 2ul );
    BOOST_CHECK_EQUAL( mngr->region_key( "bulk" ), 1ul );
    BOOST_CHECK_EQUAL( mngr->get_region( 1 ).label(), "bulk" );
  
    std::vector< boost::shared_ptr< geometry::base_region > > gvect;
    geometry::coordinate canon_pos1( 1.1, 1.1, 1.1 );
    mngr->locate_region( canon_pos1, 0.0, gvect );
    BOOST_CHECK_EQUAL( gvect.size(), 2ul );
    BOOST_CHECK_EQUAL( gvect.at( 0 )->label(), "cell" );
  
    geometry::coordinate canon_pos2( 0.9, 0.9, 0.9 );
    mngr->locate_region( canon_pos2, 0.0, gvect );
    BOOST_CHECK_EQUAL( gvect.size(), 1ul );
    BOOST_CHECK_EQUAL( gvect.at( 0 )->label(), "cell" );
  
    boost::archive::text_oarchive oa( store );
    oa << mngr;
  }
  {
    boost::shared_ptr< geometry::geometry_manager > mngr;
    boost::archive::text_iarchive ia( store );
    ia >> mngr;
  
    BOOST_CHECK_EQUAL( mngr->region_count(), 2ul );
    BOOST_CHECK_EQUAL( mngr->region_key( "cell" ), 0ul );
    BOOST_CHECK_EQUAL( mngr->get_region( 0 ).label(), "cell" );
    BOOST_CHECK_EQUAL( mngr->region_key( "bulk" ), 1ul );
    BOOST_CHECK_EQUAL( mngr->get_region( 1 ).label(), "bulk" );
    BOOST_CHECK_EQUAL( mngr->system_region().label(), "cell" );
  
    std::string canon_cell("region\nname cell\ntype periodic-cube\nwidth 10.000000\nend\n");
    std::string canon_bulk("region\nname bulk\nopen true\norigin 1 1 1\ntype cube\nwidth 8.000000\nend\n");
  
    core::input_document wrtr( 1ul );
    mngr->write_document( wrtr );
    std::stringstream idoc;
    wrtr.write( idoc );
    const std::string sdoc( idoc.str() );
    BOOST_CHECK_LE( sdoc.find( canon_cell ), sdoc.size() );
    BOOST_CHECK_LE( sdoc.find( canon_bulk ), sdoc.size() );
  }

}

BOOST_AUTO_TEST_CASE( region_meta_lifetime_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< geometry::region_meta >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< geometry::region_meta >::type {} );
    BOOST_CHECK( not std::is_move_constructible< geometry::region_meta >::type {} );
    BOOST_CHECK( not( std::is_assignable< geometry::region_meta, geometry::region_meta >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< geometry::region_meta >::type {} );
  }
  {
    boost::shared_ptr< geometry::geometry_manager > man;
    // main ctor
    boost::shared_ptr< geometry::region_meta > dobj( new geometry::region_meta( man ) );
    BOOST_CHECK_EQUAL( dobj->section_label(), core::strngs::fsregn() );
    BOOST_CHECK_EQUAL( dobj->multiple(), true );
    BOOST_CHECK_EQUAL( dobj->required(), false );
  }
  

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( region_meta_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      core::input_help hlpr;
      m->publish_help( hlpr );
      {
        std::stringstream ss;
        hlpr.write( ss );
        BOOST_CHECK( ss.str().find( "name" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "type" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "open" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "origin" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "width" ) < ss.str().size() );
      }
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    BOOST_CHECK_EQUAL( mngr->region_count(), 2ul );
    BOOST_CHECK_EQUAL( mngr->region_key( "bulk" ), 1ul );
  
    geometry::cube_region const* regptr;
    regptr = dynamic_cast< geometry::cube_region const* >( &( mngr->get_region( 1ul ) ) );
    BOOST_REQUIRE( regptr != nullptr );
    BOOST_CHECK_EQUAL( regptr->is_open(), true );
    BOOST_CHECK_EQUAL( regptr->length(), 8.0 );
    geometry::coordinate canon_pos( 1.0, 1.0, 1.0 );
    BOOST_CHECK_EQUAL( regptr->origin(), canon_pos );
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_region_name_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Name is hidden by comment character
    std::string canon_input( "region\nname #\"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Region label parameter \"name\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>name #\"bulk\"<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_region_no_name_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Name is hidden by comment character
    std::string canon_input( "region\n#name \"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Geometry section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: name **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_region_repeated_name_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\ntype cube\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\nname channel\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Parameter \"name\" appears more than once in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>name channel<<\n** Parameter \"name\" can only appear once per section. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_region_no_type_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\n#type cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Geometry section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: type **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_region_type_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\ntype #cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Region subtype parameter \"type\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>type #cube<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_region_type_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cubic\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Region subtype parameter \"type\" with value (cubic) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>type cubic<<\n** A value from the list (cube) was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_region_repeated_type_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\ntype cube\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth 8.0\ntype periodic_cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Parameter \"type\" appears more than once in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>type periodic_cube<<\n** Parameter \"type\" can only appear once per section. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_no_width_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\n#width 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: width **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_width_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth #8.0\ntype cube\nend\n\n" );
  
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
      const std::string canon{ "Cube size parameter \"width\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>width #8.0<<\n** Expected a value. **" };
      //std::cout << canon << "\n";
      BOOST_CHECK( msg.find( canon ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_width_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth eight\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube size parameter \"width\" with value (eight) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>width eight<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_width_poor_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth 0.8 pm\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube size parameter \"width\" with value (0.8 pm) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>width 0.8 pm<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_width_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\norigin 1.0 1.0 1.0\nwidth -1.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube size parameter \"width\" with value (-1.0) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>width -1.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_no_origin_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\n#origin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: origin **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_origin_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\norigin #1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube location parameter \"origin\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>origin #1.0 1.0 1.0<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_origin_short_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\norigin 1.0 1.0 #1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube location parameter \"origin\" with value (1.0 1.0) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>origin 1.0 1.0 #1.0<<\n** Ensure coordinate data has exactly three numeric values. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_origin_long_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\norigin 1.0 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube location parameter \"origin\" with value (1.0 1.0 1.0 1.0) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>origin 1.0 1.0 1.0 1.0<<\n** Ensure coordinate data has exactly three numeric values. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_origin_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\norigin 1.0 1.0 l.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube location parameter \"origin\" with value (1.0 1.0 l.0) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>origin 1.0 1.0 l.0<<\n** Element l.0: A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_open_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\nopen\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    geometry::cube_region const* regptr;
    regptr = dynamic_cast< geometry::cube_region const* >( &( mngr->get_region( 1ul ) ) );
    BOOST_REQUIRE( regptr != nullptr );
    BOOST_CHECK_EQUAL( regptr->is_open(), true );
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_open_false_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\nopen false\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    geometry::cube_region const* regptr;
    regptr = dynamic_cast< geometry::cube_region const* >( &( mngr->get_region( 1ul ) ) );
    BOOST_REQUIRE( regptr != nullptr );
    BOOST_CHECK_EQUAL( regptr->is_open(), false );
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_open_true_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\nopen true\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    geometry::cube_region const* regptr;
    regptr = dynamic_cast< geometry::cube_region const* >( &( mngr->get_region( 1ul ) ) );
    BOOST_REQUIRE( regptr != nullptr );
    BOOST_CHECK_EQUAL( regptr->is_open(), true );
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_open_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\nopen once\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cube wall permeability parameter \"open\" with value (once) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>open once<<\n** Expected a boolean value (ie. true|false). **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_unknown_param_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\nopen true\nunknown parameter\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Unknown parameter \"unknown\" in section \"region\" subtype \"cube\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n** Parameter \"unknown\" is not expected in this section and type. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cube_region_repeated_param_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname bulk\nopen true\nwidth 6.0\norigin 1.0 1.0 1.0\nwidth 8.0\ntype cube\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Parameter \"width\" appears more than once in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>width 8.0<<\n** Parameter \"width\" can only appear once per section. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_periodic_cube_region_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::periodic_cube_region::add_definition( *m );
  
      core::input_help hlpr;
      m->publish_help( hlpr );
      {
        std::stringstream ss;
        hlpr.write( ss );
        BOOST_CHECK( ss.str().find( "name" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "type" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "width" ) < ss.str().size() );
      }
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nwidth 8.0\ntype periodic-cube\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    BOOST_CHECK_EQUAL( mngr->region_count(), 2ul );
    BOOST_CHECK_EQUAL( mngr->region_key( "bulk" ), 1ul );
  
    geometry::periodic_cube_region const* regptr;
    regptr = dynamic_cast< geometry::periodic_cube_region const* >( &( mngr->get_region( 1ul ) ) );
    BOOST_REQUIRE( regptr != nullptr );
    BOOST_CHECK_EQUAL( regptr->length(), 8.0 );
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_periodic_cube_no_width_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::periodic_cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\n#width 8.0\ntype periodic-cube\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Periodic cube section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 5.\n** Add the following parameters to the section: width **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_periodic_cube_width_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::periodic_cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\nwidth #8.0\ntype periodic-cube\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Periodic cube size parameter \"width\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>width #8.0<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_periodic_cube_width_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::periodic_cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\nwidth 8.0 pm\ntype periodic-cube\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Periodic cube size parameter \"width\" with value (8.0 pm) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>width 8.0 pm<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_periodic_cube_width_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::periodic_cube_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
    // Type is hidden by comment character
    std::string canon_input( "region\nname \"bulk\"\nwidth -8.0\ntype periodic-cube\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Periodic cube size parameter \"width\" with value (-8.0) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>width -8.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_region_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
  
      core::input_help hlpr;
      m->publish_help( hlpr );
      {
        std::stringstream ss;
        hlpr.write( ss );
        BOOST_CHECK( ss.str().find( "name" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "type" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "radius" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "half-length" ) < ss.str().size() );
      }
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype cylinder\nradius 8\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    BOOST_CHECK_EQUAL( mngr->region_count(), 2ul );
    BOOST_CHECK_EQUAL( mngr->region_key( "bulk" ), 1ul );
  
    geometry::cylinder_region const* regptr;
    regptr = dynamic_cast< geometry::cylinder_region const* >( &( mngr->get_region( 1ul ) ) );
    BOOST_REQUIRE( regptr != nullptr );
    BOOST_CHECK_EQUAL( regptr->half_length(), 8.0 );
    BOOST_CHECK_EQUAL( regptr->radius(), 8.0 );
    BOOST_CHECK_EQUAL( regptr->volume( 0.0 ), core::constants::pi() * 1024 );
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_no_halflength_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\n#half-length 8.0\ntype cylinder\nradius 8\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Cylinder section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: half-length **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_halflength_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length #8.0\ntype cylinder\nradius 8\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Cylinder size parameter \"half-length\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>half-length #8.0<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_halflength_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length eight#8.0\ntype cylinder\nradius 8\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Cylinder size parameter \"half-length\" with value (eight) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>half-length eight#8.0<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_halflength_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length -8.0\ntype cylinder\nradius 8\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Cylinder size parameter \"half-length\" with value (-8.0) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>half-length -8.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_no_radius_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype cylinder\n#radius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Cylinder section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: radius **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_radius_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype cylinder\nradius #8\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Cylinder size parameter \"radius\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>radius #8<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_radius_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype cylinder\nradius eight#8\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Cylinder size parameter \"radius\" with value (eight) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>radius eight#8<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_cylinder_radius_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype cylinder\nradius -8\nend\n\n" );
  
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
      //std::cout << msg;
      BOOST_CHECK( msg.find( "Cylinder size parameter \"radius\" with value (-8) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>radius -8<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_region_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      core::input_help hlpr;
      m->publish_help( hlpr );
      {
        std::stringstream ss;
        hlpr.write( ss );
        BOOST_CHECK( ss.str().find( "name" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "type" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "radius" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "half-length" ) < ss.str().size() );
      }
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype open-cylinder\nradius 8\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    BOOST_CHECK_EQUAL( mngr->region_count(), 2ul );
    BOOST_CHECK_EQUAL( mngr->region_key( "bulk" ), 1ul );
  
    geometry::open_cylinder_region const* regptr;
    regptr = dynamic_cast< geometry::open_cylinder_region const* >( &( mngr->get_region( 1ul ) ) );
    BOOST_REQUIRE( regptr != nullptr );
    BOOST_CHECK_EQUAL( regptr->half_length(), 8.0 );
    BOOST_CHECK_EQUAL( regptr->radius(), 8.0 );
    BOOST_CHECK_EQUAL( regptr->volume( 0.0 ), core::constants::pi() * 1024 );
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_no_halflength_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\n#half-length 8.0\ntype open-cylinder\nradius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Open cylinder section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: half-length **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_halflength_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length #8.0\ntype open-cylinder\nradius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Open cylinder size parameter \"half-length\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>half-length #8.0<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_halflength_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length eight#8.0\ntype open-cylinder\nradius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Open cylinder size parameter \"half-length\" with value (eight) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>half-length eight#8.0<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_halflength_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length -8.0\ntype open-cylinder\nradius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Open cylinder size parameter \"half-length\" with value (-8.0) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>half-length -8.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_no_radius_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype open-cylinder\n#radius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Open cylinder section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: radius **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_radius_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype open-cylinder\nradius #8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Open cylinder size parameter \"radius\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>radius #8<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_radius_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype open-cylinder\nradius eight#8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Open cylinder size parameter \"radius\" with value (eight) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>radius eight#8<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_open_cylinder_radius_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype open-cylinder\nradius -8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Open cylinder size parameter \"radius\" with value (-8) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>radius -8<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_region_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      core::input_help hlpr;
      m->publish_help( hlpr );
      {
        std::stringstream ss;
        hlpr.write( ss );
        BOOST_CHECK( ss.str().find( "name" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "type" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "radius" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "offset" ) < ss.str().size() );
        BOOST_CHECK( ss.str().find( "half-length" ) < ss.str().size() );
      }
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype split-cylinder\nradius 8\noffset 4\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    BOOST_CHECK_EQUAL( mngr->region_count(), 2ul );
    BOOST_CHECK_EQUAL( mngr->region_key( "bulk" ), 1ul );
  
    geometry::open_split_cylinder_region const* regptr;
    regptr = dynamic_cast< geometry::open_split_cylinder_region const* >( &( mngr->get_region( 1ul ) ) );
    BOOST_REQUIRE( regptr != nullptr );
    BOOST_CHECK_EQUAL( regptr->half_length(), 8.0 );
    BOOST_CHECK_EQUAL( regptr->radius(), 8.0 );
    BOOST_CHECK_EQUAL( regptr->offset(), 4.0 );
    BOOST_CHECK_EQUAL( regptr->volume( 0.0 ), core::constants::pi() * 1024 );
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_no_halflength_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    std::string canon_input( "region\nname \"bulk\"\n#half-length 8.0\ntype split-cylinder\nradius 8\noffset 4\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 7.\n** Add the following parameters to the section: half-length **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_halflength_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length #8.0\ntype split-cylinder\nradius 8\noffset 4\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder size parameter \"half-length\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>half-length #8.0<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_halflength_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length eight#8.0\ntype split-cylinder\noffset 4\nradius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder size parameter \"half-length\" with value (eight) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>half-length eight#8.0<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_halflength_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    std::string canon_input( "region\nname \"bulk\"\noffset 4\nhalf-length -8.0\ntype split-cylinder\nradius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder size parameter \"half-length\" with value (-8.0) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>half-length -8.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_radius_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\noffset 4.0\nname \"bulk\"\nhalf-length 8.0\ntype split-cylinder\nradius #8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder size parameter \"radius\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>radius #8<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_radius_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\noffset 4\nname \"bulk\"\nhalf-length 8.0\ntype split-cylinder\nradius eight#8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder size parameter \"radius\" with value (eight) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>radius eight#8<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_no_radius_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype split-cylinder\n#radius 8\noffset 4\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 7.\n** Add the following parameters to the section: radius **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_radius_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\noffset 4.0\nname \"bulk\"\nhalf-length 8.0\ntype split-cylinder\nradius -8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder size parameter \"radius\" with value (-8) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>radius -8<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_no_offset_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype split-cylinder\nradius 8\n#offset 4\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder section \"region\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 7.\n** Add the following parameters to the section: offset **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_offset_no_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype split-cylinder\nradius 8\noffset #4\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder split parameter \"offset\" in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>offset #4<<\n** Expected a value. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_offset_bad_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    std::string canon_input( "region\nname \"bulk\"\nhalf-length 8.0\ntype split-cylinder\noffset eight#4\nradius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder split parameter \"offset\" with value (eight) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>offset eight#4<<\n** A numeric value was expected. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

// Use periodic cube region input data to test region_meta functions.
BOOST_AUTO_TEST_CASE( input_split_cylinder_offset_negative_value_test )
{
  // Test read input (specie meta)
  {
    boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< geometry::geometry_manager > mngr( new geometry::geometry_manager( sys_rgn ) );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< geometry::region_meta > m( new geometry::region_meta( mngr ) );
      geometry::open_split_cylinder_region::add_definition( *m );
  
      dg.add_input_delegate( m );
    }
  
    std::string canon_input( "region\nname \"bulk\"\noffset -4.2\nhalf-length 8.0\ntype split-cylinder\nradius 8\nend\n\n" );
  
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
      BOOST_CHECK( msg.find( "Split cylinder split parameter \"offset\" with value (-4.2) in section \"region\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>offset -4.2<<\n** Input value must be greater than zero. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string("exception thrown by \"dg.read_input( reader )\" was not expected type: ") + err.what() );
    }
  }

}

