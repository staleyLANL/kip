
#pragma once

// -----------------------------------------------------------------------------
// cylinder
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class cylinder : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // modified cylinder: (0,0,0), (h,0,0), r
   mutable rotate<3,real,true> rot;
   mutable real h2;

   // get_*
   inline bool
   get_base0(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_baseh(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_curve(const point<real> &, const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(cylinder);

   // point a, point b, radius
   point<real> a;
   point<real> b;
   real r;

   inline point<real> back(const point<real> &from) const { return rot.back(r*from); }

#define   kip_class cylinder
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(cylinder)
{
   rot = mod2(a-eyeball) < mod2(b-eyeball)
    ? rotate<3,real,true>(a, b, eyeball, 1/r)
    : rotate<3,real,true>(b, a, eyeball, 1/r);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   h2 = (rot.ey-1)*(rot.ey+1);  // = rot.ey^2 - r^2, with r scaled to 1
   interior = inside(eyeball);

   return r*(
       rot.ex <= 0
    ?  rot.ey <= 1
    ? -rot.ex // west
    :  std::sqrt(op::square(rot.ex) + op::square(rot.ey-1))  // northwest
    :  rot.ey >= 1
    ?  rot.ey - 1  // north
    :  op::min(rot.ex, 1-rot.ey)  // inside
   );
} kip_end



// bound_cylinder - helper
namespace internal {
   template<template<class,class> class shape, class real, class tag>
   bbox<real> bound_cylinder(const shape<real,tag> &obj, const real r)
   {
      point<real> min, max;
      internal::bound_abr(obj.a,obj.b,r, min,max);

      const real dx = obj.b.x - obj.a.x;
      const real dy = obj.b.y - obj.a.y;
      const real dz = obj.b.z - obj.a.z;

      return bbox<real>(
         true, op::min(min.x, min.x+dx),   op::max(max.x, max.x+dx), true,
         true, op::min(min.y, min.y+dy),   op::max(max.y, max.y+dy), true,
         true, op::min(min.z, min.z+dz),   op::max(max.z, max.z+dz), true
      );
   }
}

// aabb
kip_aabb(cylinder)
{
   return internal::bound_cylinder(*this, this->r);
} kip_end



// inside
kip_inside(cylinder)
{
   // ex <= h test not necessary, due to transformation
   return 0 <= rot.ex && h2 <= 0;
} kip_end



// dry
kip_dry(cylinder)
{
   real az;  if (seg.lt(a,az)) return false;
   real bz;  if (seg.lt(b,bz)) return false;

   // Different from e.g. washer and tabular because we use rotate<3,real,true>,
   // where "true" turns on scaling - making rot.h = (its usual value) / r.
   const real tmp = rot.h*r;
   return op::square(rot.h*(op::min(az,bz)-seg.c)) >= (tmp-az+bz)*(tmp+az-bz);
} kip_end



// check
kip_check(cylinder)
{
   return internal::onetwor_check<real>("Cylinder", *this);
} kip_end



// random
kip_random(cylinder)
{
   return internal::random_abr<real,tag>(obj);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(cylinder)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (interior) {

      // x = 0
      if (dx < 0) {
         q = -rot.ex/dx;
         if (0 < q && q < qmin) {
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            if (op::square(q.y) + op::square(q.z) <= 1) {
               q.x = real(0);
               return q(-1,0,0, this, normalized_t::yesnorm, r), true;
            }
         }
      }

      // x = h
      if (dx > 0) {
         q = (rot.h-rot.ex)/dx;
         if (0 < q && q < qmin) {
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            if (op::square(q.y) + op::square(q.z) <= 1) {
               q.x = rot.h;
               return q(1,0,0, this, normalized_t::yesnorm, r), true;
            }
         }
      }

      // curve
      const real c = dy*dy, d = tar.z*tar.z, s = c - d*h2;
      if (s < 0 || c+d == 0) return false;

      q = (std::sqrt(s) - rot.ey*dy)/(c+d);
      if (!(0 < q && q < qmin)) return false;

      q.x = rot.ex + q*dx;  // inside, so don't need the range check

   } else {

      // Outside...
      // see if we even hit the extended cylinder
      const real c = dy*dy, d = tar.z*tar.z, s = c - d*h2;
      if (s < 0) return false;

      // x = 0
      if (rot.ex < 0) {
         if (dx <= 0) return false;
         q = -rot.ex/dx;
         if (!(0 < q && q < qmin)) return false;

         q.y = rot.ey + q*dy;
         q.z = q*tar.z;

         if (op::square(q.y) + op::square(q.z) <= 1) {
            q.x = real(0);
            return q(-1,0,0, this, normalized_t::yesnorm, r), true;
         }
      }

      // curve
      if (c+d == 0) return false;

      q = -(rot.ey*dy + std::sqrt(s))/(c+d);
      if (!(0 < q && q < qmin)) return false;
      q.x = rot.ex + q*dx;
      if (!(0 <= q.x && q.x <= rot.h)) return false;
   }

   q.y = rot.ey + q*dy;
   q.z = q*tar.z;
   return q(0, q.y, q.z, this, normalized_t::nonorm, r), true;
} kip_end



// -----------------------------------------------------------------------------
// get_base0
// get_baseh
// get_curve
// -----------------------------------------------------------------------------

// get_base0
template<class real, class tag>
inline bool cylinder<real,tag>::get_base0(
   const point<real> &tar,
   const real dx,
   const real dy,
   const real qmin,
   inq<real,tag> &info
) const {
   // compute & check
   if (dx == 0) return false;
   info.q = -rot.ex/dx;
   if (!(0 < info.q && info.q < qmin)) return false;

   info.y = rot.ey + dy*info.q;
   info.z = tar.z*info.q;
   if (op::square(info.y) + op::square(info.z) <= 1) {
      info.x = real(0);
      return info(-1,0,0, this, normalized_t::yesnorm, r), true;
   }
   return false;
}



// get_baseh
template<class real, class tag>
inline bool cylinder<real,tag>::get_baseh(
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
   if (op::square(info.y) + op::square(info.z) <= 1) {
      info.x = rot.h;
      return info(1,0,0, this, normalized_t::yesnorm, r), true;
   }
   return false;
}



// get_curve
template<class real, class tag>
inline bool cylinder<real,tag>::get_curve(
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

   return info(0, info.y, info.z, this, normalized_t::nonorm, r), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(cylinder)
{
   if (interior) return cylinder<real,tag>::infirst(
      kip_etd,qmin,ints.one(),insub);

   // Outside...
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (rot.ex < 0 && dx <= 0)
      return false;

   ints.convex();
   inq<real,tag> q1, q2;

   if ((get_base0(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
       (get_baseh(tar,dx,dy,qmin,q2) && ints.convex(q2)))
      return true;

   const real c = dy*dy, d = tar.z*tar.z, s = c - d*h2;
   if (s >= 0 && c+d != 0) {
      const real tmp = std::sqrt(s), tmp2 = rot.ey*dy, tmp3 = 1/(c+d);

      inq<real,tag> q3(-(tmp + tmp2)*tmp3);  // -std::sqrt
      if (get_curve(tar,dx,dy,qmin,q3) && ints.convex(q3)) return true;

      if (!ints.size()) return false;

      inq<real,tag> q4( (tmp - tmp2)*tmp3);  // +std::sqrt
      if (get_curve(tar,dx,dy,qmin,q4) && ints.convex(q4)) return true;
   }

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(cylinder) {

   // (
   //    a.x, a.y, a.z,
   //    b.x, b.y, b.z,
   //    r,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.a, "\"a\" endpoint") && read_comma(s) &&
      read_value(s, obj.b, "\"b\" endpoint") && read_comma(s) &&
      read_value(s, obj.r, "radius") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(cylinder) {
   return internal::onetwor_write(k,obj, obj.a,obj.b,obj.r, "cylinder");
}

#define   kip_class cylinder
#include "kip-macro-io.h"
