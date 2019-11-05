
// -----------------------------------------------------------------------------
// xplane
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class xplane : public shape<real,tag> {
public:
   using shape<real,tag>::misc;
   kip_functions(xplane);

   // x value, rectangle y and z sizes, tag (generally color)
   real x;
   class { public: real y, z; } size;
   tag color;

   point<real> back(const point<real> &from) const { return from; }

   // xplane([x[,size.y,size.z[,color[,base]]]])
   explicit xplane(
      const real _x = 0
   ) :
      shape<real,tag>(this),
      x(_x),
      color(tag())
   {
      this->eyelie = false;
      size.y = 1;
      size.z = 1;
   }

   explicit xplane(
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

   explicit xplane(
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
   xplane(const xplane &from) :
      shape<real,tag>(from),
      x(from.x),
      color(from.color)
   {
      size.y = from.size.y;
      size.z = from.size.z;
   }

   // xplane = xplane
   xplane &operator=(const xplane &from)
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
   misc.plane.norm = misc.plane.h < 0 ? -1 : 1;
   this->interior = false;
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
      ? q.inter.x = x,
        q.inter.y = eyeball.y - q*diff.y,
        q.inter.z = eyeball.z - q*diff.z,
        q.set(
           misc.plane.norm, 0, 0, this,
          (int(std::abs(q.inter.y/size.y)) % 2 == (q.inter.y > 0)) ==
          (int(std::abs(q.inter.z/size.z)) % 2 == (q.inter.z > 0))
           ? &color
           : &this->base(),
           normalized::yes
        ),
        true
      : false;
} kip_end


// inall
kip_inall(xplane)
{
   return xplane<real,tag>::infirst(etd, insub, qmin, ints.one())
      ? ints.push(ints[0]), true
      : false;
} kip_end


// check
kip_check(xplane)
{
   diagnostic rv = diagnostic::good;

   // size.y
   if (size.y <= 0) {
      std::ostringstream oss;
      oss << "Xplane has non-positive size.y=" << size.y;
      rv = error(oss);
   }

   // size.z
   if (size.z <= 0) {
      std::ostringstream oss;
      oss << "Xplane has non-positive size.z=" << size.z;
      rv = error(oss);
   }

   return rv;
} kip_end


// randomize
kip_randomize(xplane)
{
   // x
   obj.x = random_full<real>();

   // size.y, size.z
   obj.size.y = random_unit<real>();
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
      read_value(s, obj.color /*, "color"*/ ) &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



namespace detail {

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

} // namespace detail



// kip::ostream
kip_ostream(xplane) {
   return detail::plane_write(k, obj, 'x', obj.x, obj.size.y, obj.size.z);
}

#define   kip_class xplane
#include "kip-macro-io.h"
