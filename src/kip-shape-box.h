
// -----------------------------------------------------------------------------
// box
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class box : public shape<real,tag> {
   bool inside(const point<real> &) const;

   // rotation: clockwise around x, then y, then z; then +c translation
   private: mutable rotate<3,real,op::full,op::unscaled> rot;
   private: mutable point<real> emr;
   public:  mutable point<real> p[8];
   private: mutable real modr;

public:
   using shape<real,tag>::basic;
   kip_functions(box);

   // compute_corners
   void compute_corners() const
   {
      // rotation to move corners to their actual locations
      const rotate<3,real,op::full,op::unscaled> move(a.x, a.y, a.z, c);

      // p: the actual corners
      p[0] = move.back(-r.x, -r.y, -r.z);
      p[1] = move.back( r.x, -r.y, -r.z);
      p[2] = move.back( r.x,  r.y, -r.z);
      p[3] = move.back(-r.x,  r.y, -r.z);
      p[4] = move.back(-r.x, -r.y,  r.z);
      p[5] = move.back( r.x, -r.y,  r.z);
      p[6] = move.back( r.x,  r.y,  r.z);
      p[7] = move.back(-r.x,  r.y,  r.z);
   }

   // center, angles, radii (half-sizes)
   point<real> c;
   point<real> a;
   point<real> r;

   point<real> back(const point<real> &from) const
      { return rot.back(from); }



   // box([c[,a[,r[,base]]]])
   explicit box(
      const point<real> &_c = point<real>(0,0,0),
      const point<real> &_a = point<real>(0,0,0),
      const point<real> &_r = point<real>(1,1,1)
   ) :
      shape<real,tag>(this),
      c(_c), a(_a), r(_r)
   { }

   explicit box(
      const point<real> &_c,
      const point<real> &_a,
      const point<real> &_r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(_c), a(_a), r(_r)
   { }



   // box(c.x,c.y,c.z[,a.x,a.y,a.z[,r.x,r.y,r.z[,base]]])
   explicit box(
      const real cx, const real cy, const real cz
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(0,0,0),
      r(1,1,1)
   { }

   explicit box(
      const real cx, const real cy, const real cz,
      const real ax, const real ay, const real az
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(1,1,1)
   { }

   explicit box(
      const real cx, const real cy, const real cz,
      const real ax, const real ay, const real az,
      const real rx, const real ry, const real rz
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(rx,ry,rz)
   { }

   explicit box(
      const real cx, const real cy, const real cz,
      const real ax, const real ay, const real az,
      const real rx, const real ry, const real rz, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(rx,ry,rz)
   { }



   // box(box)
   box(const box &from) :
      shape<real,tag>(from),
      c(from.c),
      a(from.a),
      r(from.r)
   { }

   // box = box
   box &operator=(const box &from)
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

kip_process(box)
{
   compute_corners();  // computes p[0..7], the actual corner locations

   /*
      z
      |  y
      | /
      |/
      +-----x


           7 +--------------+ 6
            /|             /|
           / |            / |
          /  |           /  |
         /   |          /   |
      4 +--------------+ 5  |
        |    |         |    |
        |    |         |    |
        |  3 +--------------+ 2
        |   /          |   /
        |  /           |  /
        | /            | /
        |/             |/
      0 +--------------+ 1
   */

   // rot, rot.h, eye, emr
   // note: rot.h is changed for convenience; original is not otherwise needed
   static const unsigned p0 = 0, p1 = 1, p2 = 2, p3 = 3;
   static const unsigned p4 = 4, p5 = 5, p6 = 6, p7 = 7;

   const real x = r.y*r.z;
   const real y = r.x*r.z;
   const real z = r.x*r.y;

   const unsigned xyz = x > y && x > z ? 0 : y > x && y > z ? 1 : 2;
   const real *const are[3] = { &r.x, &r.y, &r.z };

   // Theoretical statistically-optimal (for speed) 3-rotation point orderings,
   // given ray check against x=0, y=0, and z=0 planes, in that order.
   static const unsigned order[8][3][6] = {
      {{ p0, p1, p2, 0,1,2 },  // 0 x [p0] [p1] [p2] xyz
      {  p0, p3, p7, 1,2,0 },  // 0 y [p0] [p3] [p7] yzx
      {  p0, p4, p5, 2,0,1 }}, // 0 z [p0] [p4] [p5] zxy

      {{ p1, p0, p4, 0,2,1 },  // 1 x [p1] [p0] [p4] xzy
      {  p1, p2, p3, 1,0,2 },  // 1 y [p1] [p2] [p3] yxz
      {  p1, p5, p6, 2,1,0 }}, // 1 z [p1] [p5] [p6] zyx

      {{ p2, p3, p0, 0,1,2 },  // 2 x [p2] [p3] [p0] xyz
      {  p2, p1, p5, 1,2,0 },  // 2 y [p2] [p1] [p5] yzx
      {  p2, p6, p7, 2,0,1 }}, // 2 z [p2] [p6] [p7] zxy

      {{ p3, p2, p6, 0,2,1 },  // 3 x [p3] [p2] [p6] xzy
      {  p3, p0, p1, 1,0,2 },  // 3 y [p3] [p0] [p1] yxz
      {  p3, p7, p4, 2,1,0 }}, // 3 z [p3] [p7] [p4] zyx

      {{ p4, p5, p1, 0,2,1 },  // 4 x [p4] [p5] [p1] xzy
      {  p4, p7, p6, 1,0,2 },  // 4 y [p4] [p7] [p6] yxz
      {  p4, p0, p3, 2,1,0 }}, // 4 z [p4] [p0] [p3] zyx

      {{ p5, p4, p7, 0,1,2 },  // 5 x [p5] [p4] [p7] xyz
      {  p5, p6, p2, 1,2,0 },  // 5 y [p5] [p6] [p2] yzx
      {  p5, p1, p0, 2,0,1 }}, // 5 z [p5] [p1] [p0] zxy

      {{ p6, p7, p3, 0,2,1 },  // 6 x [p6] [p7] [p3] xzy
      {  p6, p5, p4, 1,0,2 },  // 6 y [p6] [p5] [p4] yxz
      {  p6, p2, p1, 2,1,0 }}, // 6 z [p6] [p2] [p1] zyx

      {{ p7, p6, p5, 0,1,2 },  // 7 x [p7] [p6] [p5] xyz
      {  p7, p4, p0, 1,2,0 },  // 7 y [p7] [p4] [p0] yzx
      {  p7, p3, p2, 2,0,1 }}  // 7 z [p7] [p3] [p2] zxy
   };

   unsigned ppos = 0;  // index of closest point to eyeball, to be determined
   real pmin = mod2(p[0] - eyeball);

   for (unsigned n = 1;  n < 8;  ++n) {
      const real newmin = mod2(p[n] - eyeball);
      if (newmin < pmin) {
         ppos = n;
         pmin = newmin;
      }
   }

   // rot
   rot = rotate<3,real,op::full,op::unscaled>(
      p[ order[ppos][xyz][0] ],
      p[ order[ppos][xyz][1] ],
      p[ order[ppos][xyz][2] ]
   );

   // eye
   basic.eye() = rot.fore(eyeball);

   // emr
   emr = basic.eye() - point<real>(
      *are[ order[ppos][xyz][3] ],
      *are[ order[ppos][xyz][4] ],
      *are[ order[ppos][xyz][5] ]
   );

   // rot.h
   rot.h = 2**are[order[ppos][xyz][5]];

   // lie
   basic.lie() = point<float>(rot.fore(light));

   // modr
   modr = mod(r);

   // interior
   this->interior = inside(eyeball);

   // minimum distance
   return detail::bbox_minimum(basic.eye(), rot.ex, rot.ey, rot.h);
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(box)
{
   compute_corners();  // computes p[0..7], the actual corner locations

   // [0]
   real xmin = p[0].x, xmax = xmin;
   real ymin = p[0].y, ymax = ymin;
   real zmin = p[0].z, zmax = zmin;

   // [1..7]
   for (unsigned n = 1;  n < 8;  ++n) {
      xmin = op::min(xmin, p[n].x), xmax = op::max(xmax, p[n].x);
      ymin = op::min(ymin, p[n].y), ymax = op::max(ymax, p[n].y);
      zmin = op::min(zmin, p[n].z), zmax = op::max(zmax, p[n].z);
   }

   // done
   return bbox<real>(
      true,xmin, xmax,true,
      true,ymin, ymax,true,
      true,zmin, zmax,true
   );
} kip_end



// inside
kip_inside(box)
{
   return 0 <= basic.eye().x && 0 <= basic.eye().y && 0 <= basic.eye().z;
} kip_end



// dry
kip_dry(box)
{
   return
      seg.ge(p[0]) && seg.ge(p[6]) && seg.ge(p[1]) && seg.ge(p[7]) &&
      seg.ge(p[2]) && seg.ge(p[4]) && seg.ge(p[3]) && seg.ge(p[5]);
} kip_end



// check
kip_check(box)
{
   static const char *const err = "Box has non-positive half-size ";
   using ostring = std::ostringstream;
   diagnostic rv = diagnostic::good;

   if (r.x <= 0) { ostring oss; oss << err << "r.x=" << r.x; rv=error(oss); }
   if (r.y <= 0) { ostring oss; oss << err << "r.y=" << r.y; rv=error(oss); }
   if (r.z <= 0) { ostring oss; oss << err << "r.z=" << r.z; rv=error(oss); }

   return rv;
} kip_end



// randomize
kip_randomize(box)
{
   // center
   random_full(obj.c);

   // angles
   obj.a.x = pi<real>*random_unit<real>();
   obj.a.y = pi<real>*random_unit<real>();
   obj.a.z = pi<real>*random_unit<real>();

   // radii
   obj.r.x = real(0.15)*random_unit<real>();
   obj.r.y = real(0.15)*random_unit<real>();
   obj.r.z = real(0.15)*random_unit<real>();

   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(box)
{
   const point<real>  tar = rot.fore(target);
   const point<real> &eye = basic.eye();

   const real dx = tar.x - eye.x;
   const real dy = tar.y - eye.y;
   const real dz = tar.z - eye.z;

   if (this->interior) {
      if (dx < 0) {
         if (0 < (q = -eye.x/dx) && q < qmin &&
             0 <= (q.inter.y = eye.y + q*dy) && q.inter.y <= rot.ey &&
             0 <= (q.inter.z = eye.z + q*dz) && q.inter.z <= rot.h) {
            q.inter.x = 0;  return q.set(-1,0,0, this, normalized::yes), true;
         }
      } else if (dx > 0 && 0 < (q = (rot.ex - eye.x)/dx) && q < qmin &&
             0 <= (q.inter.y = eye.y + q*dy) && q.inter.y <= rot.ey &&
             0 <= (q.inter.z = eye.z + q*dz) && q.inter.z <= rot.h) {
         q.inter.x = rot.ex;  return q.set(1,0,0, this, normalized::yes), true;
      }

      if (dy < 0) {
         if (0 < (q = -eye.y/dy) && q < qmin &&
             0 <= (q.inter.x = eye.x + q*dx) && q.inter.x <= rot.ex &&
             0 <= (q.inter.z = eye.z + q*dz) && q.inter.z <= rot.h) {
            q.inter.y = 0;  return q.set(0,-1,0, this, normalized::yes), true;
         }
      } else if (dy > 0 && 0 < (q = (rot.ey - eye.y)/dy) && q < qmin &&
             0 <= (q.inter.x = eye.x + q*dx) && q.inter.x <= rot.ex &&
             0 <= (q.inter.z = eye.z + q*dz) && q.inter.z <= rot.h) {
         q.inter.y = rot.ey;  return q.set(0,1,0, this, normalized::yes), true;
      }

      if (dz < 0) {
         if (0 < (q = -eye.z/dz) && q < qmin &&
             0 <= (q.inter.x = eye.x + q*dx) && q.inter.x <= rot.ex &&
             0 <= (q.inter.y = eye.y + q*dy) && q.inter.y <= rot.ey) {
            q.inter.z = 0;  return q.set(0,0,-1, this, normalized::yes), true;
         }
      } else if (dz > 0 && 0 < (q = (rot.h  - eye.z)/dz) && q < qmin &&
             0 <= (q.inter.x = eye.x + q*dx) && q.inter.x <= rot.ex &&
             0 <= (q.inter.y = eye.y + q*dy) && q.inter.y <= rot.ey) {
         q.inter.z = rot.h;  return q.set(0,0,1, this, normalized::yes), true;
      }

      return false;
   }

   // early return if ray misses bounding sphere
   if (mod2(cross(point<real>(dx,dy,dz), emr)) > modr*modr)
      return false;

   // check the 0-planes
   if (eye.x < 0) {
      if (dx <= 0 || (q = -eye.x/dx) >= qmin) return false;
      if (0 <= (q.inter.y = eye.y + q*dy) && q.inter.y <= rot.ey &&
          0 <= (q.inter.z = eye.z + q*dz) && q.inter.z <= rot.h) {
         q.inter.x = 0;  return q.set(-1,0,0, this, normalized::yes), true;
      }
   }

   if (eye.y < 0) {
      if (dy <= 0 || (q = -eye.y/dy) >= qmin) return false;
      if (0 <= (q.inter.x = eye.x + q*dx) && q.inter.x <= rot.ex &&
          0 <= (q.inter.z = eye.z + q*dz) && q.inter.z <= rot.h) {
         q.inter.y = 0;  return q.set(0,-1,0, this, normalized::yes), true;
      }
   }

   if (eye.z < 0) {
      if (dz <= 0 || (q = -eye.z/dz) >= qmin) return false;
      if (0 <= (q.inter.x = eye.x + q*dx) && q.inter.x <= rot.ex &&
          0 <= (q.inter.y = eye.y + q*dy) && q.inter.y <= rot.ey) {
         q.inter.z = 0;  return q.set(0,0,-1, this, normalized::yes), true;
      }
   }

   return false;
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(box)
{
   if (this->interior)
      return box<real,tag>::infirst(etd, insub, qmin, ints.one());

   // Outside...
   const point<real>  tar = rot.fore(target);
   const point<real> &eye = basic.eye();

   const real dx = tar.x - eye.x;
   const real dy = tar.y - eye.y;
   const real dz = tar.z - eye.z;

   // early return if ray misses bounding sphere
   if (mod2(cross(point<real>(dx,dy,dz), emr)) > modr*modr)
      return false;

   // check each plane...
   ints.convex();
   inq<real,tag> inq;  // try static[nthreads], here and elsewhere

   // x min
   if (0 < (inq.q = -eye.x/dx) && inq.q < qmin &&
       0 <= (inq.inter.y = eye.y + inq.q*dy) && inq.inter.y <= rot.ey &&
       0 <= (inq.inter.z = eye.z + inq.q*dz) && inq.inter.z <= rot.h) {
      inq.inter.x = 0;
      if (ints.convex(inq.set(-1,0,0, this, normalized::yes))) return true;
   }

   // x max
   if (dx > 0 && 0 < (inq.q = (rot.ex - eye.x)/dx) && inq.q < qmin &&
       0 <= (inq.inter.y = eye.y + inq.q*dy) && inq.inter.y <= rot.ey &&
       0 <= (inq.inter.z = eye.z + inq.q*dz) && inq.inter.z <= rot.h) {
      inq.inter.x = rot.ex;
      if (ints.convex(inq.set(1,0,0, this, normalized::yes))) return true;
   }

   // y min
   if (0 < (inq.q = -eye.y/dy) && inq.q < qmin &&
       0 <= (inq.inter.x = eye.x + inq.q*dx) && inq.inter.x <= rot.ex &&
       0 <= (inq.inter.z = eye.z + inq.q*dz) && inq.inter.z <= rot.h) {
      inq.inter.y = 0;
      if (ints.convex(inq.set(0,-1,0, this, normalized::yes))) return true;
   }

   // y max
   if (dy > 0 && 0 < (inq.q = (rot.ey - eye.y)/dy) && inq.q < qmin &&
       0 <= (inq.inter.x = eye.x + inq.q*dx) && inq.inter.x <= rot.ex &&
       0 <= (inq.inter.z = eye.z + inq.q*dz) && inq.inter.z <= rot.h) {
      inq.inter.y = rot.ey;
      if (ints.convex(inq.set(0,1,0, this, normalized::yes))) return true;
   }

   // z min
   if (0 < (inq.q = -eye.z/dz) && inq.q < qmin &&
       0 <= (inq.inter.x = eye.x + inq.q*dx) && inq.inter.x <= rot.ex &&
       0 <= (inq.inter.y = eye.y + inq.q*dy) && inq.inter.y <= rot.ey) {
      inq.inter.z = 0;
      if (ints.convex(inq.set(0,0,-1, this, normalized::yes))) return true;
   }

   // z max
   if (dz > 0 && 0 < (inq.q = (rot.h  - eye.z)/dz) && inq.q < qmin &&
       0 <= (inq.inter.x = eye.x + inq.q*dx) && inq.inter.x <= rot.ex &&
       0 <= (inq.inter.y = eye.y + inq.q*dy) && inq.inter.y <= rot.ey) {
      inq.inter.z = rot.h;
      if (ints.convex(inq.set(0,0,1, this, normalized::yes))) return true;
   }

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(box) {

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
      read_value(s, obj.r, "half-sizes") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   }

   obj.a *= pi<real>/180;
   return !s.fail();
}



// kip::ostream
kip_ostream(box) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "box()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "box("
               <<  obj.c << ", "
               << (180/pi<real>)*obj.a << ", "
               <<  obj.r &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << "box(\n   " &&
             k.indent() <<  obj.c << ",\n   " &&
             k.indent() << (180/pi<real>)*obj.a << ",\n   " &&
             k.indent() <<  obj.r &&
             write_finish(k, obj, false);

   kip_ostream_end(box);
}

#define   kip_class box
#include "kip-macro-io.h"
