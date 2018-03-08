

#ifndef DEBUG
#define DEBUG 0
#endif

#include "observable/Accum.hpp"

namespace observable {

std::size_t Accum::rdf_index(std::size_t ispec, std::size_t jspec, std::size_t ireg) const 
{

}

// exemplar
Accum Accum::accum;

// maximum number of histogram bins in r direction
const std::size_t Accum::nrgmx;

// maximum number of particles in filter region we count for
// generating the co-occupancy matrix 'aocc'
const std::size_t Accum::noccmx;

// Undefined
Accum::Accum(const Accum & )
{

}

// Undefined
const Accum & Accum::operator =(const Accum & ) 
{

}


} // namespace observable
