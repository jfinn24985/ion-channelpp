#ifndef IONCH__LINE_INTEGRATOR_HPP
#define IONCH__LINE_INTEGRATOR_HPP


#include "CC/integrator.hpp"

//(from bodacea)
struct line_integrator : public integrator 
{
    // start z-coord
    double z0;

    // end z-coord
    double z1;

    // radial
    double r0;

    inline line_integrator(int j, double a0, double a1, double b0, double c0, double c1) : integrator (j, c0, c1), z0 (a0), z1 (a1), r0 (b0)
        {
            GRS400_REQUIRE (! feq (r0, 0.), "The radial coordinate must not equal zero");
            GRS400_REQUIRE (! feq (z0, z1), "The two z-coordinates must not be equal");
        };

    inline double operator ()(uint16_type ii, const patch_type & ptch) const {
            double result = 0.;
            const int nsub_ (jj == ii ? NSubII : NSubIJ);
            // z2 and z1 are end and begin of this particular ring so split up
            // each ring into pieces and also around the phi angle
            const double dz = (z1 - z0) / double (nsub_);
            const double dphi = (phi1 - phi0) / double (nsub_);
    
            // JJF: talfa is the tangent of the slope of the line so === 0 as
            // here lines are parallel to z-axis
    
            std::valarray< double > cosphij (nsub_);
            std::valarray< double > sinphij (nsub_);
    
            if (nsub_ == NSubIJ)
            {
                if (cosphij_.size () != NSubIJ)
                {
                    const_cast< line_integrator & > (*this).build_table ();
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
    
            // double loop over subtiles, over z and over theta angle so each
            // patch is going to have nsub*nsub tiny patches
            for (int i_ = 0; i_ < nsub_; ++i_)
            {
                const double zz0 = z0 + i_ * dz;
                const double zz1 = zz0 + dz;
                // JEF: get the centroid in this weird way i can't figure out
                // r0 is radius if line at beginning (left) side
                // z0 beginning (left) of line
                // talfa is tangent of the slope of the line
                // dzsub is spacing between each tiny patch
                // zz2 is right side of tinypatch
                // zz1 is left side of tinypatch
                const double ar = r0 * dz;
                const double szaml = 0.5 * r0 * (sqr (zz1) - sqr (zz0));
    
                // loop over phi angle
                patch_integral (r0 * dz * dphi, r0, szaml / ar, cosphij, sinphij, nsub_, result, ptch);
            }
    
            return result;
        };


};
#endif
