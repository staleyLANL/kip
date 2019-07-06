
#pragma once

// This file provides the "vars" class template, which kip uses internally.

// -----------------------------------------------------------------------------
// vars
// -----------------------------------------------------------------------------

namespace kip {
namespace internal {

template<class real, class tag>  // template arguments defaulted elsewhere
class vars {
public:
   // Shapes: abstract access for uniform and block methods
   array<2,std::vector<minimum_and_shape<real,tag>>> uniform;
   array<3,vec_reset<real,tag>> block;

   // Miscellaneous
   rotate<3,real> t2e;
   point<real> eyeball;

   real hmax, hhalf,hfull, heps, hrat,hratsub;
   real vmax, vhalf,vfull, veps, vrat,vratsub;

   rotate<-3,real> left, right;
   rotate<-3,real> bottom, top, behind;

   array<1,rotate<-3,real>> seg_hneg, seg_hpos;
   array<1,rotate<-3,real>> seg_vneg, seg_vpos;
   array<1,rotate<-3,real>> seg_diag, seg_quad, seg_3060, seg_1575;

   unsigned anti2;

   // qqq figure out if we really need the crap below
   // anti-dependent constants
   // in long double, double, and single: 1/anti, and 1/(anti*anti)
   mutable long double q_rec_anti, q_rec_anti2;
   mutable      double d_rec_anti, d_rec_anti2;
   mutable       float f_rec_anti, f_rec_anti2;

   long double &rec_anti (const long double) const { return q_rec_anti ; }
        double &rec_anti (const      double) const { return d_rec_anti ; }
         float &rec_anti (const       float) const { return f_rec_anti ; }

   long double &rec_anti2(const long double) const { return q_rec_anti2; }
        double &rec_anti2(const      double) const { return d_rec_anti2; }
         float &rec_anti2(const       float) const { return f_rec_anti2; }
};

}
}
