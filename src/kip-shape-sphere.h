
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// sphere
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class sphere : public shape<real,tag> {

   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::misc;

   // center, radius
   point<real> c;
   real r;

   kip_functions(sphere);

   inline point<real> back(const point<real> &from) const
      { return from; }



   // sphere([c[,r[,base]]])
   inline explicit sphere(
      const point<real> &_c = point<real>(real(0),real(0),real(0)),
      const real &_r = real(1)
   ) :
      shape<real,tag>(this),
      c(_c), r(_r)
   { this->eyelie = false; }

   inline explicit sphere(
      const point<real> &_c,
      const real &_r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(_c), r(_r)
   { this->eyelie = false; }

   // sphere(c.x,c.y,c.z[,r[,base]])
   inline explicit sphere(
      const real &cx, const real &cy, const real &cz,
      const real &_r = real(1)
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz), r(_r)
   { this->eyelie = false; }

   inline explicit sphere(
      const real &cx, const real &cy, const real &cz,
      const real &_r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(cx,cy,cz), r(_r)
   { this->eyelie = false; }



   // sphere(sphere)
   inline sphere(const sphere &from) :
      shape<real,tag>(from),
      c(from.c),
      r(from.r)
   { }

   // sphere=sphere
   inline sphere &operator=(const sphere &from)
   {
      this->shape<real,tag>::operator=(from);
      c = from.c;
      r = from.r;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(sphere)
{
   misc.sphere.f() = eyeball - c;
   const real modf = mod(misc.sphere.f());
   misc.sphere.m = (modf-r)*(modf+r);  // = modf^2 - r^2
   interior = inside(eyeball);

   return op::abs(modf-r);
} kip_end


// aabb
kip_aabb(sphere)
{
   return kip::bbox<real>(
      true, c.x-r,   c.x+r, true,
      true, c.y-r,   c.y+r, true,
      true, c.z-r,   c.z+r, true
   );
} kip_end


// inside
kip_inside(sphere)
{
   return misc.sphere.m <= 0;
} kip_end


// dry
kip_dry(sphere)
{
   // perhaps slightly faster than: return seg.forez(c) >= r;
   real cz;  (void)seg.lt(c,cz);  return cz >= r+seg.c;
} kip_end


// check
kip_check(sphere)
{
   if (r > real(0)) return diagnostic_t::diagnostic_good;
   std::ostringstream oss;
   oss << "Sphere has non-positive radius r=" << r;
   return error(oss);
} kip_end



// -----------------------------------------------------------------------------
// infirst
// inall
// -----------------------------------------------------------------------------

// infirst
kip_infirst(sphere)
{
   const real p = dot(misc.sphere.f(),diff), h = p*p - misc.sphere.m;
   if (h < 0) return false;

   if (interior)
      q = p + op::sqrt(h);
   else
      q = p - op::sqrt(h);
   if (!(0 < q && q < qmin)) return false;

   q.point<real>::operator=(eyeball - real(q)*diff);
   return q(q - c, this, normalized_t::nonorm), true;
} kip_end



// inall
kip_inall(sphere)
{
   const real p = dot(misc.sphere.f(),diff), h = p*p - misc.sphere.m;
   if (h < 0) return false;

   if (interior) {
      ints[0] = p + op::sqrt(h);
      if (!(0 < ints[0] && ints[0] < qmin)) return false;
      ints[0].point<real>::operator=(eyeball - real(ints[0])*diff);
      ints[0](ints[0] - c, this, normalized_t::nonorm);
      ints.setsize(1);

   } else {
      const real hsqrt = op::sqrt(h);

      ints[0] = p - hsqrt;
      if (!(0 < ints[0] && ints[0] < qmin)) return false;
      ints[0].point<real>::operator=(eyeball - real(ints[0])*diff);
      ints[0](ints[0] - c, this, normalized_t::nonorm);

      ints[1] = p + hsqrt;
      if (0 < ints[1] && ints[1] < qmin) {
         ints[1].point<real>::operator=(eyeball - real(ints[1])*diff);
         ints[1](ints[1] - c, this, normalized_t::nonorm);
         ints.setsize(2);
      } else
         ints.setsize(1);
   }

   return true;
} kip_end



// -----------------------------------------------------------------------------
// random
// -----------------------------------------------------------------------------

kip_random(sphere)
{
   // center
   obj.c.x = op::twice(random<real>() - real(0.5));
   obj.c.y = op::twice(random<real>() - real(0.5));
   obj.c.z = op::twice(random<real>() - real(0.5));

   // radius
   obj.r = real(0.15)*random<real>();

   // base
   random(obj.base());  obj.baseset = true;
   return obj;

   /*
   // center
   obj.c.x =  4*(random<real>() - real(0.5));
   obj.c.y = 10*(random<real>() - real(0.1));
   obj.c.z =  2*(random<real>() - real(0.5));

   // radius
   obj.r = real(0.02)*random<real>();

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
   */

} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(sphere) {

   // (
   //    c.x, c.y, c.z,
   //    r,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.c, "center") && read_comma(s) &&
      read_value(s, obj.r, "radius") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(sphere) {
   bool okay;

   // stub
   if (kip::format == kip::format_t::format_stub)
      okay = k << "sphere()";

   // one
   // op
   else if (kip::format == kip::format_t::format_one ||
            kip::format == kip::format_t::format_op)
      okay = k << "sphere("
               << obj.c << ", "
               << obj.r &&
             kip::write_finish(k, obj, true);

   // full
   else
      okay = k << "sphere(\n   " &&
             k.indent() << obj.c << ",\n   " &&
             k.indent() << obj.r &&
             kip::write_finish(k, obj, false);

   kip_ostream_end(sphere);
}

}

#define   kip_class sphere
#include "kip-macro-io.h"
