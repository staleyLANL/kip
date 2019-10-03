
// -----------------------------------------------------------------------------
// zplane
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class zplane : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::misc;
   kip_functions(zplane);

   // z value, rectangle x and y sizes, tag (generally color)
   real z;
   class { public: real x, y; } size;
   tag color;

   inline point<real> back(const point<real> &from) const { return from; }



   // zplane([z[,size.x,size.y[,color[,base]]]])
   inline explicit zplane(
      const real _z = real(0)
   ) :
      shape<real,tag>(this),
      z(_z),
      color(tag())
   {
      this->eyelie = false;
      size.x = real(1);
      size.y = real(1);
   }

   inline explicit zplane(
      const real _z,
      const real _xsize, const real _ysize,
      const tag &_color = tag()
   ) :
      shape<real,tag>(this),
      z(_z),
      color(_color)
   {
      this->eyelie = false;
      size.x = _xsize;
      size.y = _ysize;
   }

   inline explicit zplane(
      const real _z,
      const real _xsize, const real _ysize,
      const tag &_color, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      z(_z),
      color(_color)
   {
      this->eyelie = false;
      size.x = _xsize;
      size.y = _ysize;
   }



   // zplane(zplane)
   inline zplane(const zplane &from) :
      shape<real,tag>(from),
      z(from.z),
      color(from.color)
   {
      size.x = from.size.x;
      size.y = from.size.y;
   }

   // zplane = zplane
   inline zplane &operator=(const zplane &from)
   {
      this->shape<real,tag>::operator=(from);
      size.x = from.size.x;  z = from.z;
      size.y = from.size.y;  color = from.color;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(zplane)
{
   misc.plane.h = eyeball.z - z;
   misc.plane.norm = misc.plane.h < 0 ? real(-1) : real(1);
   interior = false;
   return std::abs(misc.plane.h);
} kip_end



// aabb
kip_aabb(zplane)
{
   const real infinity = std::numeric_limits<real>::max();
   return bbox<real>(
      false,-infinity, infinity,false,
      false,-infinity, infinity,false,
      true,z, z,true
   );
} kip_end



// dry
kip_dry(zplane)
{
   return false;
} kip_end



// infirst
kip_infirst(zplane)
{
   return 0 < (q = misc.plane.h/diff.z) && q < qmin
      ? q.x = eyeball.x - q*diff.x,
        q.y = eyeball.y - q*diff.y,
        q.z = z,
        q(
           0, 0, misc.plane.norm, this,
          (int(std::abs(q.x/size.x)) % 2 == (q.x > 0)) ==
          (int(std::abs(q.y/size.y)) % 2 == (q.y > 0)) ? &color : this,
           normalized::yes
        ),
        true
      : false;
} kip_end



// inall
kip_inall(zplane)
{
   return zplane<real,tag>::infirst(kip_etd,qmin,ints.one(),insub)
      ? ints.push(ints[0]), true
      : false;
} kip_end



// check
kip_check(zplane)
{
   diagnostic rv = diagnostic::good;

   // size.x
   if (size.x <= real(0)) {
      std::ostringstream oss;
      oss << "Zplane has non-positive size.x=" << size.x;
      rv = error(oss);
   }

   // size.y
   if (size.y <= real(0)) {
      std::ostringstream oss;
      oss << "Zplane has non-positive size.y=" << size.y;
      rv = error(oss);
   }

   return rv;
} kip_end



// randomize
kip_randomize(zplane)
{
   // z
   obj.z = random_full<real>();

   // size.x, size.y
   obj.size.x = random_unit<real>();
   obj.size.y = random_unit<real>();

   // color, base
   randomize(obj.color);
   randomize(obj.base());
   obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(zplane) {

   // (
   //    z,
   //    size.x, size.y,
   //    color,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.z, "z value") && read_comma(s) &&
      read_value(s, obj.size.x, "x size") && read_comma(s) &&
      read_value(s, obj.size.y, "y size") && read_comma(s) &&
      read_value(s, obj.color /*, "color"*/ ) &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(zplane) {
   return detail::plane_write(k, obj, 'z', obj.z, obj.size.x, obj.size.y);
}

#define   kip_class zplane
#include "kip-macro-io.h"
