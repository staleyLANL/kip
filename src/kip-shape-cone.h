
#pragma once

// -----------------------------------------------------------------------------
// cone
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class cone : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // modified cone: (0,0,0), (h,0,0), r
   mutable rotate<3,real> rot;
   mutable real rsq, h1, h2;

   // get_*
   inline bool
   get_baseh(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_curve(const point<real> &, const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(cone);

   // point a, point b, radius
   point<real> a, b;
   real r;

   inline point<real> back(const point<real> &from) const { return rot.back(from); }

#define   kip_class cone
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(cone)
{
   rot = rotate<3,real>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   const real hsq = rot.h*rot.h;
   const real tmp = r*rot.ex/rot.h;

   rsq =  r*r;
   h1  = -rsq/hsq;
   h2  = (rot.ey - tmp)*(rot.ey + tmp);

   interior = inside(eyeball);

   return
       rot.ex >= rot.h && rot.ey <= r
    ?  rot.ex - rot.h  // east
    :  r*rot.ey + rot.h*rot.ex <= 0
    ?  std::sqrt(rot.ex*rot.ex + rot.ey*rot.ey)  // southwest
    :  rot.h*(rot.h - rot.ex)/r + r - rot.ey <= 0
    ?  std::sqrt(op::square(rot.ex-rot.h) + op::square(rot.ey-r))  // northeast
    :  rot.h*rot.ey - r*rot.ex >= 0
    ?  std::abs(r*rot.ex - rot.h*rot.ey)/std::sqrt(rsq+hsq)  // outside
    :  op::min(  // inside...
          std::abs(r*rot.ex - rot.h*rot.ey)/std::sqrt(rsq+hsq), rot.h-rot.ex
       )
    ;
} kip_end



// aabb
kip_aabb(cone)
{
   point<real> min, max;
   internal::bound_abr(b,a,r, min,max);

   return bbox<real>(
      true, op::min(a.x,min.x),   op::max(a.x,max.x), true,
      true, op::min(a.y,min.y),   op::max(a.y,max.y), true,
      true, op::min(a.z,min.z),   op::max(a.z,max.z), true
   );
} kip_end



// inside
kip_inside(cone)
{
   return 0 <= rot.ex && rot.ex <= rot.h && h2 <= 0;
} kip_end



// dry
kip_dry(cone)
{
   real az;  if (seg.lt(a,az)) return false;
   real bz;  if (seg.lt(b,bz)) return false;
   return op::square(bz-seg.c) >= h1*(az-bz-rot.h)*(az-bz+rot.h);
} kip_end



// check
kip_check(cone)
{
   return internal::onetwor_check<real>("Cone", *this);
} kip_end



// randomize
kip_randomize(cone)
{
   return internal::random_abr<real,tag>(obj);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(cone)
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
            if (op::square(q.y) + op::square(q.z) <= rsq) {
               q.x = rot.h;
               return q(1,0,0, this, normalized_t::yesnorm), true;
            }
         }
      }

      // curve
      const real f = dx*dx*(1-h1)-1, g = rot.ey*dy + h1*rot.ex*dx, s = g*g + f*h2;
      if (s < 0 || f == 0) return false;

      q = (g - std::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.x = rot.ex + q*dx;  // inside, so don't need the range check

   } else {

      // Outside...
      // zzz

      // x = h
      if (rot.ex > rot.h) {
         if (dx >= 0) return false;
         q = (rot.h - rot.ex)/dx;
         if (!(0 < q && q < qmin)) return false;

         q.y = rot.ey + q*dy;
         q.z = q*tar.z;

         if (op::square(q.y) + op::square(q.z) <= rsq) {
            q.x = rot.h;
            return q(1,0,0, this, normalized_t::yesnorm), true;
         }
      }

      // curve
      const real f = dx*dx*(1-h1)-1, g = rot.ey*dy + h1*rot.ex*dx, s = g*g + f*h2;
      if (s < 0 || f == 0) return false;

      q = (g + std::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.x = rot.ex + q*dx;
      if (!(0 <= q.x && q.x <= rot.h)) return false;

   }

   q.y = rot.ey + q*dy;
   q.z = q*tar.z;
   return q(h1*q.x, q.y, q.z, this, normalized_t::nonorm), true;
} kip_end



// -----------------------------------------------------------------------------
// get_baseh
// get_curve
// -----------------------------------------------------------------------------

// get_baseh
template<class real, class tag>
inline bool cone<real,tag>::get_baseh(
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
   if (op::square(info.y) + op::square(info.z) <= rsq) {
      info.x = rot.h;
      return info(1,0,0, this, normalized_t::yesnorm), true;
   }
   return false;
}



// get_curve
template<class real, class tag>
inline bool cone<real,tag>::get_curve(
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

   return info(h1*info.x, info.y, info.z, this, normalized_t::nonorm), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(cone)
{
   if (interior) return cone<real,tag>::infirst(
      kip_etd,qmin,ints.one(),insub);

   // Outside...
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   const real f = dx*dx*(1-h1)-1;
   const real g = rot.ey*dy + h1*rot.ex*dx;
   const real s = g*g + f*h2;

   if (s < 0 || (rot.ex > rot.h && dx >= 0) || f == 0) return false;
   const real tmp = std::sqrt(s);

   ints.convex();
   inq<real,tag> q1((g+tmp)/f);
   inq<real,tag> q2((g-tmp)/f), qf;

   if (
      (get_curve(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
      (get_curve(tar,dx,dy,qmin,q2) && ints.convex(q2)) ||
      (get_baseh(tar,dx,dy,qmin,qf) && ints.convex(qf))
   )
      return true;

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(cone) {

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
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(cone) {
   return internal::onetwor_write(k,obj, obj.a,obj.b,obj.r, "cone");
}

#define   kip_class cone
#include "kip-macro-io.h"
