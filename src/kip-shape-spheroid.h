
#pragma once

// -----------------------------------------------------------------------------
// spheroid
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class spheroid : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // modified spheroid: (-h,0,0), (h,0,0), r
   mutable rotate<3,real> rot;
   mutable real irsq, ihsq, eyr, exh, i;
   mutable real r2h2, rsq;

   // get_curve
   inline bool
   get_curve(const point<real> &, const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(spheroid);

   // point a, point b, radius
   point<real> a;
   point<real> b;
   real r;

   inline point<real> back(const point<real> &from) const { return rot.back(from); }

#define   kip_class spheroid
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(spheroid)
{
   rot = rotate<3,real>(real(0.5)*(a+b), b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   const real bs = op::max(r,rot.h);  // radius of bounding sphere
   rsq  = r*r;
   irsq = 1/rsq;
   ihsq = 1/rot.h/rot.h;
   eyr  = rot.ey*irsq;
   exh  = rot.ex*ihsq;
   i    = rot.ex*exh + rot.ey*eyr - real(1);
   r2h2 = r/(rot.h+rot.h);

   // The minimum estimate below can be improved (generally made larger),
   // but finding it exactly involves solving a cubic equation, which we
   // won't bother with at the moment.

   // eyeball's distance to 0
   const real d0 = op::sqrt(op::sq(rot.ex) + op::sq(rot.ey));

   // interior
   interior = inside(eyeball);

   // minimum
   if (rot.ex >= rot.h || rot.ey >= r || rot.ex <= -rot.h) {
      // OUTSIDE BOUNDING BOX (implies outside the object itself)

      // distance to bounding box
      const real bb =
          rot.ex <= -rot.h
       ?  rot.ey >= r
       ?  op::sqrt(op::sq(rot.ex + rot.h) + op::sq(rot.ey - r))  // northwest
       : -rot.h - rot.ex  // west
       :  rot.ex >= rot.h
       ?  rot.ey >= r
       ?  op::sqrt(op::sq(rot.ex - rot.h) + op::sq(rot.ey - r))  // northeast
       :  rot.ex - rot.h  // east
       :  rot.ey - r  // north
       ;

      return d0 <= bs
         ? bb                 // INSIDE  bounding sphere
         : op::max(bb,d0-bs)  // OUTSIDE bounding sphere
         ;

   } else {
      // INSIDE BOUNDING BOX

      if (d0 <= bs) {
         // INSIDE BOUNDING SPHERE
         const real is = op::min(rot.h,r);  // radius of inner sphere
         return is <= d0
            ? 0      // too bad
            : is-d0  // inside inner sphere
            ;
      } else {
         // OUTSIDE BOUNDING SPHERE
         return d0-bs;
      }
   }
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// bound_spheroid - helper
namespace internal {
   template<class real>
   inline real bound_spheroid(
      const real &a, const real &b, const real &c,
      const real &hsq, const real &rsq
   ) {
      return op::sqrt(a*a*hsq + (b*b + c*c)*rsq);
   }
}

// aabb
kip_aabb(spheroid)
{
   const rotate<2,real> rot2(real(0.5)*(a+b), b);
   const real hsq = rot2.h*rot2.h, rsq = r*r;

   const real xval =
      internal::bound_spheroid(rot2.m1.x, rot2.m2x, rot2.m3.x, hsq,rsq);
   const real yval =
      internal::bound_spheroid(rot2.m1.y, rot2.m2y, rot2.m3.y, hsq,rsq);
   const real zval =
      internal::bound_spheroid(rot2.m1.z, real(0),  rot2.m3.z, hsq,rsq);

   return kip::bbox<real>(
      true, rot2.o.x-xval,   rot2.o.x+xval, true,
      true, rot2.o.y-yval,   rot2.o.y+yval, true,
      true, rot2.o.z-zval,   rot2.o.z+zval, true
   );
} kip_end



// inside
kip_inside(spheroid)
{
   return i <= 0;
} kip_end



// dry
kip_dry(spheroid)
{
   real az;  if (seg.lt(a,az)) return false;
   real bz;  if (seg.lt(b,bz)) return false;
   return (bz-seg.c)*(az-seg.c) + op::sq((az-bz)*r2h2) >= rsq;
} kip_end



// check
kip_check(spheroid)
{
   return internal::onetwor_check<real>("Spheroid", *this);
} kip_end



// random
kip_random(spheroid)
{
   return internal::random_abr<real,tag>(obj);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(spheroid)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   const real g = dx*dx*(irsq-ihsq) - irsq;
   const real p = dx*exh + dy*eyr;
   const real s = p*p + g*i;
   if (s < 0 || g == 0) return false;

   if (interior)
      q = (p - op::sqrt(s))/g;
   else
      q = (p + op::sqrt(s))/g;
   if (!(0 < q && q < qmin)) return false;

   q.x = rot.ex + q*dx;
   q.y = rot.ey + q*dy;
   q.z = q*tar.z;

   return q(ihsq*q.x, irsq*q.y, irsq*q.z, this, normalized_t::nonorm), true;
} kip_end



// -----------------------------------------------------------------------------
// get_curve
// -----------------------------------------------------------------------------

template<class real, class tag>
inline bool spheroid<real,tag>::get_curve(
   const point<real> &tar,
   const real dx, const real dy,
   const real qmin,
   inq<real,tag> &info
) const {
   // check
   if (!(0 < info.q && info.q < qmin)) return false;

   // intersection
   info.x = rot.ex + info.q*dx;
   info.y = rot.ey + info.q*dy;
   info.z = info.q*tar.z;

   // normal
   return info(ihsq*info.x, irsq*info.y, irsq*info.z, this, normalized_t::nonorm), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(spheroid)
{
   if (interior) return spheroid<real,tag>::infirst(
      kip_etd,qmin,ints.one(),insub);

   // Outside...
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   const real g = dx*dx*(irsq-ihsq) - irsq;
   const real p = dx*exh + dy*eyr;
   const real s = p*p + g*i;
   if (s < 0 || g == 0) return false;
   const real tmp = op::sqrt(s);

   ints.convex();

   inq<real,tag> q1((p+tmp)/g);
   if (get_curve(tar,dx,dy,qmin,q1) && ints.convex(q1)) return true;

   if (!ints.size()) return false;

   inq<real,tag> q2((p-tmp)/g);
   if (get_curve(tar,dx,dy,qmin,q2) && ints.convex(q2)) return true;

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(spheroid) {

   // (
   //    a.x, a.y, a.z,
   //    b.x, b.y, b.z,
   //    r,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.a, "\"a\" pole") && read_comma(s) &&
      read_value(s, obj.b, "\"b\" pole") && read_comma(s) &&
      read_value(s, obj.r, "radius") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(spheroid) {
   return kip::internal::onetwor_write(k,obj, obj.a,obj.b,obj.r, "spheroid");
}

#define   kip_class spheroid
#include "kip-macro-io.h"
