
#pragma once

// This file provides the "view" class.



// -----------------------------------------------------------------------------
// view
// -----------------------------------------------------------------------------

template<class real = default_real_t>
class view {
public:
   static constexpr real default_d = real(8);
   static constexpr real default_fov = real(16);

   // point at which eyeball is looking
   point<real> target = point<real>(0,0,0);

   mutable real d   = default_d;   // eyeball's distance from target
   mutable real fov = default_fov; // field of view

   real theta = real(60); // angle in xy plane, from +x axis
   real phi   = real(30); // angle up(+) or down(-) from xy plane
   real roll  = real( 0); // right(+) or left(-) roll of eyeball

   // prior zzz Eventually make private, so users can't disturb
   class _prior {
   public:
      /// mutable point<real> target;
      /// mutable real theta, phi, roll;
      mutable real d, fov;
      mutable bool first = true;
   } prior;

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
      oss << (d = default_d);
      (void)warning(oss);
   }

   // fov
   if (fov <= 0 || fov >= 180) {
      std::ostringstream oss;
      oss << "View has fov=" << fov << "; setting to default=";
      oss << (fov = default_fov) << " degrees";
      (void)warning(oss);
   }

   return *this;
}
