
#pragma once

// This file provides the "view" class.

namespace kip {



// -----------------------------------------------------------------------------
// Defaults
// -----------------------------------------------------------------------------

namespace internal {

// d
template<class real>
inline real default_d() { return real(8); }

// target
template<class real>
inline point<real> default_target()
{
   return point<real>(real(0), real(0), real(0));
}

// theta, phi, roll
template<class real> inline real default_theta() { return real(60); }
template<class real> inline real default_phi  () { return real(30); }
template<class real> inline real default_roll () { return real( 0); }

// fov
template<class real>
inline real default_fov() { return real(16); }

}



// -----------------------------------------------------------------------------
// view
// -----------------------------------------------------------------------------

template<class real = default_real_t>
class view {
public:
   point<real> target;  // point at which eyeball is looking

   mutable real d;    // eyeball's distance from target
   mutable real fov;  // field of view

   real theta;   // angle in xy plane, from +x axis
   real phi;     // angle up(+) or down(-) from xy plane
   real roll;    // right(+) or left(-) roll of eyeball

   // prior   zzz Eventually make private, so users can't disturb
   class _prior {
   public:
      /// mutable point<real> target;
      /// mutable real theta, phi, roll;
      mutable real d, fov;

      // first
      mutable bool first;

      // c'tor
      inline explicit _prior() : first(true) { }
   } prior;



   // view()
   inline explicit view() :
   target(internal::default_target<real>()),
   d     (internal::default_d     <real>()),
   fov   (internal::default_fov   <real>()),
   theta (internal::default_theta <real>()),
   phi   (internal::default_phi   <real>()),
   roll  (internal::default_roll  <real>())
   { }

   // fix
   inline const view &fix() const;
};



// -----------------------------------------------------------------------------
// fix
// -----------------------------------------------------------------------------

template<class real>
const view<real> &view<real>::fix() const
{
   // d
   if (d <= 0) {
      std::ostringstream oss;
      oss << "View has d=" << d << "; setting to default=";
      oss << (d = internal::default_d<real>());
      (void)warning(oss);
   }

   // fov
   if (fov <= 0 || fov >= 180) {
      std::ostringstream oss;
      oss << "View has fov=" << fov << "; setting to default=";
      oss << (fov = internal::default_fov<real>()) << " degrees";
      (void)warning(oss);
   }

   // done
   return *this;
}

}
