
// -----------------------------------------------------------------------------
// cone
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class cone : public shape<real,tag> {
   bool inside(const point<real> &) const;

   // modified cone: (0,0,0), (h,0,0), r
   mutable rotate<3,real,op::full,op::unscaled> rot;
   mutable real rsq, h1, h2;

   // get_*
   bool
   get_baseh(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const,
   get_curve(const point<real> &,
             const real, const real, const real, inq<real,tag> &) const;

public:
   using shape<real,tag>::basic;
   kip_functions(cone);

   // point a, point b, radius
   point<real> a, b;
   real r;

   point<real> back(const point<real> &from) const
   {
      return rot.back(from);
   }

#define   kip_class cone
#include "kip-macro-onetwor-ctor.h"
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(cone)
{
   rot = rotate<3,real,op::full,op::unscaled>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   const real hsq = rot.h*rot.h;
   const real tmp = r*rot.ex/rot.h;

   rsq =  r*r;
   h1  = -rsq/hsq;
   h2  = (rot.ey - tmp)*(rot.ey + tmp);

   this->interior = inside(eyeball);

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
   detail::bound_abr(b,a,r, min,max);

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
   return op::square(bz-seg.valz) >= h1*(az-bz-rot.h)*(az-bz+rot.h);
} kip_end



// check
kip_check(cone)
{
   return detail::onetwor_check<real>("Cone", *this);
} kip_end



// randomize
kip_randomize(cone)
{
   return detail::random_abr<real,tag>(obj);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(cone)
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
      const real f = dx*dx*(1-h1)-1;
      const real g = rot.ey*dy + h1*rot.ex*dx;
      const real s = g*g + f*h2;
      if (s < 0 || f == 0) return false;

      q = (g - std::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.inter.x = rot.ex + q*dx;  // inside, so don't need the range check

   } else {

      // Outside...
      // zzz

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
      const real f = dx*dx*(1-h1)-1;
      const real g = rot.ey*dy + h1*rot.ex*dx;
      const real s = g*g + f*h2;
      if (s < 0 || f == 0) return false;

      q = (g + std::sqrt(s))/f;
      if (!(0 < q && q < qmin)) return false;
      q.inter.x = rot.ex + q*dx;
      if (!(0 <= q.inter.x && q.inter.x <= rot.h)) return false;

   }

   q.inter.y = rot.ey + q*dy;
   q.inter.z = q*tar.z;
   return q.set(
      h1*q.inter.x, q.inter.y, q.inter.z,
      this, normalized::no
   ), true;

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
inline bool cone<real,tag>::get_curve(
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
      h1*info.inter.x, info.inter.y, info.inter.z,
      this, normalized::no
   ), true;
}



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(cone)
{
   if (this->interior)
      return cone<real,tag>::infirst(etd, insub, qmin, ints.one());

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
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(cone) {
   return detail::onetwor_write(k,obj, obj.a,obj.b,obj.r, "cone");
}

#define   kip_class cone
#include "kip-macro-io.h"
