
// -----------------------------------------------------------------------------
// bicylinder
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class bicylinder : public shape<real,tag> {
   bool inside(const point<real> &) const;

   // modified bicylinder: (0,0,0), (h,0,0), r.a,r.b
   mutable rotate<3,real,op::full,op::unscaled> rot;
   mutable real rasq, rbsq, hsq, rao, rbo, slope, h1, h2, h3;

   // get_*
   bool
   get_base0(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const,
   get_baseh(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const,
   get_curve(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(bicylinder);

   // point a, point b, a and b radii
   point<real> a, b;
   class { public: real a, b; } r;

   point<real> back(const point<real> &from) const
      { return rot.back(from); }



   // bicylinder([a[,b[,r.a,r.b[,base]]]])
   explicit bicylinder(
      const point<real> &_a = point<real>(real(0),real(0),real(0)),
      const point<real> &_b = point<real>(real(1),real(0),real(0))
   ) :
      shape<real,tag>(this),
      a(_a),
      b(_b)
   {
      r.a = real(1);
      r.b = real(2);
   }

   explicit bicylinder(
      const point<real> &_a,
      const point<real> &_b,
      const real _ra,
      const real _rb
   ) :
      shape<real,tag>(this),
      a(_a),
      b(_b)
   {
      r.a = _ra;
      r.b = _rb;
   }

   explicit bicylinder(
      const point<real> &_a,
      const point<real> &_b,
      const real _ra,
      const real _rb, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      a(_a),
      b(_b)
   {
      r.a = _ra;
      r.b = _rb;
   }



   // bicylinder(a.x,a.y,a.z[,b.x,b.y,b.z[,r.a,r.b[,base]]])
   explicit bicylinder(
      const real ax, const real ay, const real az
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(real(1),real(0),real(0))
   {
      r.a = real(1);
      r.b = real(2);
   }

   explicit bicylinder(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(bx,by,bz)
   {
      r.a = real(1);
      r.b = real(2);
   }

   explicit bicylinder(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz,
      const real _ra, const real _rb
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(bx,by,bz)
   {
      r.a = _ra;
      r.b = _rb;
   }

   explicit bicylinder(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz,
      const real _ra, const real _rb, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      a(ax,ay,az),
      b(bx,by,bz)
   {
      r.a = _ra;
      r.b = _rb;
   }



   // bicylinder(bicylinder)
   bicylinder(const bicylinder &from) :
      shape<real,tag>(from),
      a(from.a),
      b(from.b),
      r(from.r)
   { }

   // bicylinder = bicylinder
   bicylinder &operator=(const bicylinder &from)
   {
      this->shape<real,tag>::operator=(from);
      a = from.a;
      b = from.b;  r = from.r;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(bicylinder)
{
   rot = rotate<3,real,op::full,op::unscaled>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   rasq = r.a*r.a;
   rbsq = r.b*r.b;

   hsq = rot.h*rot.h;
   rao = rasq/hsq;
   rbo = rbsq/hsq;  slope = (r.b-r.a)/rot.h;

   const real tmp = r.a + slope*rot.ex;
   h1 = 1 + slope*slope;
   h2 = slope*tmp;
   h3 = (rot.ey - tmp)*(rot.ey + tmp);

   this->interior = inside(eyeball);

   // minimum
   if (rot.ex <= 0 && rot.ey <= r.a)
      return -rot.ex;  // west

   if (rot.ex >= rot.h && rot.ey <= r.b)
      return rot.ex - rot.h;  // east

   if (0 <= rot.ex && rot.ex <= rot.h && h3 <= 0)
      // inside
      return op::min(
         rot.ex,
         rot.h-rot.ex,
         std::abs(slope*rot.ex + r.a - rot.ey)/std::sqrt(h1)
      );

   if ((r.a-r.b)*(rot.ey-r.a) >= rot.h*rot.ex)
      return std::sqrt(op::square(rot.ex) + op::square(rot.ey-r.a));  // nw

   if ((r.a-r.b)*(rot.ey-r.b) <= rot.h*(rot.ex-rot.h))
      return std::sqrt(op::square(rot.ex-rot.h) + op::square(rot.ey-r.b)); // ne

   return std::abs(slope*rot.ex+r.a-rot.ey)/std::sqrt(h1);  // north
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// bound_bicylinder - helper
namespace detail {
   template<template<class,class> class shape, class real, class tag>
   bbox<real> bound_bicylinder(
      const shape<real,tag> &obj,
      const real ra,
      const real rb
   ) {
      point<real> amin, amax;  detail::bound_abr(obj.a, obj.b, ra, amin,amax);
      point<real> bmin, bmax;  detail::bound_abr(obj.b, obj.a, rb, bmin,bmax);

      return bbox<real>(
         true, op::min(amin.x, bmin.x),   op::max(amax.x, bmax.x), true,
         true, op::min(amin.y, bmin.y),   op::max(amax.y, bmax.y), true,
         true, op::min(amin.z, bmin.z),   op::max(amax.z, bmax.z), true
      );
   }
}



// aabb
kip_aabb(bicylinder)
{
   return detail::bound_bicylinder(*this, this->r.a, this->r.b);
} kip_end



// inside
kip_inside(bicylinder)
{
   return 0 <= rot.ex && rot.ex <= rot.h && h3 <= 0;
} kip_end



// dry
kip_dry(bicylinder)
{
   real az;  if (seg.lt(a,az)) return false;
   real bz;  if (seg.lt(b,bz)) return false;

   const real tmp = (rot.h-az+bz) * (rot.h+az-bz);
   az -= seg.valz;
   bz -= seg.valz;
   return az*az >= rao*tmp && bz*bz >= rbo*tmp;
} kip_end



// check
kip_check(bicylinder)
{
   diagnostic rv = diagnostic::good;

   // r.a
   if (r.a < real(0)) {
      std::ostringstream oss;
      oss << "Bicylinder has negative \"a\" radius r.a=" << r.a;
      rv = error(oss);
   }

   // r.b
   if (r.b < real(0)) {
      std::ostringstream oss;
      oss << "Bicylinder has negative \"b\" radius r.b=" << r.b;
      rv = error(oss);
   }

   // at least one of (r.a,r.b) must be positive (one can be zero)
   if (r.a == real(0) && r.b == real(0))
      rv = error("Bicylinder has both radii = 0 (at least one must be > 0");

   // require a != b
   if (a == b) {
      std::ostringstream oss;
      oss << "Bicylinder has coincident points a=b=" << a;
      rv = error(oss);
   }

   return rv;
} kip_end



// randomize
kip_randomize(bicylinder)
{
   // a
   random_full(obj.a);

   // b
   obj.b.x = obj.a.x + real(0.6)*random_half<real>();
   obj.b.y = obj.a.y + real(0.6)*random_half<real>();
   obj.b.z = obj.a.z + real(0.6)*random_half<real>();

   // r.a, r.b
   obj.r.a = real(0.15)*random_unit<real>();
   obj.r.b = real(0.15)*random_unit<real>();

   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(bicylinder)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (this->interior) {

      // x = 0
      if (dx < 0) {
         q = -rot.ex/dx;
         if (0 < q && q < qmin) {
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            if (op::square(q.y) + op::square(q.z) <= rasq) {
               q.x = real(0);
               return q(-1,0,0, this, normalized::yes), true;
            }
         }
      }

      // x = h
      if (dx > 0) {
         q = (rot.h-rot.ex)/dx;
         if (0 < q && q < qmin) {
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            if (op::square(q.y) + op::square(q.z) <= rbsq) {
               q.x = rot.h;
               return q(1,0,0, this, normalized::yes), true;
            }
         }
      }

      // curve
      const real f = 1 - dx*dx*h1, g = dx*h2 - dy*rot.ey, s = g*g - f*h3;
      if (s < 0 || f == 0) return false;

      q = (g + std::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;

      q.x = rot.ex + real(q)*dx;  // inside, so don't need the range check

   } else {

      // Outside...
      // see if we even hit the extended bicylinder
      const real f = 1 - dx*dx*h1, g = dx*h2 - dy*rot.ey, s = g*g - f*h3;
      if (s < 0) return false;

      // x = 0
      if (rot.ex < 0) {
         if (dx <= 0) return false;
         q = -rot.ex/dx;
         if (!(0 < q && q < qmin)) return false;

         q.y = rot.ey + q*dy;
         q.z = q*tar.z;

         if (op::square(q.y) + op::square(q.z) <= rasq) {
            q.x = real(0);
            return q(-1,0,0, this, normalized::yes), true;
         }
      }

      // x = h
      if (rot.ex > rot.h) {
         if (dx >= 0) return false;
         q = (rot.h-rot.ex)/dx;
         if (!(0 < q && q < qmin)) return false;

         q.y = rot.ey + q*dy;
         q.z = q*tar.z;

         if (op::square(q.y) + op::square(q.z) <= rbsq) {
            q.x = rot.h;
            return q(1,0,0, this, normalized::yes), true;
         }
      }

      // curve
      if (f == 0) return false;

      q = (g - std::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.x = rot.ex + q*dx;
      if (!(0 <= q.x && q.x <= rot.h)) return false;

   }

   q.y = rot.ey + q*dy;
   q.z = q*tar.z;

   return q(
      -slope*(r.a + slope*q.x), q.y, q.z, this, normalized::no
   ), true;
} kip_end



// -----------------------------------------------------------------------------
// get_base0
// get_baseh
// get_curve
// -----------------------------------------------------------------------------

// get_base0
template<class real, class tag>
inline bool bicylinder<real,tag>::get_base0(
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
   if (op::square(info.y) + op::square(info.z) <= rasq) {
      info.x = real(0);
      return info(-1,0,0, this, normalized::yes), true;
   }
   return false;
}



// get_baseh
template<class real, class tag>
inline bool bicylinder<real,tag>::get_baseh(
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
   if (op::square(info.y) + op::square(info.z) <= rbsq) {
      info.x = rot.h;
      return info(1,0,0, this, normalized::yes), true;
   }
   return false;
}



// get_curve
template<class real, class tag>
inline bool bicylinder<real,tag>::get_curve(
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

   return info(
      -slope*(r.a + slope*info.x), info.y, info.z, this, normalized::no
   ), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(bicylinder)
{
   if (this->interior) return bicylinder<real,tag>::infirst(
      kip_etd, qmin,ints.one(), insub);

   // Outside...
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if ((rot.ex < 0 && dx <= 0) || (rot.ex > rot.h && dx >= 0))
      return false;  // miss extended bicylinder, or look away

   ints.convex();
   inq<real,tag> q1, q2;

   if ((get_base0(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
       (get_baseh(tar,dx,dy,qmin,q2) && ints.convex(q2)))
      return true;

   const real f = 1 - dx*dx*h1, g = dx*h2 - dy*rot.ey, s = g*g - f*h3;
   if (s >= 0 && f != 0) {
      const real tmp = std::sqrt(s);

      inq<real,tag> q3((g - tmp)/f);
      if (get_curve(tar,dx,dy,qmin,q3) && ints.convex(q3))
         return true;

      if (!ints.size()) return false;

      inq<real,tag> q4((g + tmp)/f);
      if (get_curve(tar,dx,dy,qmin,q4) && ints.convex(q4))
         return true;
   }

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(bicylinder) {

   // (
   //    a.x, a.y, a.z,
   //    b.x, b.y, b.z,
   //    r.a,
   //    r.b,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.a, "\"a\" endpoint") && read_comma(s) &&
      read_value(s, obj.b, "\"b\" endpoint") && read_comma(s) &&
      read_value(s, obj.r.a, "\"a\" radius") && read_comma(s) &&
      read_value(s, obj.r.b, "\"b\" radius") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(bicylinder) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "bicylinder()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "bicylinder("
               << obj.a << ", "
               << obj.b << ", "
               << obj.r.a << ',' << obj.r.b &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << "bicylinder(\n   " &&
             k.indent() << obj.a << ",\n   " &&
             k.indent() << obj.b << ",\n   " &&
             k.indent() << obj.r.a << ',' << obj.r.b &&
             write_finish(k, obj, false);

   kip_ostream_end(bicylinder);
}

#define   kip_class bicylinder
#include "kip-macro-io.h"
