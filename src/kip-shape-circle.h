
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// circle
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class circle : public shape<real,tag> {
   using shape<real,tag>::interior;

   // modified circle: (0,0,0), r, in yz plane
   mutable rotate<3,real> rot;
   mutable real rsq, tmp;

public:
   using shape<real,tag>::basic;
   kip_functions(circle);

   // center, normal, radius
   point<real> c;
   point<real> n;
   real r;

   inline point<real> back(const point<real> &from) const { return rot.back(from); }

#define   kip_aye c
#define   kip_bee n
#define   kip_class circle
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(circle)
{
   rot = rotate<3,real>(c, c+n, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   rsq = r*r;
   tmp = rsq/(rot.h*rot.h);

   interior = false;

   return r < rot.ey
      ? op::sqrt(op::sq(rot.ex) + op::sq(rot.ey - r))
      : op::abs(rot.ex);
} kip_end



// aabb
kip_aabb(circle)
{
   point<real> min, max;
   internal::bound_abr(c,c+n,r, min,max);

   return kip::bbox<real>(
      true,min.x, max.x,true,
      true,min.y, max.y,true,
      true,min.z, max.z,true
   );
} kip_end



// dry
kip_dry(circle)
{
   real az;  if (seg.lt(c,az)) return false;
   real bz;  (void)seg.lt(n,bz);
   return op::sq(az-seg.c) >= tmp*(rot.h+bz)*(rot.h-bz);
} kip_end



// infirst
kip_infirst(circle)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (dx == 0 || !(0 < (q = -rot.ex/dx) && q < qmin)) return false;

   q.y = rot.ey + q*dy;
   q.z = q*tar.z;

   return op::sq(q.y) + op::sq(q.z) <= rsq
      ? q.x = real(0),
        q(rot.ex < 0 ? real(-1) : real(1), 0, 0, this, yesnorm), true
      : false;
} kip_end



// inall
kip_inall(circle)
{
   return circle<real,tag>::infirst(kip_etd,qmin,ints.one(),insub)
      ? ints.push(ints[0]), true
      : false;
} kip_end



// check
kip_check(circle)
{
   diagnostic_t rv = diagnostic_good;

   // require n != 0
   if (n == point<real>(0,0,0))
      rv = error("Circle has zero normal");

   // r
   if (r <= real(0)) {
      std::ostringstream oss;
      oss << "Circle has non-positive radius r=" << r;
      rv = error(oss);
   }

   return rv;
} kip_end



// random
kip_random(circle)
{
   // center
   obj.c(
      op::twice(real(random<real>() - 0.5)),
      op::twice(real(random<real>() - 0.5)),
      op::twice(real(random<real>() - 0.5))
   );

   // normal
   obj.n(real(random<real>()-0.5), real(random<real>()-0.5), real(random<real>()-0.5));

   // r
   obj.r = real(0.15*random<real>());

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(circle) {

   // (
   //    c.x, c.y, c.z,
   //    n.x, n.y, n.z,
   //    r,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.c, "center") && read_comma(s) &&
      read_value(s, obj.n, "normal") && read_comma(s) &&
      read_value(s, obj.r, "radius") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(circle) {
   return kip::internal::onetwor_write(k,obj, obj.c,obj.n,obj.r, "circle");
}

}

#define   kip_class circle
#include "kip-macro-io.h"