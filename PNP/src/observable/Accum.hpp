#ifndef IONCH_OBSERVABLE_ACCUM_HPP
#define IONCH_OBSERVABLE_ACCUM_HPP


#include <cstddef>

namespace observable {

class Accum
 {
   private:
    std::size_t rdf_index(std::size_t ispec, std::size_t jspec, std::size_t ireg) const;


   public:
    // exemplar
    static Accum accum;

    // maximum number of histogram bins in r direction
    static const std::size_t nrgmx=  2048;

    // maximum number of particles in filter region we count for
    // generating the co-occupancy matrix 'aocc'
    static const std::size_t noccmx=  4;


   private:
    // Sum of h vector from patch class (average: ah/ataccu) {
    ublas::vector< double > ah;

    // co-occupancy matrix - record what sets of particles
    // are in the filter region at the same time
    std::size_t aocc[3][noccmx+1][noccmx+1][noccmx+1][noccmx+1];

    // Histogram of particles in each region
    //
    // (merged 'an' into anin as anin(ibulk,*)
    hist2array anin;

    // Sum of per-specie particle number in bulk region (average: abulk/athist)
    ublas::vector< double > abulk;

    // Current per-specie concentration in bulk
    ublas::vector< double > cbulk;

    // Sum of charges (average: acharge/ataccu)
    runstat acharge;

    // Count of calls to 'hist' routine; used to determine average
    // of statistics collected in 'hist' routine
    std::size_t athist;

    // Count of calls to 'accmlt' routine; used to determine
    // average of statistics collected in 'accmlt' routine
    std::size_t ataccu;

    // Width of bins (both axial and radial) in 2D 'gin' distribution
    // and 3D 'gxyz' and 'grtz' distributions.
    double drg;

    // The axial outer-most point of the 2D 'gin' distribution
    double zinlft;

    // The radial outer-most point of the 2D 'gin distribution
    double rinup;

    // Width of histogram bins in the inter-particle radial distribution
    double drdf;

    // Width to #include "for the inner-most occupancy calculations (defaults to zlimit).hpp"
    //=0.D0
    double zocc;

    // volumes for the 2D 'gin' histogram bins (indep} //ent of z and specie)
    ublas::vector< double > ginvol;

    // The 2D 'gin' distribution
    hist3array gin;

    // Index of the particle to #include "as the theta zero.hpp"
    std::size_t grtz_zero;

    // Width of bins (both axial and radial) in 3D 'gxyz' and 'grtz' distributions.
    double d3_width;

    // volumes for the 3D 'grtz' histogram bins (indep} //ent of z and specie)
    ublas::vector< double > d3_vol;

    // The counts (numerator) for the inter-particle radial distribution
    // INDEX (ireg, jspec, ispec)
    // type (hist1array), private, allocatable, dimension (:,:,:) :: 
    hist1array * rdfhist;

    // The z-axial 'gz' distribution data
    hist2array gzhist;

    // The number of in-#include "bins in the 'gz' distribution.hpp"
    std::size_t nrg;

    // The number of radial bins in 2D 'gin' distribution
    std::size_t nrgr;

    // The number of axial bins in 2D 'gin' distribution
    std::size_t nrgz;

    // The number of active bins in the inter-particle radial distribution
    std::size_t nrdf;

    // The number of times we have saved a digest (ie called 'saves')
    // =0;
    std::size_t ksub;

    // [INPUT] How often the statistic data is saved.
    std::size_t isave;

    // [INPUT] calgin: compute 2D (z,r) profiles
    // =.false.
    bool calgin;

    // [INPUT] calrdf: compute inter-particle radial distribution
    // =.false.
    bool calrdf;

    // [INPUT] calacc: show move acceptance ratios
    // =.false.
    bool calacc;

    // [INPUT] calmob: calculate information about 'mobile' ions
    // =.false.
    bool calmob;

    // fluctuation variable sum(N_i * N_j) used in iterat
    // [Constants::nspcmx][Constants::nspcmx];
    ublas::matrix< double > ninj;


   public:
    enum chem_pot_method
     {
      no_update = 0
      ,
      malas1 = 1
      ,
      malas2 = 2
      ,
      accept = 3
    

    };


