#ifndef IONCH_EVALUATOR_EVALUATOR_FIXTURE_HPP
#define IONCH_EVALUATOR_EVALUATOR_FIXTURE_HPP

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

#include <cstddef>

namespace evaluator { class icc_surface_grid; } 
namespace geometry { class coordinate; } 

namespace evaluator {

//  Model of induced_charge to tes
//
// read/write inverted A matrix and patches.
// generate patches from geometry
class patch_matrix_fixture
 {
   private:
    // Scale factor = kT . echg / (4 * pi * eps0 * angstrom  * epsw )
    double alfa_;

    double dxf_;

    double dxw_;

    // permittivity of water/media
    double epsw_;

    // permittivity of protein
    double epspr_;

    // The expected number of tiles in grid from the parameters
    // defined in this class.
    std::size_t expected_size_;

    std::size_t nsub_;

    // Pore half length and centre for vestibule arc
    double zl1_;

    // Channel half length (zl1 + rlvest)
    double zl2_;

    // Centr for membrane arc (zl1 + rlvest - rlmemb)
    double zl3_;

    // Pore radius
    double rl1_;

    // Radial from rotation axis of centre of vestibule arc (rl1 + rlvest)
    double rl2_;

    //  Radial from rotation axis of centre of membrane arc (rl4 - rlmemb)
    double rl3_;

    // Protein outer radius
    double rl4_;

    // Vestibule arc radius
    double rlvest_;

    // Membrane arc radius
    double rlmemb_;

    // Membrane arc radius
    double temperature_;


   public:
    //As change particle : change particle Coulomb interactions are calculated
    //in the same way as change particle : config particle, the __init__ method
    //makes _do_change_contribution an alias for _do_contribution. The
    //nspec argument is the number of interacting species.
    patch_matrix_fixture();


   private:
    patch_matrix_fixture(const patch_matrix_fixture & source) = delete;

    patch_matrix_fixture(patch_matrix_fixture && source) = delete;


   public:
    ~patch_matrix_fixture()
    {}


   private:
    patch_matrix_fixture & operator=(const patch_matrix_fixture & source) = delete;


   public:
    double alfa() const
    {
       return this->alfa_;
    }
    double dxf() const
    {
       return this->dxf_;
    }
    double dxw() const
    {
       return this->dxw_;
    }
    double epsw() const
    {
       return this->epsw_;
    }
    double epspr() const
    {
       return this->epspr_;
    }
    std::size_t expected_grid_size() const
    {
       return this->expected_size_;
    }
    std::size_t nsub() const
    {
       return this->nsub_;
    }
    double rl1() const
    {
       return this->rl1_;
    }
    double rl2() const
    {
       return this->rl2_;
    }
    double rl3() const
    {
       return this->rl3_;
    }
    double rl4() const
    {
       return this->rl4_;
    }
    double rlvest() const
    {
       return this->rlvest_;
    }
    double rlmemb() const
    {
       return this->rlmemb_;
    }
    double temperature() const
    {
       return this->temperature_;
    }
    double zl1() const
    {
       return this->zl1_;
    }
    double zl2() const
    {
       return this->zl2_;
    }
    double zl3() const
    {
       return this->zl3_;
    }
    // Defining the patch grid.
    //
    // \pre grid.empty
    
    void define_grid(icc_surface_grid & grid);

    // The given XYZ position lies on the surface.
    bool on_surface(double x, double y, double z) const;

    double surface_area() const;

    // Test that the vector u at p is normal to the surface.
    bool normal_test(const geometry::coordinate & p, const geometry::coordinate & u);


};

} // namespace evaluator
#endif
