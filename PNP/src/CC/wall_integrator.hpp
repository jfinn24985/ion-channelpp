#ifndef IONCH__WALL_INTEGRATOR_HPP
#define IONCH__WALL_INTEGRATOR_HPP


#include "CC/integrator.hpp"

//(from bodacea)
struct wall_integrator : public integrator 
{
    // z-coord
    double z;

    // inner r
    double r0;

    // outer r
    double r1;

    inline wall_integrator(int j, double a, double b0, double b1, double c0, double c1) : integrator (j, c0, c1), z (a), r0 (b0), r1 (b1)
        {
            GRS400_REQUIRE (! feq (r1, r0), "radial coordinates r0 and r1 must not be equal");
        };

    inline double operator ()(uint16_type ii, const patch_type & ptch) const {
            double result = 0.;
            const int nsub_ (jj == ii ? NSubII : NSubIJ);
            const double dr = (r1 - r0) / double (nsub_);
            const double dphi = (phi1 - phi0) / double (nsub_);
    
            std::valarray< double > cosphij (nsub_);
            std::valarray< double > sinphij (nsub_);
    
            if (nsub_ == NSubIJ)
            {
                if (this->cosphij_.size () == 0)
                {
                    const_cast< wall_integrator & > (*this).build_table ();
                }
    
                cosphij = this->cosphij_;
                sinphij = this->sinphij_;
            }
            else
            {
                for (int j_ = 0; j_ < nsub_; ++j_)
                {
                    const double phij = phi0 + (double (j_) + .5) * dphi;
                    cosphij[j_] = std::cos (phij);
                    sinphij[j_] = std::sin (phij);
                }
            }
    
            // double loop over subtiles, over r and over theta angle so each
            // patch is going to have nsub*nsub tiny patches
            for (int i_ = 0; i_ < nsub_; ++i_)
            {
                const double rr0 = r0 + i_ * dr;
                const double rr1 = rr0 + dr;
    
                const double ar = 0.5 * (sqr (rr1) - sqr (rr0));
                const double szaml = (sqr (rr1) * rr1 - sqr (rr0) * rr0) / 3.;
                const double rc = szaml / ar;
                // loop over phi angle
                patch_integral (rc * dr * dphi, rc, z, cosphij, sinphij, nsub_, result, ptch);
           }
    
            return result;
        };


};
#endif
