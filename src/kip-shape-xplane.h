
#pragma once

// -----------------------------------------------------------------------------
// xplane
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class xplane : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::misc;
   kip_functions(xplane);

   // x value, rectangle y and z sizes, tag (generally color)
   real x;
   class { public: real y, z; } size;
   tag color;

   inline point<real> back(const point<real> &from) const { return from; }



   // xplane([x[,size.y,size.z[,color[,base]]]])
   inline explicit xplane(
      const real _x = real(0)
   ) :
      shape<real,tag>(this),
      x(_x),
      color(tag())
   {
      this->eyelie = false;
      size.y = real(1);
      size.z = real(1);
   }

   inline explicit xplane(
      const real _x,
      const real _ysize, const real _zsize,
      const tag &_color = tag()
   ) :
      shape<real,tag>(this),
      x(_x),
      color(_color)
   {
      this->eyelie = false;
      size.y = _ysize;
      size.z = _zsize;
   }

   inline explicit xplane(
      const real _x,
      const real _ysize, const real _zsize,
      const tag &_color, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      x(_x),
      color(_color)
   {
      this->eyelie = false;
      size.y = _ysize;
      size.z = _zsize;
   }



   // xplane(xplane)
   inline xplane(const xplane &from) :
      shape<real,tag>(from),
      x(from.x),
      color(from.color)
   {
      size.y = from.size.y;
      size.z = from.size.z;
   }

   // xplane = xplane
   inline xplane &operator=(const xplane &from)
   {
      this->shape<real,tag>::operator=(from);
      size.y = from.size.y;  x = from.x;
      size.z = from.size.z;  color = from.color;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(xplane)
{
   misc.plane.h = eyeball.x - x;
   misc.plane.norm = misc.plane.h < 0 ? real(-1) : real(1);
   interior = false;
   return std::abs(misc.plane.h);
} kip_end



// aabb
kip_aabb(xplane)
{
   const real infinity = std::numeric_limits<real>::max();
   return bbox<real>(
      true,x, x,true,
      false,-infinity, infinity,false,
      false,-infinity, infinity,false
   );
} kip_end



// dry
kip_dry(xplane)
{
   return false;
} kip_end



// infirst
kip_infirst(xplane)
{
   return 0 < (q = misc.plane.h/diff.x) && q < qmin
      ? q.x = x,
        q.y = eyeball.y - q*diff.y,
        q.z = eyeball.z - q*diff.z,
        q(
           misc.plane.norm, 0, 0, this,
          (int(std::abs(q.y/size.y)) % 2 == (q.y > 0)) ==
          (int(std::abs(q.z/size.z)) % 2 == (q.z > 0)) ? &color : this,
           normalized_t::yesnorm
        ),
        true
      : false;
} kip_end



// inall
kip_inall(xplane)
{
   return xplane<real,tag>::infirst(kip_etd,qmin,ints.one(),insub)
      ? ints.push(ints[0]), true
      : false;
} kip_end



// check
kip_check(xplane)
{
   diagnostic_t rv = diagnostic_t::diagnostic_good;

   // size.y
   if (size.y <= real(0)) {
      std::ostringstream oss;
      oss << "Xplane has non-positive size.y=" << size.y;
      rv = error(oss);
   }

   // size.z
   if (size.z <= real(0)) {
      std::ostringstream oss;
      oss << "Xplane has non-positive size.z=" << size.z;
      rv = error(oss);
   }

   return rv;
} kip_end



// random
kip_random(xplane)
{
   // x
   obj.x = op::twice(random<real>() - 0.5);

   // size.y, size.z
   obj.size.y = random<real>();
   obj.size.z = random<real>();

   // color
   random(obj.color);

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(xplane) {

   // (
   //    x,
   //    size.y, size.z,
   //    color,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.x, "x value") && read_comma(s) &&
      read_value(s, obj.size.y, "y size") && read_comma(s) &&
      read_value(s, obj.size.z, "z size") && read_comma(s) &&
      read_value(s, obj.color, "color") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



namespace internal {

// plane_write
template<class real, class tag, template<class,class> class OBJ>
kip::ostream &plane_write(
   kip::ostream &k, const OBJ<real,tag> &obj, const char ch,
   const real pos, const real one, const real two
) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << ch << "plane()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << ch << "plane("
               << pos << ", "
               << one << ',' << two << ", "
               << obj.color &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << ch << "plane(\n   " &&
             k.indent() << pos << ",\n   " &&
             k.indent() << one << ',' << two << ",\n   " &&
             k.indent() << obj.color &&
             write_finish(k, obj, false);

   // done
   if (!okay)
      (void)error(std::string("Could not write ") + ch + "plane");
   return k;
}

} // namespace internal



// kip::ostream
kip_ostream(xplane) {
   return internal::plane_write(k, obj, 'x', obj.x, obj.size.y, obj.size.z);
}

#define   kip_class xplane
#include "kip-macro-io.h"
