
// -----------------------------------------------------------------------------
// pill
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class pill : public shape<real,tag> {
   bool inside(const point<real> &) const;

   // modified pill: (0,0,0), (h,0,0), r
   mutable rotate<3,real,op::full,op::unscaled> rot;
   mutable real rsq, h1, h2, h3;

   // get_*
   bool
   get_hemi0(
      const point<real> &,
      const real, const real, const real,
      inq<real,tag> &
   ) const,
   get_hemih(
      const point<real> &,
      const real, const real, const real,
      inq<real,tag> &
   ) const,
   get_curve(
      const point<real> &,
      const real, const real, const real,
      inq<real,tag> &
   ) const;

public:
   using shape<real,tag>::basic;
   kip_functions(pill);

   // point a, point b, radius
   point<real> a;
   point<real> b;
   real r;

   point<real> back(const point<real> &from) const
   {
      return rot.back(from);
   }

#define   kip_class pill
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(pill)
{
   rot = rotate<3,real,op::full,op::unscaled>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   rsq = r*r;
   h1 = (r-rot.ey)*(r+rot.ey);
   h2 = h1 - rot.ex*rot.ex;
   h3 = h2 - rot.h*(rot.h - rot.ex - rot.ex);

   // interior
   this->interior = inside(eyeball);

   // minimum
   return std::abs(
      rot.ex < 0     ? std::sqrt(rsq-h2) - r
    : rot.ex > rot.h ? std::sqrt(rsq-h3) - r
    : rot.ey - r
   );
} kip_end



// aabb
kip_aabb(pill)
{
   return bbox<real>(
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
   return detail::onetwor_check<real>("Pill", *this);
} kip_end



// randomize
kip_randomize(pill)
{
   return detail::random_abr<real,tag>(obj);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(pill)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (this->interior) {

      // Inside...

      // x = 0
      const real f = dx*rot.ex + dy*rot.ey, disc0 = f*f + h2;
      if (disc0 >= 0) {
         // whole sphere is hit somewhere
         q = std::sqrt(disc0) - f;  // only care about furthest intersection
         if (!(q < qmin)) return false;
         if (q > 0) {
            q.inter.x = rot.ex + q*dx;
            if (q.inter.x <= 0) {
               q.inter.y = rot.ey + q*dy;
               q.inter.z = q*tar.z;
               return q.set(q.inter, this, normalized::no), true;
            }
         }
      }

      // x = h
      const real g = dx*rot.h - f, disc2 = g*g + h3;
      if (disc2 >= 0) {
         // whole sphere is hit somewhere
         q = g + std::sqrt(disc2);  // only care about furthest intersection
         if (!(q < qmin)) return false;
         if (q > 0) {
            q.inter.x = rot.ex + q*dx;
            if (q.inter.x >= rot.h) {
               q.inter.y = rot.ey + q*dy;
               q.inter.z = q*tar.z;
               return q.set(
                  q.inter.x-rot.h, q.inter.y, q.inter.z,
                  this, normalized::no
               ), true;
            }
         }
      }

      // curve
      const real c = dy*dy, d = tar.z*tar.z, s = c*rsq + d*h1;
      if (s < 0 || c+d == 0) return false;

      q = (std::sqrt(s)- rot.ey*dy)/(c+d);
      if (!(0 < q && q < qmin)) return false;
      q.inter.x = rot.ex + q*dx;  // inside, so don't need the range check

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
         q = -(f + std::sqrt(disc0));
         if (!(0 < q)) return false;  // would be <= 0 for cyl. portion, too

         q.inter.x = rot.ex + q*dx;
         if (q.inter.x <= 0) {
            if (!(q < qmin)) return false;
            q.inter.y = rot.ey + q*dy;
            q.inter.z = q*tar.z;
            return q.set(q.inter, this, normalized::no), true;
         }
      }

      // x = h
      const real g = dx*rot.h - f, disc2 = g*g + h3;
      if (disc2 >= 0) {
         // whole sphere is hit somewhere
         q = g - std::sqrt(disc2);
         if (!(0 < q)) return false;  // would be <= 0 for cyl. portion, too

         q.inter.x = rot.ex + q*dx;
         if (q.inter.x >= rot.h) {
            if (!(q < qmin)) return false;
            q.inter.y = rot.ey + q*dy;
            q.inter.z = q*tar.z;
            return q.set(
               q.inter.x-rot.h, q.inter.y, q.inter.z,
               this, normalized::no
            ), true;
         }
      }

      // curve
      if (c+d == 0) return false;

      q = -(rot.ey*dy + std::sqrt(s))/(c+d);
      if (!(0 < q && q < qmin)) return false;
      q.inter.x = rot.ex + q*dx;
      if (!(0 <= q.inter.x && q.inter.x <= rot.h)) return false;
   }

   q.inter.y = rot.ey + q*dy;
   q.inter.z = q*tar.z;
   return q.set(
      0, q.inter.y, q.inter.z,
      this, normalized::no
   ), true;

} kip_end



// -----------------------------------------------------------------------------
// get_hemi0
// get_hemih
// get_curve
// -----------------------------------------------------------------------------

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

   info.inter.x = rot.ex + info.q*dx;
   if (!(info.inter.x <= 0)) return false;

   info.inter.y = rot.ey + info.q*dy;
   info.inter.z = info.q*tar.z;

   return info.set(info.inter, this, normalized::no), true;
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

   info.inter.x = rot.ex + info.q*dx;
   if (!(info.inter.x >= rot.h)) return false;

   info.inter.y = rot.ey + info.q*dy;
   info.inter.z = info.q*tar.z;

   return info.set(
      info.inter.x-rot.h, info.inter.y, info.inter.z,
      this, normalized::no
   ), true;
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

   info.inter.x = rot.ex + info.q*dx;
   if (!(0 <= info.inter.x && info.inter.x <= rot.h)) return false;

   info.inter.y = rot.ey + info.q*dy;
   info.inter.z = info.q*tar.z;

   return info.set(
      0, info.inter.y, info.inter.z,
      this, normalized::no
   ), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(pill)
{
   if (this->interior) return pill<real,tag>::infirst(
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
      inq<real,tag> q1(-std::sqrt(disc0) - f);
      inq<real,tag> q2( std::sqrt(disc0) - f);

      if ((get_hemi0(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
          (get_hemi0(tar,dx,dy,qmin,q2) && ints.convex(q2)))
         return true;
   }

   // x = h
   const real g = dx*rot.h - f, disc2 = g*g + h3;
   if (disc2 >= 0) {
      // whole sphere is hit somewhere
      inq<real,tag> q1(g - std::sqrt(disc2));
      inq<real,tag> q2(g + std::sqrt(disc2));

      if ((get_hemih(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
          (get_hemih(tar,dx,dy,qmin,q2) && ints.convex(q2)))
         return true;
   }

   // curve
   if (c+d != 0) {
      const real tmp = std::sqrt(s), tmp2 = rot.ey*dy, tmp3 = 1/(c+d);

      inq<real,tag> q1(-(tmp + tmp2)*tmp3);  // -std::sqrt
      if (get_curve(tar,dx,dy,qmin,q1) && ints.convex(q1)) return true;

      if (!ints.size()) return false;

      inq<real,tag> q2( (tmp - tmp2)*tmp3);  // +std::sqrt
      if (get_curve(tar,dx,dy,qmin,q2) && ints.convex(q2)) return true;
   }

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

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
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(pill) {
   return detail::onetwor_write(k,obj, obj.a,obj.b,obj.r, "pill");
}

#define   kip_class pill
#include "kip-macro-io.h"
