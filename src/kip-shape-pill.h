/* -*- C++ -*- */

#ifndef KIP_SHAPE_PILL
#define KIP_SHAPE_PILL

namespace kip {



// =============================================================================
// pill
// =============================================================================

template<class real = default_real_t, class tag = default_tag_t>
class pill : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // modified pill: (0,0,0), (h,0,0), r
   mutable rotate<3,real> rot;
   mutable real rsq, h1, h2, h3;

   // get_*
   inline bool
   get_hemi0(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_hemih(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_curve(const point<real> &, const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(pill);

   // point a, point b, radius
   point<real> a;
   point<real> b;
   real r;

   inline point<real> back(const point<real> &from) const { return rot.back(from); }

#define   kip_class pill
#include "kip-macro-onetwor-ctor.h"
};



// =============================================================================
// Functions
// =============================================================================

// process
kip_process(pill)
{
   rot = rotate<3,real>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   rsq = r*r;
   h1 = (r-rot.ey)*(r+rot.ey);
   h2 = h1 - rot.ex*rot.ex;
   h3 = h2 - rot.h*(rot.h - rot.ex - rot.ex);

   // interior
   interior = inside(eyeball);

   // minimum
   return op::abs(
      rot.ex < 0     ? op::sqrt(rsq-h2) - r
    : rot.ex > rot.h ? op::sqrt(rsq-h3) - r
    : rot.ey - r
   );
} kip_end



// aabb
kip_aabb(pill)
{
   return kip::bbox<real>(
      true, op::min(a.x-r, b.x-r),   op::max(a.x+r, b.x+r), true,
      true, op::min(a.y-r, b.y-r),   op::max(a.y+r, b.y+r), true,
      true, op::min(a.z-r, b.z-r),   op::max(a.z+r, b.z+r), true
   );
} kip_end



// inside
kip_inside(pill)
{
   return (0 <= rot.ex && rot.ex <= rot.h && h1 >= 0) || h2 >= 0 || h3 >= 0;
} kip_end



// dry
kip_dry(pill)
{
   return
      seg.forez(a) >= r &&
      seg.forez(b) >= r;
} kip_end



// check
kip_check(pill)
{
   return internal::onetwor_check<real>("Pill", *this);
} kip_end



// random
kip_random(pill)
{
   return internal::random_abr<real,tag>(obj);
} kip_end



// =============================================================================
// infirst
// =============================================================================

kip_infirst(pill)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (interior) {

      // Inside...

      // x = 0
      const real f = dx*rot.ex + dy*rot.ey, disc0 = f*f + h2;
      if (disc0 >= 0) {
         // whole sphere is hit somewhere
         q = op::sqrt(disc0) - f;  // only care about furthest intersection
         if (!(q < qmin)) return false;
         if (q > 0) {
            q.x = rot.ex + q*dx;
            if (q.x <= 0) {
               q.y = rot.ey + q*dy;
               q.z = q*tar.z;
               return q(q, this, nonorm), true;
            }
         }
      }

      // x = h
      const real g = dx*rot.h - f, disc2 = g*g + h3;
      if (disc2 >= 0) {
         // whole sphere is hit somewhere
         q = g + op::sqrt(disc2);  // only care about furthest intersection
         if (!(q < qmin)) return false;
         if (q > 0) {
            q.x = rot.ex + q*dx;
            if (q.x >= rot.h) {
               q.y = rot.ey + q*dy;
               q.z = q*tar.z;
               return q(q.x-rot.h, q.y, q.z, this, nonorm), true;
            }
         }
      }

      // curve
      const real c = dy*dy, d = tar.z*tar.z, s = c*rsq + d*h1;
      if (s < 0 || c+d == 0) return false;

      q = (op::sqrt(s)- rot.ey*dy)/(c+d);
      if (!(0 < q && q < qmin)) return false;
      q.x = rot.ex + q*dx;  // inside, so don't need the range check

   } else {

      // Outside...

      // early exit if we miss the extended pill; ex < -r and ex > h+r early
      // exits are possible too, but don't seem to improve the performance.
      const real c = dy*dy, d = tar.z*tar.z, s = c*rsq + d*h1;
      if (s < 0) return false;

      // x = 0
      const real f = dx*rot.ex + dy*rot.ey, disc0 = f*f + h2;
      if (disc0 >= 0) {
         // whole sphere is hit somewhere
         q = -(f + op::sqrt(disc0));
         if (!(0 < q)) return false;  // would be <= 0 for cyl. portion, too

         q.x = rot.ex + q*dx;
         if (q.x <= 0) {
            if (!(q < qmin)) return false;
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            return q(q, this, nonorm), true;
         }
      }

      // x = h
      const real g = dx*rot.h - f, disc2 = g*g + h3;
      if (disc2 >= 0) {
         // whole sphere is hit somewhere
         q = g - op::sqrt(disc2);
         if (!(0 < q)) return false;  // would be <= 0 for cyl. portion, too

         q.x = rot.ex + q*dx;
         if (q.x >= rot.h) {
            if (!(q < qmin)) return false;
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            return q(q.x-rot.h, q.y, q.z, this, nonorm), true;
         }
      }

      // curve
      if (c+d == 0) return false;

      q = -(rot.ey*dy + op::sqrt(s))/(c+d);
      if (!(0 < q && q < qmin)) return false;
      q.x = rot.ex + q*dx;
      if (!(0 <= q.x && q.x <= rot.h)) return false;
   }

   q.y = rot.ey + q*dy;
   q.z = q*tar.z;
   return q(0, q.y, q.z, this, nonorm), true;
} kip_end



// =============================================================================
// get_hemi0
// get_hemih
// get_curve
// =============================================================================

// get_hemi0
template<class real, class tag>
inline bool pill<real,tag>::get_hemi0(
   const point<real> &tar,
   const real dx,
   const real dy,
   const real qmin,
   inq<real,tag> &info
) const {
   // check
   if (!(0 < info.q && info.q < qmin)) return false;

   info.x = rot.ex + info.q*dx;
   if (!(info.x <= 0)) return false;

   info.y = rot.ey + info.q*dy;
   info.z = info.q*tar.z;

   return info(info, this, nonorm), true;
}



// get_hemih
template<class real, class tag>
inline bool pill<real,tag>::get_hemih(
   const point<real> &tar,
   const real dx,
   const real dy,
   const real qmin,
   inq<real,tag> &info
) const {
   // check
   if (!(0 < info.q && info.q < qmin)) return false;

   info.x = rot.ex + info.q*dx;
   if (!(info.x >= rot.h)) return false;

   info.y = rot.ey + info.q*dy;
   info.z = info.q*tar.z;

   return info(info.x-rot.h, info.y, info.z, this, nonorm), true;
}



// get_curve
template<class real, class tag>
inline bool pill<real,tag>::get_curve(
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

   return info(0, info.y, info.z, this, nonorm), true;
}



// =============================================================================
// inall
// =============================================================================

kip_inall(pill)
{
   if (interior) return pill<real,tag>::infirst(
      kip_etd,qmin,ints.one(),insub);

   // Outside...
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   const real c = dy*dy;
   const real d = tar.z*tar.z;
   const real s = c*rsq + d*h1;

   if (s < 0 || (rot.ex < -r && dx <= 0) || (rot.ex > rot.h+r && dx >= 0))
      return false;  // miss extended pill, or look away

   ints.convex();

   // x = 0
   const real f = dx*rot.ex + dy*rot.ey, disc0 = f*f + h2;
   if (disc0 >= 0) {
      // whole sphere is hit somewhere
      inq<real,tag> q1(-op::sqrt(disc0) - f);
      inq<real,tag> q2( op::sqrt(disc0) - f);

      if ((get_hemi0(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
          (get_hemi0(tar,dx,dy,qmin,q2) && ints.convex(q2)))
         return true;
   }

   // x = h
   const real g = dx*rot.h - f, disc2 = g*g + h3;
   if (disc2 >= 0) {
      // whole sphere is hit somewhere
      inq<real,tag> q1(g - op::sqrt(disc2));
      inq<real,tag> q2(g + op::sqrt(disc2));

      if ((get_hemih(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
          (get_hemih(tar,dx,dy,qmin,q2) && ints.convex(q2)))
         return true;
   }

   // curve
   if (c+d != 0) {
      const real tmp = op::sqrt(s), tmp2 = rot.ey*dy, tmp3 = 1/(c+d);

      inq<real,tag> q1(-(tmp + tmp2)*tmp3);  // -op::sqrt
      if (get_curve(tar,dx,dy,qmin,q1) && ints.convex(q1)) return true;

      if (!ints.size()) return false;

      inq<real,tag> q2( (tmp - tmp2)*tmp3);  // +op::sqrt
      if (get_curve(tar,dx,dy,qmin,q2) && ints.convex(q2)) return true;
   }

   return ints.size() > 0;
} kip_end



// =============================================================================
// i/o
// =============================================================================

// read_value
kip_read_value(pill) {

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
      addendum("Detected while reading "+description, diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(pill) {
   return kip::internal::onetwor_write(k,obj, obj.a,obj.b,obj.r, "pill");
}

}

#define   kip_class pill
#include "kip-macro-io.h"

#endif
