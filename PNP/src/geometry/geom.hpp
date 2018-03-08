#ifndef IONCH_GEOMETRY_GEOM_HPP
#define IONCH_GEOMETRY_GEOM_HPP


#include <cstddef>

namespace geometry {

class Geom
 {
   public:
    static Geom geom;

    const double rbulk;

    const double zbulk1;

    const double zbulk2;

    const double vbulk;

    double volblk() const;

    double lenblk() const;

    std::size_t gz_bin(double zval) const;

    void inregn(double zval, std::size_t spec, std::size_t & ireg) const;


};

} // namespace geometry
#endif
