
#pragma once

// -----------------------------------------------------------------------------
// half
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class half : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::misc;

   kip::point<real> point;
   kip::point<real> normal;  // OUT from the material

   kip_functions(half);
   inline kip::point<real> back(const kip::point<real> &from) const
      { return from; }


   // half([point[,normal[,base]]])
   inline explicit half(
      const kip::point<real> &_p = kip::point<real>(real(0), real(0), real(0)),
      const kip::point<real> &_n = kip::point<real>(real(1), real(0), real(0))
   ) :
      shape<real,tag>(this),
      point(_p), normal(_n)
   { this->eyelie = false; }

   inline explicit half(
      const kip::point<real> &_p,
      const kip::point<real> &_n, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      point(_p), normal(_n)
   { this->eyelie = false; }


   // half(px,py,pz[,nx,ny,nz[,base]])
   inline explicit half(
      const real px, const real py, const real pz
   ) :
      shape<real,tag>(this),
      point(px,py,pz), normal(kip::point<real>(real(1), real(0), real(0)))
   { this->eyelie = false; }

   inline explicit half(
      const real px, const real py, const real pz,
      const real nx, const real ny, const real nz
   ) :
      shape<real,tag>(this),
      point(px,py,pz), normal(nx,ny,nz)
   { this->eyelie = false; }

   inline explicit half(
      const real px, const real py, const real pz,
      const real nx, const real ny, const real nz, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      point(px,py,pz), normal(nx,ny,nz)
   { this->eyelie = false; }


   // half(half)
   inline half(const half &from) :
      shape<real,tag>(from),
      point (from.point),
      normal(from.normal)
   { }

   // half = half
   inline half &operator=(const half &from)
   {
      this->shape<real,tag>::operator=(from);
      point  = from.point;
      normal = from.normal;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(half)
{
   misc.half.tmp = dot(normal, eyeball-point);
   const real d = misc.half.tmp/mod(normal);
   return (interior = d <= 0) ? -d : d;
} kip_end



// aabb
kip_aabb(half)
{
   const real infinity = std::numeric_limits<real>::max();

   return
      // x is partially bounded; normal.x != 0, by context
      normal.y == 0 && normal.z == 0
    ? bbox<real>(
         normal.x < 0, normal.x < 0 ? point.x : -infinity,  // [x or (-infinity
         normal.x > 0 ? point.x : infinity, normal.x > 0,  //  x] or infinity)
         false,-infinity, infinity,false,
         false,-infinity, infinity,false
      )

      // y is partially bounded; normal.y != 0, by context
    : normal.x == 0 && normal.z == 0
    ? bbox<real>(
         false,-infinity, infinity,false,
         normal.y < 0, normal.y < 0 ? point.y : -infinity,  // [y or (-infinity
         normal.y > 0 ? point.y : infinity, normal.y > 0,  //  y] or infinity)
         false,-infinity, infinity,false
      )

      // z is partially bounded; normal.z != 0, by context
    : normal.x == 0 && normal.y == 0
    ? bbox<real>(
         false,-infinity, infinity,false,
         false,-infinity, infinity,false,
         normal.z < 0, normal.z < 0 ? point.z : -infinity,  // [z or (-infinity
         normal.z > 0 ? point.z : infinity, normal.z > 0   //  z] or infinity)
      )

      // entirely unbounded
    : bbox<real>(
         false,-infinity, infinity,false,
         false,-infinity, infinity,false,
         false,-infinity, infinity,false
      )
   ;
} kip_end



// dry
kip_dry(half)
{
   // I suppose this is all we can say for sure about a halfspace
   return false;
} kip_end



// check
kip_check(half)
{
   return normal == kip::point<real>(0,0,0)
      ? error("Half has zero normal")
      : diagnostic_t::diagnostic_good;
} kip_end



// infirst
kip_infirst(half)
{
   const real den = dot(normal,diff);
   if (den == 0 || !(0 < (q = misc.half.tmp/den) && q < qmin)) return false;

   q.kip::template point<real>::operator=(eyeball - real(q)*diff);
   return q(normal, this, normalized_t::nonorm), true;
} kip_end



// inall
kip_inall(half)
{
   return half<real,tag>::infirst(kip_etd, qmin,ints.one(), insub);
} kip_end



// randomize
kip_randomize(half)
{
   // center
   random_full(obj.point);

   // normal
   obj.normal(random_full<real>(), random_full<real>(), random_full<real>());

   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(half) {

   // (
   //    px, py, pz,
   //    nx, ny, nz,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.point,  "point" ) && read_comma(s) &&
      read_value(s, obj.normal, "normal") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(half) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "half()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "half("
               << obj.point  << ", "
               << obj.normal &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << "half(\n   " &&
             k.indent() << obj.point << ",\n   " &&
             k.indent() << obj.normal &&
             write_finish(k, obj, false);

   kip_ostream_end(half);
}

#define   kip_class half
#include "kip-macro-io.h"
