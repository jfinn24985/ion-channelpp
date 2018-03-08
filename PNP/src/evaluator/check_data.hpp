#ifndef IONCH_EVALUATOR_CHECK_DATA_HPP
#define IONCH_EVALUATOR_CHECK_DATA_HPP

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
#include <iostream>

namespace evaluator { class induced_charge; } 
namespace ion_channel { class channel_system; } 

namespace evaluator {

//  Set of parameters used to verify if saved icc_surface_grid
//  and induced_charge_matrix data can be reused.
struct check_data 
{
    // Convert double into size_t
    constexpr static std::size_t hashdbl(double v)
    {
      return std::size_t( v * 1024 * 1024 );
    }

    // nsub0 check value
    std::size_t nsub;

    // nsub0 check value
    std::size_t dxf;

    // nsub0 check value
    std::size_t dxw;

    // nsub0 check value
    std::size_t pore_hlength;

    // nsub0 check value
    std::size_t cell_hlength;

    // nsub0 check value
    std::size_t pore_radius;

    // nsub0 check value
    std::size_t protein_radius;

    // nsub0 check value
    std::size_t cell_radius;

    // nsub0 check value
    std::size_t vestibule_arc;

    // nsub0 check value
    std::size_t membrane_arc;

    // nsub0 check value
    std::size_t permittivity;

    // nsub0 check value
    std::size_t protein_permittivity;

    void set_dxw(double val)
    {
      this->dxw = this->hashdbl( val );
    }

    void set_pore_hlength(double val)
    {
      this->pore_hlength = this->hashdbl( val );
    }

    void set_cell_hlength(double val)
    {
      this->cell_hlength = this->hashdbl( val );
    }

    void set_cell_radius(double val)
    {
      this->cell_radius = this->hashdbl( val );
    }

    void set_dxf(double val)
    {
      this->dxf = this->hashdbl( val );
    }

    void set_dxw(double val)
    {
      this->dxw = this->hashdbl( val );
    }

    void set_membrane_arc(double val)
    {
      this->membrane_arc = this->hashdbl( val );
    }

    void set_nsub(double val)
    {
      this->nsub = this->hashdbl( val );
    }

    void set_permittivity(double val)
    {
      this->permittivity = this->hashdbl( val );
    }

    void set_protein_permittivity(double val)
    {
      this->protein_permittivity = this->hashdbl( val );
    }

    void set_protein_radius(double val)
    {
      this->protein_radius = this->hashdbl( val );
    }

    void set_pore_hlength(double val)
    {
      this->pore_hlength = this->hashdbl( val );
    }

    void set_pore_radius(double val)
    {
      this->pore_radius = this->hashdbl( val );
    }

    void set_vestibule_arc(double val)
    {
      this->vestibule_arc = this->hashdbl( val );
    }

    // Set object's data from source data.
    void set_data(const induced_charge & icc, const ion_channel::channel_system & sim);

    // Compare source data with this object
    bool compare(const induced_charge & icc, const ion_channel::channel_system & sim) const;

    // Compare two check data objects
    bool equivalent(const check_data & other) const;

    // Write data to stream.
    void write_data(std::ostream & os) const;

    // Read data from a stream. Return false if problems reading the stream.
    bool read_data(std::istream & input);

    check_data()
    : nsub(), dxf(), dxw(), pore_hlength(), cell_hlength(), pore_radius()
    , protein_radius(), cell_radius(), vestibule_arc(), membrane_arc()
    , permittivity(), protein_permittivity()
    {}

    check_data(const check_data & source) = default;

    check_data(check_data && source) = default;

    // Compare two check data objects
    bool swap(const check_data & other) const;

    ~check_data() {}

    check_data & operator=(check_data source)
    {
      this->swap( source );
      return *this;
    }


};

} // namespace evaluator
#endif
