
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// paraboloid
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class paraboloid : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // modified paraboloid: (0,0,0), (h,0,0), r
   mutable rotate<3,real> rot;
   mutable real rsq, h1, h2, h3;

   // get_*
   inline bool
   get_baseh(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_curve(const point<real> &, const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(paraboloid);

   // point a, point b, radius
   point<real> a, b;
   real r;

   inline point<real> back(const point<real> &from) const { return rot.back(from); }

#define   kip_class paraboloid
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(paraboloid)
{
   rot = rotate<3,real>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   rsq = r*r;
   h1 = r/rot.h;
   h2 = 2*rot.h/rsq;
   h3 = h2*rot.ey*rot.ey - 2*rot.ex;

   // interior
   interior = inside(eyeball);

   // minimum
   if (rot.ex >= rot.h && rot.ey <= r)
      return rot.ex - rot.h;  // east

   else if (r*(r - rot.ey) <= 2*rot.h*(rot.ex - rot.h)) {
      return op::sqrt(op::sq(rot.ex-rot.h) + op::sq(rot.ey-r));  // northeast

   } else {
      // safe - based on bounding cylinder
      return
         rot.ex <= 0 && rot.ey >= r
      ?  op::sqrt(op::sq(rot.ex) + op::sq(rot.ey-r))
      :  rot.ex <= 0
      ? -rot.ex
      :  rot.ey >= r
      ?  rot.ey-r
      :  0;  // inside bounding cylinder (and possibly inside paraboloid)
   }
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// bound_paraboloid - helper
namespace internal {
   template<class real>
   inline void bound_paraboloid(
      const real &a, const real &b, const real &c, const real &h, const real &r,
      const real &one, const real &two, real &min, real &max
   ) {
      const real v = r * op::sqrt(b*b + c*c);  min = two - v;
      const real u = op::twice(op::abs(a)*h);  max = two + v;

      if (v <= u) {
         const real at = v*v/(u+u);
         min = op::min(min, one - at);
         max = op::max(max, one + at);
      }
   }
}

// aabb
kip_aabb(paraboloid)
{
   real xmin, ymin, zmin;  const rotate<2,real> rot2(a,b);
   real xmax, ymax, zmax;  using namespace internal;
   // zzz Should probably avoid ANY "using namespace" (as above) in .h codes

   bound_paraboloid(rot2.m1.x, rot2.m2x, rot2.m3.x, rot2.h,r, a.x,b.x, xmin,xmax);
   bound_paraboloid(rot2.m1.y, rot2.m2y, rot2.m3.y, rot2.h,r, a.y,b.y, ymin,ymax);
   bound_paraboloid(rot2.m1.z, real(0),     rot2.m3.z, rot2.h,r, a.z,b.z, zmin,zmax);

   return kip::bbox<real>(
      true,xmin, xmax,true,
      true,ymin, ymax,true,
      true,zmin, zmax,true
   );
} kip_end



// inside
kip_inside(paraboloid)
{
   return 0 <= rot.ex && rot.ex <= rot.h && h3 <= 0;
} kip_end



// dry
kip_dry(paraboloid)
{
   real az;  if (seg.lt(a,az)) return false;
   real bz;  if (seg.lt(b,bz)) return false;

   const real tmp = h1*h1*(rot.h-az+bz)*(rot.h+az-bz);
   az -= seg.c;
   bz -= seg.c;
   return bz*bz >= tmp && (bz <= az || az*az >= tmp || 4*(bz-az)*az >= tmp);
} kip_end



// check
kip_check(paraboloid)
{
   return internal::onetwor_check<real>("Paraboloid", *this);
} kip_end



// random
kip_random(paraboloid)
{
   return internal::random_abr<real,tag>(obj);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(paraboloid)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (interior) {

      // x = h
      if (dx > 0) {
         q = (rot.h-rot.ex)/dx;
         if (0 < q && q < qmin) {
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            if (op::sq(q.y) + op::sq(q.z) <= rsq) {
               q.x = rot.h;
               return q(1,0,0, this, yesnorm), true;
            }
         }
      }

      // curve
      const real f = h2*(1-dx*dx), g = dx - h2*dy*rot.ey, s = g*g - f*h3;
      if (s < 0 || f == 0) return false;

      q = (g + op::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.x = rot.ex + q*dx;  // inside, so don't need the range check

   } else {

      // Outside...

      // x = h
      if (rot.ex > rot.h) {
         if (dx >= 0) return false;
         q = (rot.h - rot.ex)/dx;
         if (!(0 < q && q < qmin)) return false;

         q.y = rot.ey + q*dy;
         q.z = q*tar.z;

         if (op::sq(q.y) + op::sq(q.z) <= rsq) {
            q.x = rot.h;
            return q(1,0,0, this, yesnorm), true;
         }
      }

      // curve
      const real f = h2*(1-dx*dx), g = dx - h2*dy*rot.ey, s = g*g - f*h3;
      if (s < 0 || f == 0) return false;

      q = (g - op::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.x = rot.ex + real(q)*dx;
      if (!(0 <= q.x && q.x <= rot.h)) return false;
   }

   /*
   paraboloid:
      (h/r^2)*(y^2 + z^2) - x = 0

      partial x = -1
      partial y =  2*h/r^2 * y = h2*y
      partial z =  2*h/r^2 * z = h2*z

   can scale by 1/h2 to get normal.
   */

   q.y = rot.ey + real(q)*dy;
   q.z = real(q)*tar.z;
   return q(-1/h2, q.y, q.z, this, nonorm), true;
} kip_end



// -----------------------------------------------------------------------------
// get_baseh
// get_curve
// -----------------------------------------------------------------------------

// get_baseh
template<class real, class tag>
inline bool paraboloid<real,tag>::get_baseh(
   const point<real> &tar,
   const real dx,
   const real dy,
   const real qmin,
   inq<real,tag> &info
) const {
   // compute & check
   if (dx == 0) return false;
   info.q = (rot.h-rot.ex)/dx;
   if (!(0 < info.q && info.q < qmin)) return false;

   info.y = rot.ey + dy*info.q;
   info.z = tar.z*info.q;
   if (op::sq(info.y) + op::sq(info.z) <= rsq) {
      info.x = rot.h;
      return info(1,0,0, this, yesnorm), true;
   }
   return false;
}



// get_curve
template<class real, class tag>
inline bool paraboloid<real,tag>::get_curve(
   const point<real> &tar,
   const real dx,
   const real dy,
   const real qmin,
   inq<real,tag> &info
) const {
   // check
   if (!(0 < info.q && info.q < qmin)) return false;

   info.x = rot.ex + info.q*dx;
   if (!(0 <= info.x && info.x <= rot.h)) return false;

   info.y = rot.ey + info.q*dy;
   info.z = info.q*tar.z;

   return info(-1/h2, info.y, info.z, this, nonorm), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(paraboloid)
{
   if (interior) return paraboloid<real,tag>::infirst(
      kip_etd,qmin,ints.one(),insub);

   // Outside...
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   const real f = h2*(1-dx*dx), g = dx - h2*dy*rot.ey, s = g*g - f*h3;
   // zzz might actually be a problem with the f == 0 test (cone and cyl, too?)
   if (s < 0 || (rot.ex > rot.h && dx >= 0) || f == 0) return false;
   const real tmp = op::sqrt(s);

   ints.convex();
   inq<real,tag> q1((g-tmp)/f);
   inq<real,tag> q2((g+tmp)/f), qf;

   if ((get_curve(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
       (get_curve(tar,dx,dy,qmin,q2) && ints.convex(q2)) ||
       (get_baseh(tar,dx,dy,qmin,qf) && ints.convex(qf)))
      return true;

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(paraboloid) {

   // (
   //    a.x, a.y, a.z,
   //    b.x, b.y, b.z,
   //    r,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.a, "apex coordinate") && read_comma(s) &&
      read_value(s, obj.b, "base coordinate") && read_comma(s) &&
      read_value(s, obj.r, "radius") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(paraboloid) {
   return kip::internal::onetwor_write(k,obj, obj.a,obj.b,obj.r, "paraboloid");
}

}

#define   kip_class paraboloid
#include "kip-macro-io.h"
