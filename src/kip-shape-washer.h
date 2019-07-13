
#pragma once

// -----------------------------------------------------------------------------
// washer
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class washer : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // modified washer: (0,0,0), (h,0,0), i, o
   mutable rotate<3,real> rot;
   mutable real isq, osq, hsq, hin, hout, p;

   inline bool first_in(
      const point<real> &tar, const real dx, const real dy, const real qmin, inq<real,tag> &q
   ) const;

   inline bool first_out(
      const point<real> &tar, const real dx, const real dy, const real qmin, inq<real,tag> &q
   ) const;

   // get_*
   inline bool
   get_base0(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_baseh(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_inner(const point<real> &, const real, const real, const real, inq<real,tag> &) const,
   get_outer(const point<real> &, const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(washer);

   // point a, point b, inner and outer radii
   point<real> a, b;
   real i, o;

   inline point<real> back(const point<real> &from) const { return rot.back(from); }



   // washer([a[,b[,i,o[,base]]]])
   inline explicit washer(
      const point<real> &_a = point<real>(real(0),real(0),real(0)),
      const point<real> &_b = point<real>(real(1),real(0),real(0))
   ) :
      shape<real,tag>(this),
      a(_a),
      b(_b),
      i(real(1)), o(real(2))
   { }

   inline explicit washer(
      const point<real> &_a,
      const point<real> &_b,
      const real _i,
      const real _o
   ) :
      shape<real,tag>(this),
      a(_a),
      b(_b),
      i(_i), o(_o)
   { }

   inline explicit washer(
      const point<real> &_a,
      const point<real> &_b,
      const real _i,
      const real _o, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      a(_a),
      b(_b),
      i(_i), o(_o)
   { }



   // washer(a.x,a.y,a.z[,b.x,b.y,b.z[,i,o[,base]]])
   inline explicit washer(
      const real ax, const real ay, const real az
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(real(1),real(0),real(0)),
      i(real(1)), o(real(2))
   { }

   inline explicit washer(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(bx,by,bz),
      i(real(1)), o(real(2))
   { }

   inline explicit washer(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz,
      const real _i, const real _o
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(bx,by,bz),
      i(_i), o(_o)
   { }

   inline explicit washer(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz,
      const real _i, const real _o, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      a(ax,ay,az),
      b(bx,by,bz),
      i(_i), o(_o)
   { }



   // washer(washer)
   inline washer(const washer &from) :
      shape<real,tag>(from),
      a(from.a),
      b(from.b),
      i(from.i),
      o(from.o)
   { }

   // washer = washer
   inline washer &operator=(const washer &from)
   {
      this->shape<real,tag>::operator=(from);
      a = from.a;  i = from.i;
      b = from.b;  o = from.o;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(washer)
{
   rot = mod2(a-eyeball) < mod2(b-eyeball)
      ? rotate<3,real>(a, b, eyeball)
      : rotate<3,real>(b, a, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   isq = i*i;  hin  = isq - rot.ey*rot.ey;  hsq = rot.h*rot.h;
   osq = o*o;  hout = osq - rot.ey*rot.ey;  p   = osq/hsq;

   // interior
   interior = inside(eyeball);

   // minimum:
   //
   //       y
   //
   //       |     |
   //       |
   //   nw  |  n  |  ne
   //       |
   // - - - +-----+ - - - o
   //       |*****|
   //       |*****|
   //    w  |*****|   e
   //       |*****|
   //       |*****|
   // - - - +-----+ - - - i
   //       |
   //       |     |
   //   sw  |  s     se
   //       |     |
   //       |
   // ------0-----+------ x
   //       |     h
   //       |
   //
   // eyeball = (ex,ey)

   // note: "east" variants no longer needed, due to transformation
   return
       rot.ey > o
    ?  rot.ex < 0
    ?  std::sqrt(op::square(rot.ex) + op::square(rot.ey-o))  // nw
    :  rot.ey - o  // n
    :  rot.ey >= i
    ?  rot.ex <= 0
    ? -rot.ex  // w
    :  op::min(rot.ex, o-rot.ey, rot.ey-i)  // inside
    :  rot.ex < 0
    ?  std::sqrt(op::square(rot.ex) + op::square(rot.ey-i))  // sw
    :  i - rot.ey  // s
    ;
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(washer)
{
   // same as for cylinder - cutout doesn't change bounds
   return internal::bound_cylinder(*this, this->o);
} kip_end



// inside
kip_inside(washer)
{
   // ex <= h test no longer necessary, due to transformation
   return 0 <= rot.ex && hin <= 0 && hout >= 0;
} kip_end



// dry
kip_dry(washer)
{
   real az;  if (seg.lt(a,az)) return false;
   real bz;  if (seg.lt(b,bz)) return false;
   return op::square(op::min(az,bz)-seg.c) >= p*(rot.h-az+bz)*(rot.h+az-bz);
} kip_end



// -----------------------------------------------------------------------------
// first_in
// -----------------------------------------------------------------------------

template<class real, class tag>
inline bool washer<real,tag>::first_in(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   // curves...
   const real c = dy*dy, d = tar.z*tar.z;
   if (c+d == 0) return false;  // 0 iff line-of-sight is parallel to x axis

   // inner
   if (dy < 0) {
      const real idisc = c*isq + d*hin;
      if (idisc >= 0) {
         q = -(std::sqrt(idisc) + rot.ey*dy)/(c+d);
         if (0 < q && q < qmin) {
            q.x = rot.ex + q*dx;
            if (0 <= q.x && q.x <= rot.h) {
               q.y = rot.ey + q*dy;
               q.z = q*tar.z;
               return q(0, -q.y, -q.z, this, normalized_t::nonorm), true;
            }
         }
      }
   }

   // outer
   const real odisc = c*osq + d*hout;
   if (odisc >= 0) {
      q = (std::sqrt(odisc) - rot.ey*dy)/(c+d);
      if (0 < q && q < qmin) {
         q.x = rot.ex + q*dx;
         if (0 <= q.x && q.x <= rot.h) {
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            return q(0, q.y, q.z, this, normalized_t::nonorm), true;
         }
      }
   }

   // x = 0
   if (dx < 0) {
      q = -rot.ex/dx;
      if (0 < q && q < qmin) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;

         const real tmp = op::square(q.y) + op::square(q.z);
         if (isq <= tmp && tmp <= osq) {
            q.x = real(0);
            return q(-1,0,0, this, normalized_t::yesnorm), true;
         }
      }
   }

   // x = h
   if (dx > 0) {
      q = (rot.h-rot.ex)/dx;
      if (0 < q && q < qmin) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;

         const real tmp = op::square(q.y) + op::square(q.z);
         if (isq <= tmp && tmp <= osq) {
            q.x = rot.h;
            return q(1,0,0, this, normalized_t::yesnorm), true;
         }
      }
   }

   return false;
}



// -----------------------------------------------------------------------------
// first_out
// -----------------------------------------------------------------------------

template<class real, class tag>
inline bool washer<real,tag>::first_out(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   // see if we even hit the extended washer
   const real c = dy*dy, d = tar.z*tar.z, odisc = c*osq + d*hout;
   if (odisc < 0) return false;

   // x = 0
   if (rot.ex < 0) {
      if (dx <= 0) return false;

      q = -rot.ex/dx;
      if (!(0 < q && q < qmin)) return false;

      q.y = rot.ey + q*dy;
      q.z = q*tar.z;

      const real tmp = op::square(q.y) + op::square(q.z);
      if (isq <= tmp && tmp <= osq) {
         q.x = real(0);
         return q(-1,0,0, this, normalized_t::yesnorm), true;
      }
   }

   // curves...
   if (c+d == 0) return false;  // 0 iff line-of-sight is parallel to x axis

   // outer
   if (rot.ey > o) {  // oh (outer radius), not zero
      if (dy >= 0) return false;

      q = -(rot.ey*dy + std::sqrt(odisc))/(c+d);
      if (!(0 < q && q < qmin)) return false;

      q.x = rot.ex + q*dx;
      if (0 <= q.x && q.x <= rot.h) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;
         return q(0, q.y, q.z, this, normalized_t::nonorm), true;
      }
   }

   // inner
   if (rot.ex < 0) {
      const real idisc = c*isq + d*hin;
      if (idisc < 0) return false;

      q = (std::sqrt(idisc) - rot.ey*dy)/(c+d);
      if (!(0 < q && q < qmin)) return false;

      q.x = rot.ex + q*dx;
      if (0 <= q.x && q.x <= rot.h) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;
         return q(0, -q.y, -q.z, this, normalized_t::nonorm), true;
      }
   }

   return false;
}



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(washer)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   return interior ? first_in(tar,dx,dy,qmin,q) : first_out(tar,dx,dy,qmin,q);
} kip_end



// -----------------------------------------------------------------------------
// get_base0
// get_baseh
// get_inner
// get_outer
// -----------------------------------------------------------------------------

// get_base0
template<class real, class tag>
inline bool washer<real,tag>::get_base0(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   q = -rot.ex/dx;
   if (0 < q && q < qmin) {
      q.y = rot.ey + q*dy;
      q.z = q*tar.z;

      const real tmp = op::square(q.y) + op::square(q.z);
      if (isq <= tmp && tmp <= osq) {
         q.x = real(0);
         return q(-1,0,0, this, normalized_t::yesnorm), true;
      }
   }
   return false;
}



// get_baseh
template<class real, class tag>
inline bool washer<real,tag>::get_baseh(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   q = (rot.h-rot.ex)/dx;
   if (0 < q && q < qmin) {
      q.y = rot.ey + q*dy;
      q.z = q*tar.z;

      const real tmp = op::square(q.y) + op::square(q.z);
      if (isq <= tmp && tmp <= osq) {
         q.x = rot.h;
         return q(1,0,0, this, normalized_t::yesnorm), true;
      }
   }
   return false;
}



// get_inner
template<class real, class tag>
inline bool washer<real,tag>::get_inner(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   if (0 < q && q < qmin) {
      q.x = rot.ex + q*dx;
      if (0 <= q.x && q.x <= rot.h) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;
         return q(0, -q.y, -q.z, this, normalized_t::nonorm), true;
      }
   }
   return false;
}



// get_outer
template<class real, class tag>
inline bool washer<real,tag>::get_outer(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   if (0 < q && q < qmin) {
      q.x = rot.ex + q*dx;
      if (0 <= q.x && q.x <= rot.h) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;
         return q(0, q.y, q.z, this, normalized_t::nonorm), true;
      }
   }
   return false;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(washer)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;  if (rot.ex < 0 && dx <= 0) return false;
   const real dy = tar.y - rot.ey;  if (rot.ey > o && dy >= 0) return false;

   const real c = dy*dy, d = tar.z*tar.z, odisc = c*osq + d*hout;
   if (odisc < 0) return false;

   ints.four();
   inq<real,tag> qtmp;

   // ends
   if (get_base0(tar,dx,dy,qmin,qtmp)) ints.four(qtmp);
   if (get_baseh(tar,dx,dy,qmin,qtmp)) ints.four(qtmp);

   // curves...
   if (c+d != 0) {
      const real f = rot.ey*dy, div = 1/(c+d);

      // outer
      const real so = std::sqrt(odisc);
      if (!((get_outer(tar,dx,dy,qmin,qtmp =-(so+f)*div) && ints.four(qtmp)) ||
            (get_outer(tar,dx,dy,qmin,qtmp = (so-f)*div) && ints.four(qtmp)))) {

         // inner
         const real idisc = c*isq + d*hin;
         if (idisc >= 0) {
            const real si = std::sqrt(idisc);
            (get_inner(tar,dx,dy,qmin,qtmp =-(si+f)*div) && ints.four(qtmp)) ||
            (get_inner(tar,dx,dy,qmin,qtmp = (si-f)*div) && ints.four(qtmp));
         }
      }
   }

   return ints.sort();
} kip_end



// -----------------------------------------------------------------------------
// check
// -----------------------------------------------------------------------------

kip_check(washer)
{
   diagnostic_t rv = diagnostic_t::diagnostic_good;

   // i
   // I think we're allowing == 0, just not < 0
   if (i < real(0)) {
      std::ostringstream oss;
      oss << "Washer has negative inner radius i=" << i;
      rv = error(oss);
   }

   // o
   if (o <= real(0)) {
      std::ostringstream oss;
      oss << "Washer has non-positive outer radius o=" << o;
      rv = error(oss);
   }

   // require i < o
   if (i >= o) {
      std::ostringstream oss;
      oss << "Washer has inner radius i=" << i
          << " >= outer radius o=" << o;
      rv = error(oss);
   }

   // require a != b
   if (a == b) {
      std::ostringstream oss;
      oss << "Washer has coincident points a=b=" << a;
      rv = error(oss);
   }

   return rv;
} kip_end



// -----------------------------------------------------------------------------
// random
// -----------------------------------------------------------------------------

kip_random(washer)
{
   // a
   random_full(obj.a);

   // b
   obj.b.x = obj.a.x + real(0.6)*random_half<real>();
   obj.b.y = obj.a.y + real(0.6)*random_half<real>();
   obj.b.z = obj.a.z + real(0.6)*random_half<real>();

   // i, o
   obj.o = real(0.15)*random_unit<real>();
   obj.i = obj.o*random_unit<real>();

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(washer) {

   // (
   //    a.x, a.y, a.z,
   //    b.x, b.y, b.z,
   //    i,
   //    o,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.a, "endpoint #1") && read_comma(s) &&
      read_value(s, obj.b, "endpoint #2") && read_comma(s) &&
      read_value(s, obj.i, "inner radius") && read_comma(s) &&
      read_value(s, obj.o, "outer radius") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(washer) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "washer()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "washer("
               << obj.a << ", "
               << obj.b << ", "
               << obj.i << ',' << obj.o &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << "washer(\n   " &&
             k.indent() << obj.a << ",\n   " &&
             k.indent() << obj.b << ",\n   " &&
             k.indent() << obj.i << ',' << obj.o &&
             write_finish(k, obj, false);

   kip_ostream_end(washer);
}

#define   kip_class washer
#include "kip-macro-io.h"
