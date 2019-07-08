
#pragma once

// -----------------------------------------------------------------------------
// ellipsoid
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class ellipsoid : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // rotation: clockwise around x, then y, then z; then +c translation
   mutable rotate<3,real> rot;
   mutable point<real> irsq;
   mutable real bs, is, i;

   // get_curve
   inline bool get_curve(const real,const real,const real,const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(ellipsoid);

   // center, angles, radii
   point<real> c;
   point<real> a;
   point<real> r;

   inline point<real> back(const point<real> &from) const { return rot.back(from); }



   // ellipsoid([c[,a[,r[,base]]]])
   inline explicit ellipsoid(
      const point<real> &_c = point<real>(real(0),real(0),real(0)),
      const point<real> &_a = point<real>(real(0),real(0),real(0)),
      const point<real> &_r = point<real>(real(1),real(1),real(1))
   ) :
      shape<real,tag>(this),
      c(_c), a(_a), r(_r)
   { }

   inline explicit ellipsoid(
      const point<real> &_c,
      const point<real> &_a,
      const point<real> &_r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(_c), a(_a), r(_r)
   { }



   // ellipsoid(c.x,c.y,c.z[,a.x,a.y,a.z[,r.x,r.y,r.z[,base]]])
   inline explicit ellipsoid(
      const real &cx, const real &cy, const real &cz
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(real(0),real(0),real(0)),
      r(real(1),real(1),real(1))
   { }

   inline explicit ellipsoid(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(real(1),real(1),real(1))
   { }

   inline explicit ellipsoid(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az,
      const real &rx, const real &ry, const real &rz
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(rx,ry,rz)
   { }

   inline explicit ellipsoid(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az,
      const real &rx, const real &ry, const real &rz, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(rx,ry,rz)
   { }



   // ellipsoid(ellipsoid)
   inline ellipsoid(const ellipsoid &from) :
      shape<real,tag>(from),
      c(from.c),
      a(from.a),
      r(from.r)
   { }

   // ellipsoid = ellipsoid
   inline ellipsoid &operator=(const ellipsoid &from)
   {
      this->shape<real,tag>::operator=(from);
      c = from.c;
      a = from.a;
      r = from.r;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(ellipsoid)
{
   // rotation to move ellipsoid to its actual location
   const rotate<3,real> move(a.x, a.y, a.z, c);

   // rot, eye, lie
   rot = rotate<3,real>(
      c,  // == move.back(0,0,0)
      move.back_n00(r.x),
      move.back_0n0(r.y)
   );
   basic.eye() = rot.fore(eyeball);
   basic.lie() = point<float>(rot.fore(light));

   const point<real> &eye = basic.eye();

   // irsq
   irsq(1/r.x/r.x, 1/r.y/r.y, 1/r.z/r.z);

   // bs: radius of bounding sphere
   bs = op::max(r.x, r.y, r.z);

   // is: radius of inner sphere
   is = op::min(r.x, r.y, r.z);

   // i
   i = eye.x*eye.x*irsq.x + eye.y*eye.y*irsq.y + eye.z*eye.z*irsq.z - real(1);

   // The minimum estimate below can be improved (generally made larger),
   // but as with spheroid, doing so involves somewhat more work, which
   // we won't bother with at the moment.

   // eyeball's distance to 0
   const real d0 = mod(point<real>(eye));

   // interior
   interior = inside(eyeball);

   // minimum
   if (eye.x >  r.x || eye.y >  r.y || eye.z >  r.z ||
       eye.x < -r.x || eye.y < -r.y || eye.z < -r.z) {
      // OUTSIDE BOUNDING BOX (implies outside the object itself)

      // distance to bounding box
      // use eye+r, not eye, because bbox_minimum() is designed for objects
      // from 0 to "+diameter".
      const real bb = internal::bbox_minimum(point<real>(eye)+r, 2*r.x, 2*r.y, 2*r.z);

      return d0 <= bs
         ? bb                 // INSIDE  bounding sphere
         : op::max(bb,d0-bs)  // OUTSIDE bounding sphere
         ;

   } else {
      // INSIDE BOUNDING BOX

      if (d0 <= bs) {
         // INSIDE BOUNDING SPHERE
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

// bound_ellipsoid - helper
namespace internal {
   template<class real>
   inline real bound_ellipsoid(
      const real &a, const real &b, const real &c,
      const point<real> &r
   ) {
      return op::sqrt(op::sq(a*r.x) + op::sq(b*r.y) + op::sq(c*r.z));
   }
}

// aabb
kip_aabb(ellipsoid)
{
   rot = rotate<3,real>(a.x, a.y, a.z, c);

   const real xval = internal::bound_ellipsoid(rot.f1.x, rot.f2.x, rot.f3.x, r);
   const real yval = internal::bound_ellipsoid(rot.f1.y, rot.f2.y, rot.f3.y, r);
   const real zval = internal::bound_ellipsoid(rot.f1.z, rot.f2.z, rot.f3.z, r);

   return bbox<real>(
      true, c.x-xval,   c.x+xval, true,
      true, c.y-yval,   c.y+yval, true,
      true, c.z-zval,   c.z+zval, true
   );
} kip_end



// inside
kip_inside(ellipsoid)
{
   return i <= 0;
} kip_end



// dry
kip_dry(ellipsoid)
{
   const real cz = seg.forez(c);
   return
      cz >= is &&
     (cz >= bs ||
      cz*cz >=
         op::sq( dot(seg.f, point<real>(rot.f1.x, rot.f1.y, rot.f1.z)) * r.x) +
         op::sq( dot(seg.f, point<real>(rot.f2.x, rot.f2.y, rot.f2.z)) * r.y) +
         op::sq( dot(seg.f, point<real>(rot.f3.x, rot.f3.y, rot.f3.z)) * r.z)
     );
} kip_end



// check
kip_check(ellipsoid)
{
   static const char *const err = "Ellipsoid has non-positive radius r.";
   using ostr_t = std::ostringstream;
   diagnostic_t rv = diagnostic_t::diagnostic_good;

   if (r.x <= real(0)) { ostr_t oss; oss << err << "x=" << r.x; rv=error(oss); }
   if (r.y <= real(0)) { ostr_t oss; oss << err << "y=" << r.y; rv=error(oss); }
   if (r.z <= real(0)) { ostr_t oss; oss << err << "z=" << r.z; rv=error(oss); }

   return rv;
} kip_end



// random
kip_random(ellipsoid)
{
   // center
   obj.c.x = op::twice(random<real>() - real(0.5));
   obj.c.y = op::twice(random<real>() - real(0.5));
   obj.c.z = op::twice(random<real>() - real(0.5));

   // angles
   obj.a.x = real(kip_pi)*random<real>();
   obj.a.y = real(kip_pi)*random<real>();
   obj.a.z = real(kip_pi)*random<real>();

   // radii
   obj.r.x = real(0.15)*random<real>();
   obj.r.y = real(0.15)*random<real>();
   obj.r.z = real(0.15)*random<real>();

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(ellipsoid)
{
   const point<real>  tar = rot.fore(target);
   const point<real> &eye = basic.eye();

   const real dx = eye.x - tar.x, cix = dx*irsq.x;
   const real dy = eye.y - tar.y, ciy = dy*irsq.y;
   const real dz = eye.z - tar.z, ciz = dz*irsq.z;

   const real g = cix*dx + ciy*dy + ciz*dz;
   const real p = cix*eye.x + ciy*eye.y + ciz*eye.z;
   const real s = p*p - g*i;
   if (s < 0 || g == 0) return false;

   if (interior)
      q = (p + op::sqrt(s))/g;
   else
      q = (p - op::sqrt(s))/g;
   if (!(0 < q && q < qmin)) return false;

   q.x = eye.x - q*dx;
   q.y = eye.y - q*dy;
   q.z = eye.z - q*dz;

   return q(irsq.x*q.x, irsq.y*q.y, irsq.z*q.z, this, normalized_t::nonorm), true;
} kip_end



// -----------------------------------------------------------------------------
// get_curve
// -----------------------------------------------------------------------------

template<class real, class tag>
inline bool ellipsoid<real,tag>::get_curve(
   const real dx,
   const real dy,
   const real dz,
   const real qmin,
   inq<real,tag> &info
) const {
   // check
   if (!(0 < info.q && info.q < qmin)) return false;

   // intersection
   info.x = basic.eye().x - info.q*dx;
   info.y = basic.eye().y - info.q*dy;
   info.z = basic.eye().z - info.q*dz;

   // normal
   return info(irsq.x*info.x, irsq.y*info.y, irsq.z*info.z, this, normalized_t::nonorm), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(ellipsoid)
{
   if (interior) return ellipsoid<real,tag>::infirst(
      kip_etd,qmin,ints.one(),insub);

   // Outside...
   const point<real>  tar = rot.fore(target);
   const point<real> &eye = basic.eye();

   const real dx = eye.x - tar.x, cix = dx*irsq.x;
   const real dy = eye.y - tar.y, ciy = dy*irsq.y;
   const real dz = eye.z - tar.z, ciz = dz*irsq.z;

   const real g = cix*dx + ciy*dy + ciz*dz;
   const real p = cix*eye.x + ciy*eye.y + ciz*eye.z;
   const real s = p*p - g*i;
   if (s < 0 || g == 0) return false;
   const real tmp = op::sqrt(s);

   ints.convex();

   inq<real,tag> q1((p-tmp)/g);
   if (get_curve(dx,dy,dz,qmin,q1) && ints.convex(q1)) return true;

   if (!ints.size()) return false;

   inq<real,tag> q2((p+tmp)/g);
   if (get_curve(dx,dy,dz,qmin,q2) && ints.convex(q2)) return true;

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(ellipsoid) {

   // (
   //    c.x, c.y, c.z,
   //    a.x, a.y, a.z,
   //    r.x, r.y, r.z,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.c, "center") && read_comma(s) &&
      read_value(s, obj.a, "angles") && read_comma(s) &&
      read_value(s, obj.r, "radii" ) &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }

   obj.a *= real(kip_pi)/180;
   return !s.fail();
}



// kip::ostream
kip_ostream(ellipsoid) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "ellipsoid()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "ellipsoid("
               <<  obj.c << ", "
               << (180/real(kip_pi))*obj.a << ", "
               <<  obj.r &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << "ellipsoid(\n   " &&
             k.indent() <<  obj.c << ",\n   " &&
             k.indent() << (180/real(kip_pi))*obj.a << ",\n   " &&
             k.indent() <<  obj.r &&
             write_finish(k, obj, false);

   kip_ostream_end(ellipsoid);
}

#define   kip_class ellipsoid
#include "kip-macro-io.h"
