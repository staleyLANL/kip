
// -----------------------------------------------------------------------------
// sphere
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class sphere : public shape<real,tag> {

   bool inside(const point<real> &) const;

public:
   using shape<real,tag>::misc;

   // center, radius
   point<real> c;
   real r;

   kip_functions(sphere);

   point<real> back(const point<real> &from) const
      { return from; }

   // sphere([c[,r[,base]]])
   explicit sphere(
      const point<real> &_c = point<real>(0,0,0),
      const real _r = 1
   ) :
      shape<real,tag>(this),
      c(_c), r(_r)
   { this->eyelie = false; }

   explicit sphere(
      const point<real> &_c,
      const real _r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(_c), r(_r)
   { this->eyelie = false; }

   // sphere(c.x,c.y,c.z[,r[,base]])
   explicit sphere(
      const real cx, const real cy, const real cz,
      const real _r = 1
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz), r(_r)
   { this->eyelie = false; }

   explicit sphere(
      const real cx, const real cy, const real cz,
      const real _r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(cx,cy,cz), r(_r)
   { this->eyelie = false; }

   // sphere(sphere)
   sphere(const sphere &from) :
      shape<real,tag>(from),
      c(from.c),
      r(from.r)
   { }

   // sphere=sphere
   sphere &operator=(const sphere &from)
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
   this->interior = inside(eyeball);

   return std::abs(modf-r);
} kip_end

// aabb
kip_aabb(sphere)
{
   return bbox<real>(
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
   real cz;  (void)seg.lt(c,cz);  return cz >= r+seg.valz;
} kip_end

// check
kip_check(sphere)
{
   if (r > 0) return diagnostic::good;
   std::ostringstream oss;
   oss << "Sphere has non-positive radius r=" << r;
   return error(oss);
} kip_end



// -----------------------------------------------------------------------------
// in*
// -----------------------------------------------------------------------------

// infirst
template<class real, class tag>
bool sphere<real,tag>::infirst(
   const eyetardiff<real> &etd, // 72
   const detail::subinfo &,     // 28
   const real qmin,             //  8
   inq<real,tag> &q             // 88
) const {
   const kip::point<real> &eyeball = etd.eyeball;
   const kip::point<real> &diff    = etd.diff;

   const real p = dot(misc.sphere.f(),diff);
   const real h = p*p - misc.sphere.m;
   if (h < 0) return false;

   if (this->interior)
      q = p + std::sqrt(h);
   else
      q = p - std::sqrt(h);

   if (!(0 < q && q < qmin)) return false;

   // diff = "t2e"
   q.inter = eyeball - real(q)*diff;
   return q.set(q.inter-c, this, normalized::no), true;
}



// inall
kip_inall(sphere)
{
   const real p = dot(misc.sphere.f(),diff), h = p*p - misc.sphere.m;
   if (h < 0) return false;

   if (this->interior) {
      ints[0] = p + std::sqrt(h);
      if (!(0 < ints[0] && ints[0] < qmin)) return false;
      ints[0].inter = eyeball - real(ints[0])*diff;
      ints[0].set(ints[0].inter - c, this, normalized::no);
      ints.setsize(1);

   } else {
      const real hsqrt = std::sqrt(h);

      ints[0] = p - hsqrt;
      if (!(0 < ints[0] && ints[0] < qmin)) return false;
      ints[0].inter = eyeball - real(ints[0])*diff;
      ints[0].set(ints[0].inter - c, this, normalized::no);

      ints[1] = p + hsqrt;
      if (0 < ints[1] && ints[1] < qmin) {
         ints[1].inter = eyeball - real(ints[1])*diff;
         ints[1].set(ints[1].inter - c, this, normalized::no);
         ints.setsize(2);
      } else
         ints.setsize(1);
   }

   return true;
} kip_end



// -----------------------------------------------------------------------------
// randomize
// -----------------------------------------------------------------------------

kip_randomize(sphere)
{
   // center
   random_full(obj.c);

   // radius
   obj.r = real(0.15)*random_unit<real>();

   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
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
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(sphere) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "sphere()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "sphere("
               << obj.c << ", "
               << obj.r &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << "sphere(\n   " &&
             k.indent() << obj.c << ",\n   " &&
             k.indent() << obj.r &&
             write_finish(k, obj, false);

   kip_ostream_end(sphere);
}

#define   kip_class sphere
#include "kip-macro-io.h"
