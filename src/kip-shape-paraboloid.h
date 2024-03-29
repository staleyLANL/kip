
// -----------------------------------------------------------------------------
// paraboloid
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class paraboloid : public shape<real,tag> {
   bool inside(const point<real> &) const;

   // modified paraboloid: (0,0,0), (h,0,0), r
   mutable rotate<3,real,op::full,op::unscaled> rot;
   mutable real rsq, h1, h2, h3;

   // get_*
   bool
   get_baseh(
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
   kip_functions(paraboloid);

   // point a, point b, radius
   point<real> a, b;
   real r;

   point<real> back(const point<real> &from) const
   {
      return rot.back(from);
   }

#define   kip_class paraboloid
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(paraboloid)
{
   rot = rotate<3,real,op::full,op::unscaled>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   rsq = r*r;
   h1 = r/rot.h;
   h2 = 2*rot.h/rsq;
   h3 = h2*rot.ey*rot.ey - 2*rot.ex;

   // interior
   this->interior = inside(eyeball);

   // minimum
   if (rot.ex >= rot.h && rot.ey <= r)
      return rot.ex - rot.h;  // east

   else if (r*(r - rot.ey) <= 2*rot.h*(rot.ex - rot.h)) {
      return std::sqrt(
         op::square(rot.ex-rot.h) +
         op::square(rot.ey-r)
      );  // northeast

   } else {
      // safe - based on bounding cylinder
      return
         rot.ex <= 0 && rot.ey >= r
      ?  std::sqrt(op::square(rot.ex) + op::square(rot.ey-r))
      :  rot.ex <= 0
      ? -rot.ex
      :  rot.ey >= r
      ?  rot.ey-r
      :  0;  // inside bounding cylinder (and possibly inside paraboloid)
   }
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// bound_paraboloid - helper
namespace detail {
   template<class real>
   inline void bound_paraboloid(
      const real a, const real b, const real c, const real h, const real r,
      const real one, const real two, real &min, real &max
   ) {
      const real v = r * std::sqrt(b*b + c*c);  min = two - v;
      const real u = op::twice(std::abs(a)*h);  max = two + v;

      if (v <= u) {
         const real at = v*v/(u+u);
         min = op::min(min, one-at);
         max = op::max(max, one+at);
      }
   }
}

// aabb
kip_aabb(paraboloid)
{
   real xmin, ymin, zmin;
   const rotate<2,real,op::full,op::unscaled> rot2(a,b);
   real xmax, ymax, zmax;

   detail::bound_paraboloid(
      rot2.mat.a.x, rot2.mat.b.x, rot2.mat.c.x, rot2.h,r, a.x,b.x, xmin,xmax);
   detail::bound_paraboloid(
      rot2.mat.a.y, rot2.mat.b.y, rot2.mat.c.y, rot2.h,r, a.y,b.y, ymin,ymax);
   detail::bound_paraboloid(
      rot2.mat.a.z, real(0), rot2.mat.c.z, rot2.h,r, a.z,b.z, zmin,zmax);

   return bbox<real>(
      true,xmin, xmax,true,
      true,ymin, ymax,true,
      true,zmin, zmax,true
   );
} kip_end



// inside
kip_inside(paraboloid)
{
   return 0 <= rot.ex && rot.ex <= rot.h && h3 <= 0;
} kip_end



// dry
kip_dry(paraboloid)
{
   real az;  if (seg.lt(a,az)) return false;
   real bz;  if (seg.lt(b,bz)) return false;

   const real tmp = h1*h1*(rot.h-az+bz)*(rot.h+az-bz);
   az -= seg.valz;
   bz -= seg.valz;
   return bz*bz >= tmp && (bz <= az || az*az >= tmp || 4*(bz-az)*az >= tmp);
} kip_end



// check
kip_check(paraboloid)
{
   return detail::onetwor_check<real>("Paraboloid", *this);
} kip_end



// randomize
kip_randomize(paraboloid)
{
   return detail::random_abr<real,tag>(obj);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(paraboloid)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   if (this->interior) {

      // x = h
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

      // curve
      const real f = h2*(1-dx*dx), g = dx - h2*dy*rot.ey, s = g*g - f*h3;
      if (s < 0 || f == 0) return false;

      q = (g + std::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.inter.x = rot.ex + q*dx;  // inside, so don't need the range check

   } else {

      // Outside...

      // x = h
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

      // curve
      const real f = h2*(1-dx*dx), g = dx - h2*dy*rot.ey, s = g*g - f*h3;
      if (s < 0 || f == 0) return false;

      q = (g - std::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.inter.x = rot.ex + real(q)*dx;
      if (!(0 <= q.inter.x && q.inter.x <= rot.h)) return false;
   }

   /*
   paraboloid:
      (h/r^2)*(y^2 + z^2) - x = 0

      partial x = -1
      partial y =  2*h/r^2 * y = h2*y
      partial z =  2*h/r^2 * z = h2*z

   can scale by 1/h2 to get normal.
   */

   q.inter.y = rot.ey + real(q)*dy;
   q.inter.z = real(q)*tar.z;
   return q.set(
     -1/h2, q.inter.y, q.inter.z,
      this, normalized::no
   ), true;

} kip_end



// -----------------------------------------------------------------------------
// get_baseh
// get_curve
// -----------------------------------------------------------------------------

// get_baseh
template<class real, class tag>
inline bool paraboloid<real,tag>::get_baseh(
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
inline bool paraboloid<real,tag>::get_curve(
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
     -1/h2, info.inter.y, info.inter.z,
      this, normalized::no
   ), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(paraboloid)
{
   if (this->interior)
      return paraboloid<real,tag>::infirst(etd, insub, qmin, ints.one());

   // Outside...
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   const real f = h2*(1-dx*dx), g = dx - h2*dy*rot.ey, s = g*g - f*h3;
   // zzz might actually be a problem with the f == 0 test (cone and cyl, too?)
   if (s < 0 || (rot.ex > rot.h && dx >= 0) || f == 0) return false;
   const real tmp = std::sqrt(s);

   ints.convex();
   inq<real,tag> q1((g-tmp)/f);
   inq<real,tag> q2((g+tmp)/f), qf;

   if ((get_curve(tar,dx,dy,qmin,q1) && ints.convex(q1)) ||
       (get_curve(tar,dx,dy,qmin,q2) && ints.convex(q2)) ||
       (get_baseh(tar,dx,dy,qmin,qf) && ints.convex(qf)))
      return true;

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(paraboloid) {

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
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(paraboloid) {
   return detail::onetwor_write(k,obj, obj.a,obj.b,obj.r, "paraboloid");
}

#define   kip_class paraboloid
#include "kip-macro-io.h"