   private:
    inline Accum() : amobdl(Constants::nspcmx)
        , amobvr(Constants::nspcmx)
        , amobdx(Constants::nspcmx)
        , amobdy(Constants::nspcmx)
        , amobdz(Constants::nspcmx)
        , ah()
        , aocc()
        , anin()
        , abulk(Constants::nspcmx)
        , cbulk(Constants::nspcmx)
        , acharge()
        , athist(0)
        , ataccu(0)
        , drg(0.1), zinlft(0.0), rinup(0.0), drdf(0.0), zocc(0.0)
        , ginvol()
        , gin()
        , gxyz(), grtz()
        , grtz_zero(0)
        , d3_width(0.1), d3_rmax(0.0), d3_zmax(0.0)
        , d3_zgrid_size(0), d3_rgrid_size(0)
        , d3_vol()
        , rdfhist(NULL)
        , gzhist()
        , nrg(0), nrgr(0), nrgz(0), nrdf(0)
        , ksub(0), isave(0)
        , calgin(false)
        , calrdf(false)
        , calacc(false)
        , calmob(false)
        , ninj(Constants::nspcmx,Constants::nspcmx)
      {
        for (std::size_t idx = 0; idx //= Constants::nspcmx; ++idx)
          {
    	for (std::size_t jdx = 0; jdx //= Constants::nspcmx; ++jdx)
    	  {
    	    ninj(idx, jdx) = 0;
    	  }
          }
      };

    // Undefined
    Accum(const Accum & );

    // Undefined
    const Accum & operator =(const Accum & );


   public:
    inline ~Accum() {
        if (rdfhist //= NULL) delete [] rdfhist;
      };

    //public :: accmlt, acceql, accblk, iterat, ecaccu, gz_av, hist, rdaccu, rfaccu, saves, zeroac
    // -------------------------------------------------------------
    // Accumulate simple collective statistics
    //
    //  This routine collects filter occupancy data from 'nin' arrays.
    //  It also saves the 'h' vector and charge.
    inline void accmlt() {
        // LOCALS
        std::size_t ireg,ispec  // loop counters for regions,species,patches
          ,i1,i2,i3,i4; // histogram indices
    
        ++this->ataccu;
        this->acharge.rs_push(Conf::conf.charge);
    
        // for (h3c4v2:  ; XXX != ; ++XXX) {
        for (ireg = Constants::izlim; ireg //= Constants::ichan + 1; ++ireg)
          {
    	i1 = std::min(Conf::conf.nin(ireg,Spec::spec.idxcl()),noccmx);
    	i2 = std::min(Conf::conf.nin(ireg,Spec::spec.idxcl()+1),noccmx);
    	i3 = std::min(Conf::conf.nin(ireg,Spec::spec.idxcl()+2),noccmx);
    	i4 = std::min(Conf::conf.nin(ireg,Spec::spec.idxcl()+3),noccmx);
    	++this->aocc[ireg][i1][i2][i3][i4];
          } //for (h3c4v2 ; XXX != ; ++XXX) {
    
        // for (l0z8a5: do  ; XXX != ; ++XXX) {
        for (ispec = 0; ispec //= Spec::spec.nspec(); ++ispec);
        {
          this->anin.hist_push (Constants::izlim, ispec, Conf::conf.nin(Constants::izlim,ispec));
          this->anin.hist_push (Constants::ifilt, ispec, Conf::conf.nin(Constants::ifilt,ispec));
          this->anin.hist_push (Constants::ichan, ispec, Conf::conf.nin(Constants::ichan,ispec));
          this->anin.hist_push (Constants::ibulk, ispec, Conf::conf.ni(ispec));
        } //for (l0z8a5 ; XXX != ; ++XXX) {
    
        if (not Patch::patch.homog)
          {
    	for (std::size_t ipch = 0; ipch //= Patch::patch.npatch(); ++ipch)
    	  {
    	    this->ah[ipch] += Conf::conf.h(ipch);
    	  }
          }
        this->anin.hist_};


};

} // namespace observable
#endif
