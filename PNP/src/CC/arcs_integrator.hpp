#ifndef IONCH__ARCS_INTEGRATOR_HPP
#define IONCH__ARCS_INTEGRATOR_HPP


#include "CC/integrator.hpp"

//(from bodacea)
struct arcs_integrator : public integrator 
{
    // centre z-coord
    double z0;

    // centre r-coord
    double r0;

    // arc radius
    double radius;

    // arc start angle
    double theta0;

    // arc end angle
    double theta1;

    inline arcs_integrator(int j, double a0, double a1, double b, double c0, double c1, double d0, double d1) : integrator (j, d0, d1),  z0 (a0), r0 (a1), radius (b), theta0 (c0), theta1 (c1)
        {
            if (DEBUG) { std::clog << "Arc = (" << a0 << ", " << a1 << ": " << b << "), Theta (" << c0 << ", " << c1 << ") at " << __LINE__ << "\n"; }
    
            GRS400_REQUIRE (! feq (radius, 0.), "The arc radius must not equal zero");
            GRS400_REQUIRE (! feq (theta0, theta1), "The arc start and end angles must not be equal");
        };

    // [02-intarc]
    inline double operator ()(uint16_type ii, const patch_type & ptch) const {
            double result = 0.;
            const int nsub_ (jj == ii ? NSubII : NSubIJ);
            const double dtheta = (theta1 - theta0) / double (nsub_);
            const double dphi = (phi1 - phi0) / double (nsub_);
            std::valarray< double > cosphij (nsub_);
            std::valarray< double > sinphij (nsub_);
    
            if (nsub_ == NSubIJ)
            {
                if (cosphij_.size () != NSubIJ)
                {
                    const_cast< arcs_integrator & > (*this).build_table ();
                }
    
                cosphij = cosphij_;
                sinphij = sinphij_;
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
    
            for (int i_ = 0; i_ < nsub_; ++i_)
            {
                const double tt0 = theta0 + i_ * dtheta;
                const double tt1 = tt0 + dtheta;
                const double costt0 = std::cos (tt0);
                const double costt1 = std::cos (tt1);
                const double ar = r0 * dtheta - radius * (costt0 - costt1);
                const double szaml = 0.5 * r0 * (sqr (tt0) + sqr (tt1)) - radius * (tt1 * costt1 - tt0 * costt0 + std::sin (tt1) - std::sin (tt0));
                const double ttc = szaml / ar;
                patch_integral (radius * ar * dphi, r0 + radius * std::sin (ttc),  z0 + radius * std::cos (ttc), cosphij, sinphij, nsub_, result, ptch);
            }
    
            return result;
        };


};
#endif
