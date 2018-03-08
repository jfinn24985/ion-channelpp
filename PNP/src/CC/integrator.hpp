#ifndef IONCH__INTEGRATOR_HPP
#define IONCH__INTEGRATOR_HPP


#include <boost/serialization/valarray.hpp>

//(from bodacea)
struct integrator 
{
    // revolution start angle
    double phi0;

    // revolution end angle
    double phi1;

    // Cache tables of cosine and sine values
    inline integrator(int j, double a0, double a1) : phi0 (a0), phi1 (a1), jj (j), cosphij_(), sinphij_()
        {
            GRS400_REQUIRE (! feq (phi0, phi1), "rotation angles phi0 and phi1 must not be equal");
        };

    virtual double operator ()(uint16_type ii, const patch_type & ptch) const = 0;

    inline void build_table() {
            this->cosphij_.resize (NSubIJ);
            this->sinphij_.resize (NSubIJ);
            const double dphi = (this->phi1 - this->phi0) / double (NSubIJ);
    
            for (int j_ = 0; j_ < NSubIJ; ++j_)
            {
                const double phij = this->phi0 + (double (j_) + .5) * dphi;
                this->cosphij_[j_] = std::cos (phij);
                this->sinphij_[j_] = std::sin (phij);
            }
        };

    inline void clear_table() {
            this->cosphij_.resize (0, 0.);
            this->sinphij_.resize (0, 0.);
        };

    inline void patch_integral(double arel, double r, double z, std::valarray< double > sinphij, std::valarray< double > cosphij, int nsub, double & result, const patch_type & ptch) const {
                for (int j_ = 0; j_ < nsub; ++j_)
                {
                    const double pxj = r * cosphij[j_];
                    const double pyj = r * sinphij[j_];
                    const double pxij = ptch.px - pxj;
                    const double pyij = ptch.py - pyj;
                    const double pzij = ptch.pz - z;
                    const double rijsq = sqr (pxij) + sqr (pyij) + sqr (pzij);
                    const double rij3 = rijsq * std::sqrt (rijsq);
                    const double rk = (pxij * ptch.ux + pyij * ptch.uy + pzij * ptch.uz) / rij3;
                    result = ::fma(rk, arel, result);
                }
        };


};
#endif
