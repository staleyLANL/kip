
// -----------------------------------------------------------------------------
// yplane
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class yplane : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::misc;
   kip_functions(yplane);

   // y value, rectangle x and z sizes, tag (generally color)
   real y;
   class { public: real x, z; } size;
   tag color;

   inline point<real> back(const point<real> &from) const { return from; }



   // yplane([y[,size.x,size.z[,color[,base]]]])
   inline explicit yplane(
      const real _y = real(0)
   ) :
      shape<real,tag>(this),
      y(_y),
      color(tag())
   {
      this->eyelie = false;
      size.x = real(1);
      size.z = real(1);
   }

   inline explicit yplane(
      const real _y,
      const real _xsize, const real _zsize,
      const tag &_color = tag()
   ) :
      shape<real,tag>(this),
      y(_y),
      color(_color)
   {
      this->eyelie = false;
      size.x = _xsize;
      size.z = _zsize;
   }

   inline explicit yplane(
      const real _y,
      const real _xsize, const real _zsize,
      const tag &_color, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      y(_y),
      color(_color)
   {
      this->eyelie = false;
      size.x = _xsize;
      size.z = _zsize;
   }



   // yplane(yplane)
   inline yplane(const yplane &from) :
      shape<real,tag>(from),
      y(from.y),
      color(from.color)
   {
      size.x = from.size.x;
      size.z = from.size.z;
   }

   // yplane = yplane
   inline yplane &operator=(const yplane &from)
   {
      this->shape<real,tag>::operator=(from);
      size.x = from.size.x;  y = from.y;
      size.z = from.size.z;  color = from.color;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(yplane)
{
   misc.plane.h = eyeball.y - y;
   misc.plane.norm = misc.plane.h < 0 ? real(-1) : real(1);
   interior = false;
   return std::abs(misc.plane.h);
} kip_end



// aabb
kip_aabb(yplane)
{
   const real infinity = std::numeric_limits<real>::max();
   return bbox<real>(
      false,-infinity, infinity,false,
      true,y, y,true,
      false,-infinity, infinity,false
   );
} kip_end



// dry
kip_dry(yplane)
{
   return false;
} kip_end



// infirst
kip_infirst(yplane)
{
   return 0 < (q = misc.plane.h/diff.y) && q < qmin
      ? q.x = eyeball.x - q*diff.x,
        q.y = y,
        q.z = eyeball.z - q*diff.z,
        q(
           0, misc.plane.norm, 0, this,
          (int(std::abs(q.x/size.x)) % 2 == (q.x > 0)) ==
          (int(std::abs(q.z/size.z)) % 2 == (q.z > 0)) ? &color : this,
           normalized::yes
        ),
        true
      : false;
} kip_end



// inall
kip_inall(yplane)
{
   return yplane<real,tag>::infirst(kip_etd,qmin,ints.one(),insub)
      ? ints.push(ints[0]), true
      : false;
} kip_end



// check
kip_check(yplane)
{
   diagnostic rv = diagnostic::good;

   // size.x
   if (size.x <= real(0)) {
      std::ostringstream oss;
      oss << "Yplane has non-positive size.x=" << size.x;
      rv = error(oss);
   }

   // size.z
   if (size.z <= real(0)) {
      std::ostringstream oss;
      oss << "Yplane has non-positive size.z=" << size.z;
      rv = error(oss);
   }

   return rv;
} kip_end



// randomize
kip_randomize(yplane)
{
   // y
   obj.y = random_full<real>();

   // size.x, size.z
   obj.size.x = random_unit<real>();
   obj.size.z = random_unit<real>();

   // color
   randomize(obj.color);

   // base
   randomize(obj.base());
   obj.baseset = true;

   // done
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(yplane) {

   // (
   //    y,
   //    size.x, size.z,
   //    color,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.y, "y value") && read_comma(s) &&
      read_value(s, obj.size.x, "x size") && read_comma(s) &&
      read_value(s, obj.size.z, "z size") && read_comma(s) &&
      read_value(s, obj.color /*, "color"*/ ) &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(yplane) {
   return detail::plane_write(k, obj, 'y', obj.y, obj.size.x, obj.size.z);
}

#define   kip_class yplane
#include "kip-macro-io.h"
