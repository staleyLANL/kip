
#pragma once

// -----------------------------------------------------------------------------
// biwasher
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class biwasher : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // modified biwasher: (0,0,0), (h,0,0), i.a,i.b, o.a,o.b
   mutable rotate<3,real,op::full,op::unscaled> rot;
   mutable real iasq, oasq, islope, itmp1, itmp2, itmp3;
   mutable real ibsq, obsq, oslope, otmp1, otmp2, otmp3;

   // first_in/first_out: called by infirst() when inside/outside
   inline bool
      first_in(
         const point<real> &,
         const real, const real, const real,
         inq<real,tag> &
      ) const,
      first_out(
         const point<real> &,
         const real, const real, const real,
         inq<real,tag> &
      ) const;

   // get_*
   inline bool
   get_base0(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const,
   get_baseh(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const,
   get_inner(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const,
   get_outer(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(biwasher);

   // point a, point b, inner and outer a and b radii
   point<real> a, b;
   class { public: real a, b; } i;
   class { public: real a, b; } o;

   inline point<real> back(const point<real> &from) const
      { return rot.back(from); }



   // biwasher([a[,b[,i.a,i.b,o.a,o.b[,base]]]])
   inline explicit biwasher(
      const point<real> &_a = point<real>(real(0),real(0),real(0)),
      const point<real> &_b = point<real>(real(1),real(0),real(0))
   ) :
      shape<real,tag>(this),
      a(_a), b(_b)
   {
      i.a = real(1);  i.b = real(2);
      o.a = real(2);  o.b = real(3);
   }

   inline explicit biwasher(
      const point<real> &_a,
      const point<real> &_b,
      const real _ia, const real _ib,
      const real _oa, const real _ob
   ) :
      shape<real,tag>(this),
      a(_a), b(_b)
   {
      i.a = _ia;  i.b = _ib;
      o.a = _oa;  o.b = _ob;
   }

   inline explicit biwasher(
      const point<real> &_a,
      const point<real> &_b,
      const real _ia, const real _ib,
      const real _oa, const real _ob, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      a(_a), b(_b)
   {
      i.a = _ia;  i.b = _ib;
      o.a = _oa;  o.b = _ob;
   }



   // biwasher(a.x,a.y,a.z[,b.x,b.y,b.z[,i.a,i.b,o.a,o.b[,base]]])
   inline explicit biwasher(
      const real ax, const real ay, const real az
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(real(1),real(0),real(0))
   {
      i.a = real(1);  i.b = real(2);
      o.a = real(2);  o.b = real(3);
   }

   inline explicit biwasher(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(bx,by,bz)
   {
      i.a = real(1);  i.b = real(2);
      o.a = real(2);  o.b = real(3);
   }

   inline explicit biwasher(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz,
      const real _ia, const real _ib,
      const real _oa, const real _ob
   ) :
      shape<real,tag>(this),
      a(ax,ay,az),
      b(bx,by,bz)
   {
      i.a = _ia;  i.b = _ib;
      o.a = _oa;  o.b = _ob;
   }

   inline explicit biwasher(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz,
      const real _ia, const real _ib,
      const real _oa, const real _ob, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      a(ax,ay,az),
      b(bx,by,bz)
   {
      i.a = _ia;  i.b = _ib;
      o.a = _oa;  o.b = _ob;
   }



   // biwasher(biwasher)
   inline biwasher(const biwasher &from) :
      shape<real,tag>(from),
      a(from.a),
      b(from.b)
   {
      i.a = from.i.a;  i.b = from.i.b;
      o.a = from.o.a;  o.b = from.o.b;
   }

   // biwasher = biwasher
   inline biwasher &operator=(const biwasher &from)
   {
      this->shape<real,tag>::operator=(from);
      a = from.a;  i.a = from.i.a;  i.b = from.i.b;
      b = from.b;  o.a = from.o.a;  o.b = from.o.b;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(biwasher)
{
   rot = rotate<3,real,op::full,op::unscaled>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   iasq = i.a*i.a;  oasq = o.a*o.a;  islope = (i.b-i.a)/rot.h;
   ibsq = i.b*i.b;  obsq = o.b*o.b;  oslope = (o.b-o.a)/rot.h;

   itmp1 = 1 + islope*islope;  itmp2 = islope*(i.a + islope*rot.ex);
   otmp1 = 1 + oslope*oslope;  otmp2 = oslope*(o.a + oslope*rot.ex);

   itmp3 = rot.ey*rot.ey - op::square(i.a + islope*rot.ex);
   otmp3 = rot.ey*rot.ey - op::square(o.a + oslope*rot.ex);

   interior = inside(eyeball);

   // minimum...
   const real x = rot.ex;
   const real y = rot.ey, h = rot.h;

   // west
   if (x < 0 && i.a <= y && y <= o.a) return -x;

   // east
   if (x > h && i.b <= y && y <= o.b) return  x-h;

   if ((x < 0 && y > o.a) || (x > h && y > o.b) ||
       (x >= 0 && x <= h && y > o.a + oslope*x)) {

      // northwest: cross((o.a-o.b,h), (x,y-o.a)) >= 0
      if ((o.a-o.b)*(y-o.a) - h*x >= 0)
         return std::sqrt(op::square(x) + op::square(y-o.a));

      // northeast: cross((o.a-o.b,h), (x-h,y-o.b)) <= 0
      if ((o.a-o.b)*(y-o.b) - h*(x-h) <= 0)
         return std::sqrt(op::square(x-h) + op::square(y-o.b));

      // north
      return std::abs(oslope*x+o.a-y)/std::sqrt(otmp1);
   }

   if ((x < 0 && y < i.a) || (x > h && y < i.b) ||
       (x >= 0 && x <= h && y < i.a + islope*x)) {

      // southwest: cross((i.b-i.a,-h), (x,y-i.a)) <= 0
      if ((i.b-i.a)*(y-i.a) + h*x <= 0)
         return std::sqrt(op::square(x) + op::square(y-i.a));

      // southeast: cross((i.b-i.a,-h), (x-h,y-i.b)) >= 0
      if ((i.b-i.a)*(y-i.b) + h*(x-h) >= 0)
         return std::sqrt(op::square(x-h) + op::square(y-i.b));

      // south
      return std::abs(islope*x+i.a-y)/std::sqrt(itmp1);
   }

   // inside
   return op::min(
      x, h-x,
      std::abs(oslope*x+o.a-y)/std::sqrt(otmp1),
      std::abs(islope*x+i.a-y)/std::sqrt(itmp1)
   );
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(biwasher)
{
   // same as for bicylinder - cutout doesn't change bounds
   return detail::bound_bicylinder(*this, this->o.a, this->o.b);
} kip_end



// inside
kip_inside(biwasher)
{
   return
      0 <= rot.ex && rot.ex <= rot.h
   && rot.ey <= o.a + oslope*rot.ex  // false more often
   && rot.ey >= i.a + islope*rot.ex  // false less often
   ;
} kip_end



// dry
kip_dry(biwasher)
{
   real az;  if (seg.lt(a,az)) return false;
   real bz;  if (seg.lt(b,bz)) return false;

   const real tmp = (rot.h-az+bz)*(rot.h+az-bz);
   return
      op::square((az-seg.c)*rot.h) >= oasq*tmp &&
      op::square((bz-seg.c)*rot.h) >= obsq*tmp;
} kip_end



// check
kip_check(biwasher)
{
   diagnostic rv = diagnostic::good;

   // require 0 <= i.a <= o.a
   if (!(0 <= i.a && i.a <= o.a)) {
      std::ostringstream oss;
      oss << "biwasher requires 0 <= i.a=" << i.a << " <= o.a=" << o.a;
      rv = error(oss);
   }

   // require 0 <= i.b <= o.b
   if (!(0 <= i.b && i.b <= o.b)) {
      std::ostringstream oss;
      oss << "biwasher requires 0 <= i.b=" << i.b << " <= o.b=" << o.b;
      rv = error(oss);
   }

   // require a != b
   if (a == b) {
      std::ostringstream oss;
      oss << "biwasher has coincident points a=b=" << a;
      rv = error(oss);
   }

   return rv;
} kip_end



// randomize
kip_randomize(biwasher)
{
   const real oa = real(0.15)*random_unit<real>();
   const real ob = real(0.15)*random_unit<real>();

   // a
   random_full(obj.a);

   // b
   obj.b.x = obj.a.x + real(0.6)*random_half<real>();
   obj.b.y = obj.a.y + real(0.6)*random_half<real>();
   obj.b.z = obj.a.z + real(0.6)*random_half<real>();

   // i.a, o.a
   // i.b, o.b
   obj.i.a = oa*random_unit<real>();  obj.o.a = oa;
   obj.i.b = ob*random_unit<real>();  obj.o.b = ob;

   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// first_in
// -----------------------------------------------------------------------------

template<class real, class tag>
inline bool biwasher<real,tag>::first_in(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   // x = 0
   if (dx < 0) {
      q = -rot.ex/dx;

      if (0 < q && q < qmin) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;

         const real tmp = op::square(q.y) + op::square(q.z);
         if (iasq <= tmp && tmp <= oasq) {
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

         const real tmp = op::square(q.y) + op::square(q.z);
         if (ibsq <= tmp && tmp <= obsq) {
            q.x = rot.h;
            return q(1,0,0, this, normalized::yes), true;
         }
      }
   }

   // curves...
   const real ao = 1 - dx*dx*otmp1;
   const real bo = dx*otmp2 - dy*rot.ey;
   const real so = bo*bo - ao*otmp3;
   if (so < 0) return false;  // probably impossible, given that we're inside

   // outer
   if (ao != 0) {
      q = (bo + std::sqrt(so))/ao;
      if (0 < q && q < qmin) {
         q.x = rot.ex + q*dx;
         if (0 <= q.x && q.x <= rot.h) {
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            return q(-oslope*(o.a + oslope*q.x), q.y, q.z,
                     this, normalized::no), true;
         }
      }
   }

   // inner
   const real ai = 1 - dx*dx*itmp1;
   const real bi = dx*itmp2 - dy*rot.ey;
   const real si = bi*bi - ai*itmp3;
   if (si < 0) return false;

   if (ai != 0) {
      q = (bi + std::sqrt(si))/ai;
      if (0 < q && q < qmin) {
         q.x = rot.ex + q*dx;
         if (0 <= q.x && q.x <= rot.h) {
            q.y = rot.ey + q*dy;
            q.z = q*tar.z;
            return q(islope*(i.a + islope*q.x), -q.y, -q.z,
                     this, normalized::no), true;
         }
      }
   }

   return false;
}



// -----------------------------------------------------------------------------
// first_out
// -----------------------------------------------------------------------------

template<class real, class tag>
inline bool biwasher<real,tag>::first_out(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   // x = 0
   if (rot.ex < 0) {
      if (dx <= 0) return false;

      q = -rot.ex/dx;
      if (!(0 < q && q < qmin)) return false;

      q.y = rot.ey + q*dy;
      q.z = q*tar.z;

      const real tmp = op::square(q.y) + op::square(q.z);
      if (iasq <= tmp && tmp <= oasq) {
         q.x = real(0);
         return q(-1,0,0, this, normalized::yes), true;
      }

   // x = h
   } else if (rot.ex > rot.h) {
      if (dx >= 0) return false;

      q = (rot.h-rot.ex)/dx;
      if (!(0 < q && q < qmin)) return false;

      q.y = rot.ey + q*dy;
      q.z = q*tar.z;

      const real tmp = op::square(q.y) + op::square(q.z);
      if (ibsq <= tmp && tmp <= obsq) {
         q.x = rot.h;
         return q(1,0,0, this, normalized::yes), true;
      }
   }

   // curves...
   const real ao = 1 - dx*dx*otmp1;
   const real bo = dx*otmp2 - dy*rot.ey;
   const real so = bo*bo - ao*otmp3;
   if (so < 0) return false;

   // outer
   if (ao != 0 && rot.ey > o.a + oslope*rot.ex) {
      q = (bo - std::sqrt(so))/ao;
      if (!(0 < q && q < qmin)) return false;

      q.x = rot.ex + q*dx;
      if (0 <= q.x && q.x <= rot.h) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;
         return q(-oslope*(o.a + oslope*q.x), q.y, q.z,
                  this, normalized::no), true;
      }
   }

   // inner
   const real ai = 1 - dx*dx*itmp1;
   const real bi = dx*itmp2 - dy*rot.ey;
   const real si = bi*bi - ai*itmp3;
   if (si < 0 || ai == 0) return false;

   q = (bi + std::sqrt(si))/ai;
   if (!(0 < q && q < qmin)) return false;

   q.x = rot.ex + q*dx;
   if (0 <= q.x && q.x <= rot.h) {
      q.y = rot.ey + q*dy;
      q.z = q*tar.z;
      return q(islope*(i.a + islope*q.x), -q.y, -q.z,
               this, normalized::no), true;
   }

   return false;
}



// -----------------------------------------------------------------------------
// get_base0
// get_baseh
// get_inner
// get_outer
// -----------------------------------------------------------------------------

// get_base0
template<class real, class tag>
inline bool biwasher<real,tag>::get_base0(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   q = -rot.ex/dx;

   if (0 < q && q < qmin) {
      q.y = rot.ey + q*dy;
      q.z = q*tar.z;

      const real tmp = op::square(q.y) + op::square(q.z);
      if (iasq <= tmp && tmp <= oasq) {
         q.x = real(0);
         return q(-1,0,0, this, normalized::yes), true;
      }
   }
   return false;
}



// get_baseh
template<class real, class tag>
inline bool biwasher<real,tag>::get_baseh(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   q = (rot.h-rot.ex)/dx;

   if (0 < q && q < qmin) {
      q.y = rot.ey + q*dy;
      q.z = q*tar.z;

      const real tmp = op::square(q.y) + op::square(q.z);
      if (ibsq <= tmp && tmp <= obsq) {
         q.x = rot.h;
         return q(1,0,0, this, normalized::yes), true;
      }
   }
   return false;
}



// get_inner
template<class real, class tag>
inline bool biwasher<real,tag>::get_inner(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   if (0 < q && q < qmin) {
      q.x = rot.ex + q*dx;
      if (0 <= q.x && q.x <= rot.h) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;
         return q(islope*(i.a + islope*q.x), -q.y, -q.z,
                  this, normalized::no), true;
      }
   }
   return false;
}



// get_outer
template<class real, class tag>
inline bool biwasher<real,tag>::get_outer(
   const point<real> &tar, const real dx, const real dy, const real qmin,
   inq<real,tag> &q
) const {
   if (0 < q && q < qmin) {
      q.x = rot.ex + q*dx;
      if (0 <= q.x && q.x <= rot.h) {
         q.y = rot.ey + q*dy;
         q.z = q*tar.z;
         return q(-oslope*(o.a + oslope*q.x), q.y, q.z,
                  this, normalized::no), true;
      }
   }
   return false;
}



// -----------------------------------------------------------------------------
// infirst
// inall
// -----------------------------------------------------------------------------

// infirst
kip_infirst(biwasher)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   return interior ? first_in(tar,dx,dy,qmin,q) : first_out(tar,dx,dy,qmin,q);
} kip_end



// inall
kip_inall(biwasher)
{
   // setup
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   if ((rot.ex < 0 && dx <= 0) || (rot.ex > rot.h && dx >= 0)) return false;
   const real dy = tar.y - rot.ey;

   // return early if no possible hit
   const real ao = 1 - dx*dx*otmp1;
   const real bo = dx*otmp2 - dy*rot.ey;
   const real so = bo*bo - ao*otmp3;
   if (so < 0) return false;

   // prepare for finding intersections
   ints.four();
   inq<real,tag> qtmp;

   // x = 0
   if (get_base0(tar,dx,dy,qmin,qtmp)) ints.four(qtmp);

   // x = h
   if (get_baseh(tar,dx,dy,qmin,qtmp)) ints.four(qtmp);

   // outer
   if (ao != 0) {
      const real s1 = std::sqrt(so), divo = 1/ao;
      if (!((get_outer(tar,dx,dy,qmin,qtmp=(bo+s1)*divo) && ints.four(qtmp)) ||
            (get_outer(tar,dx,dy,qmin,qtmp=(bo-s1)*divo) && ints.four(qtmp)))) {

         // inner
         const real ai = 1 - dx*dx*itmp1;
         const real bi = dx*itmp2 - dy*rot.ey;
         const real si = bi*bi - ai*itmp3;

         if (si >= 0 && ai != 0) {
            const real s2 = std::sqrt(si), divi = 1/ai;
            (get_inner(tar,dx,dy,qmin,qtmp=(bi+s2)*divi) && ints.four(qtmp)) ||
            (get_inner(tar,dx,dy,qmin,qtmp=(bi-s2)*divi) && ints.four(qtmp));
         }
      }
   }

   return ints.sort();
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(biwasher) {

   // (
   //    a.x, a.y, a.z,
   //    b.x, b.y, b.z,
   //    i.a, i.b,
   //    o.a, o.b,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.a, "\"a\" endpoint") && read_comma(s) &&
      read_value(s, obj.b, "\"b\" endpoint") && read_comma(s) &&
      read_value(s, obj.i.a, "inner radius at \"a\"") && read_comma(s) &&
      read_value(s, obj.i.b, "inner radius at \"b\"") && read_comma(s) &&
      read_value(s, obj.o.a, "outer radius at \"a\"") && read_comma(s) &&
      read_value(s, obj.o.b, "outer radius at \"b\"") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(biwasher) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "biwasher()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "biwasher("
               << obj.a << ", "
               << obj.b << ", "
               << obj.i.a << ',' << obj.i.b << ", "
               << obj.o.a << ',' << obj.o.b &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << "biwasher(\n   " &&
             k.indent() << obj.a << ",\n   " &&
             k.indent() << obj.b << ",\n   " &&
             k.indent() << obj.i.a << ',' << obj.i.b << ",\n   " &&
             k.indent() << obj.o.a << ',' << obj.o.b &&
             write_finish(k, obj, false);

   kip_ostream_end(biwasher);
}

#define   kip_class biwasher
#include "kip-macro-io.h"
