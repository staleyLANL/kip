
// -----------------------------------------------------------------------------
// silo
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class silo : public shape<real,tag> {
   bool inside(const point<real> &) const;

   // modified silo: (0,0,0), (h,0,0), r
   mutable rotate<3,real,op::full,op::unscaled> rot;
   mutable real rsq, hsq, h1, h2, m;

   // get_*
   bool
   get_hemi0(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const,
   get_baseh(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const,
   get_curve(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(silo);

   // point a, point b, radius
   point<real> a, b;
   real r;

   point<real> back(const point<real> &from) const
   {
      return rot.back(from);
   }

#define   kip_class silo
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(silo)
{
   rot = rotate<3,real,op::full,op::unscaled>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   rsq = r*r;
   hsq = rot.h*rot.h;
   h1 = rsq/hsq;
   h2 = (rot.ey-r)*(rot.ey+r);  // = rot.ey^2 - r^2
   m = h2 + rot.ex*rot.ex;
   // m = rot.ey^2 - r^2 + rot.ex^2

   // interior
   this->interior = inside(eyeball);

   // minimum
   return
        rot.ex <= 0
      ? std::abs(std::sqrt(rsq+m) - r)  // west, or in hemisphere
      : rot.ex >= rot.h
      ? rot.ey <= r
      ? rot.ex - rot.h  // east
      : std::sqrt(op::square(rot.ex-rot.h) + op::square(rot.ey-r))  // northeast
      : rot.ey >= r
      ? rot.ey - r  // north
      : op::min(rot.h-rot.ex, r-rot.ey)  // inside cylindrical portion
   ;
} kip_end



// aabb
kip_aabb(silo)
{
   point<real> min, max;
   detail::bound_abr(b,a,r, min,max);

   return bbox<real>(
      true, op::min(a.x-r, min.x),   op::max(a.x+r, max.x), true,
      true, op::min(a.y-r, min.y),   op::max(a.y+r, max.y), true,
      true, op::min(a.z-r, min.z),   op::max(a.z+r, max.z), true
   );
} kip_end



// inside
kip_inside(silo)
{
   return m <= 0 || (0 <= rot.ex && rot.ex <= rot.h && h2 <= 0);
} kip_end



// dry
kip_dry(silo)
{
   const real az = seg.forez(a);  if (az < r) return false;  // hemisphere
   real bz;  if (seg.lt(b,bz)) return false;  // flat end
   bz -= seg.valz;
   return bz*bz >= h1*(rot.h-az+bz)*(rot.h+az-bz);
} kip_end



// check
kip_check(silo)
{
   return detail::onetwor_check<real>("Silo", *this);
} kip_end



// randomize
kip_randomize(silo)
{
   return detail::random_abr<real,tag>(obj);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(silo)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (this->interior) {

      // Inside...

      // x = h (base)
      if (dx > 0) {
         q = (rot.h-rot.ex)/dx;
         if (0 < q && q < qmin) {
            q.inter.y = rot.ey + q*dy;
            q.inter.z = q*tar.z;
            if (op::square(q.inter.y) + op::square(q.inter.z) <= rsq) {
               q.inter.x = rot.h;
               return q.set(1,0,0, this, normalized::yes), true;
            }
         }
      }

      // x = 0 (hemisphere)
      const real p = dx*rot.ex + dy*rot.ey, g = p*p - m;
      if (g >= 0) {
         // whole sphere is hit somewhere
         q = std::sqrt(g) - p;  // only care about furthest intersection
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

      // curve
      const real c = dy*dy, d = tar.z*tar.z, s = c*rsq - d*h2;
      if (s < 0 || c+d == 0) return false;

      q = (std::sqrt(s) - rot.ey*dy)/(c+d);
      if (!(0 < q && q < qmin)) return false;
      q.inter.x = rot.ex + q*dx;  // inside, so don't need the range check

   } else {

      // Outside...

      // early exit sometimes possible if ex < -r
      if (rot.ex < -r) {
         if (dx <= 0) return false;
         q = -(r + rot.ex)/dx;
         if (!(0 < q && q < qmin)) return false;
      }

      // early exit if we miss the extended silo
      const real c = dy*dy, d = tar.z*tar.z, s = c*rsq - d*h2;
      if (s < 0) return false;

      // x = h (base)
      if (rot.ex > rot.h) {
         if (dx >= 0) return false;
         q = (rot.h - rot.ex)/dx;
         if (!(0 < q && q < qmin)) return false;

         q.inter.y = rot.ey + q*dy;
         q.inter.z = q*tar.z;

         if (op::square(q.inter.y) + op::square(q.inter.z) <= rsq) {
            q.inter.x = rot.h;
            return q.set(1,0,0, this, normalized::yes), true;
         }
      }

      // x = 0 (hemisphere)
      const real p = dx*rot.ex + dy*rot.ey, g = p*p - m;
      if (g >= 0) {
         // whole sphere is hit somewhere
         q = -(p + std::sqrt(g));
         if (!(0 < q)) return false;  // would be <= 0 for cyl. portion, too

         q.inter.x = rot.ex + q*dx;
         if (q.inter.x <= 0) {
            if (!(q < qmin)) return false;
            q.inter.y = rot.ey + q*dy;
            q.inter.z = q*tar.z;
            return q.set(q.inter, this, normalized::no), true;
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
// get_baseh
// get_curve
// -----------------------------------------------------------------------------

// get_hemi0
template<class real, class tag>
inline bool silo<real,tag>::get_hemi0(
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



// get_baseh
template<class real, class tag>
inline bool silo<real,tag>::get_baseh(
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

   info.inter.y = rot.ey + dy*info.q;
   info.inter.z = tar.z*info.q;

   if (op::square(info.inter.y) + op::square(info.inter.z) <= rsq) {
      info.inter.x = rot.h;
      return info.set(1,0,0, this, normalized::yes), true;
   }
   return false;
}



// get_curve
template<class real, class tag>
inline bool silo<real,tag>::get_curve(
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

kip_inall(silo)
{
   if (this->interior)
      return silo<real,tag>::infirst(etd, insub, qmin, ints.one());

   // Outside...
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   const real c = dy*dy;
   const real d = tar.z*tar.z;
   const real s = c*rsq - d*h2;

   if (s < 0 || (rot.ex < -r && dx <= 0) || (rot.ex > rot.h && dx >= 0))
      return false;  // miss extended silo, or look away

   ints.convex();

   // x = 0 (hemisphere)
   const real p = dx*rot.ex + dy*rot.ey;
   const real g = p*p - m;
   if (g >= 0) {
      // whole sphere is hit somewhere
      inq<real,tag> q1(-std::sqrt(g) - p);
      inq<real,tag> q2( std::sqrt(g) - p);

      if ((get_hemi0(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
          (get_hemi0(tar,dx,dy,qmin,q2) && ints.convex(q2)))
         return true;
   }

   // x = h (base)
   inq<real,tag> q3;
   if (get_baseh(tar,dx,dy,qmin,q3) && ints.convex(q3)) return true;

   // curve
   if (c+d != 0) {
      const real tmp = std::sqrt(s), tmp2 = rot.ey*dy, tmp3 = 1/(c+d);

      inq<real,tag> q4(-(tmp + tmp2)*tmp3);  // -std::sqrt
      if (get_curve(tar,dx,dy,qmin,q4) && ints.convex(q4)) return true;

      if (!ints.size()) return false;

      inq<real,tag> q5( (tmp - tmp2)*tmp3);  // +std::sqrt
      if (get_curve(tar,dx,dy,qmin,q5) && ints.convex(q5)) return true;
   }

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(silo) {

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
kip_ostream(silo) {
   return detail::onetwor_write(k,obj, obj.a,obj.b,obj.r, "silo");
}

#define   kip_class silo
#include "kip-macro-io.h"
