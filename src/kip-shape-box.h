
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// box
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class box : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // rotation: clockwise around x, then y, then z; then +c translation
   private: mutable rotate<3,real> rot;
   private: mutable point<real> emr;
   public:  mutable point<real> p[8];
   private: mutable real modr;

public:
   using shape<real,tag>::basic;
   kip_functions(box);

   // compute_corners
   inline void compute_corners(void) const
   {
      // rotation to move corners to their actual locations
      const rotate<3,real> move(a.x, a.y, a.z, c);

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

   inline point<real> back(const point<real> &from) const
      { return rot.back(from); }



   // box([c[,a[,r[,base]]]])
   inline explicit box(
      const point<real> &_c = point<real>(real(0),real(0),real(0)),
      const point<real> &_a = point<real>(real(0),real(0),real(0)),
      const point<real> &_r = point<real>(real(1),real(1),real(1))
   ) :
      shape<real,tag>(this),
      c(_c), a(_a), r(_r)
   { }

   inline explicit box(
      const point<real> &_c,
      const point<real> &_a,
      const point<real> &_r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(_c), a(_a), r(_r)
   { }



   // box(c.x,c.y,c.z[,a.x,a.y,a.z[,r.x,r.y,r.z[,base]]])
   inline explicit box(
      const real &cx, const real &cy, const real &cz
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(real(0),real(0),real(0)),
      r(real(1),real(1),real(1))
   { }

   inline explicit box(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(real(1),real(1),real(1))
   { }

   inline explicit box(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az,
      const real &rx, const real &ry, const real &rz
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(rx,ry,rz)
   { }

   inline explicit box(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az,
      const real &rx, const real &ry, const real &rz, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(rx,ry,rz)
   { }



   // box(box)
   inline box(const box &from) :
      shape<real,tag>(from),
      c(from.c),
      a(from.a),
      r(from.r)
   { }

   // box = box
   inline box &operator=(const box &from)
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
   rot = rotate<3,real>(
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
   interior = inside(eyeball);

   // minimum distance
   return internal::bbox_minimum(basic.eye(), rot.ex, rot.ey, rot.h);
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
   using ostr_t = std::ostringstream;
   diagnostic_t rv = diagnostic_good;

   if (r.x <= real(0)) { ostr_t oss; oss << err << "r.x=" << r.x; rv=error(oss); }
   if (r.y <= real(0)) { ostr_t oss; oss << err << "r.y=" << r.y; rv=error(oss); }
   if (r.z <= real(0)) { ostr_t oss; oss << err << "r.z=" << r.z; rv=error(oss); }

   return rv;
} kip_end



// random
kip_random(box)
{
   // center
   obj.c.x = op::twice(random<real>() - real(0.5));
   obj.c.y = op::twice(random<real>() - real(0.5));
   obj.c.z = op::twice(random<real>() - real(0.5));

   // angles
   obj.a.x = real(kip_pi)*random<real>();
   obj.a.y = real(kip_pi)*random<real>();
   obj.a.z = real(kip_pi)*random<real>();

   // radii
   obj.r.x = real(0.15)*random<real>();
   obj.r.y = real(0.15)*random<real>();
   obj.r.z = real(0.15)*random<real>();

   // base
   random(obj.base());  obj.baseset = true;
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

   if (interior) {
      if (dx < 0) {
         if (0 < (q = -eye.x/dx) && q < qmin &&
             0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey &&
             0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
            q.x = 0;  return q(-1,0,0, this, yesnorm), true;
         }
      } else if (dx > 0 && 0 < (q = (rot.ex - eye.x)/dx) && q < qmin &&
             0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey &&
             0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
         q.x = rot.ex;  return q(1,0,0, this, yesnorm), true;
      }

      if (dy < 0) {
         if (0 < (q = -eye.y/dy) && q < qmin &&
             0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
             0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
            q.y = 0;  return q(0,-1,0, this, yesnorm), true;
         }
      } else if (dy > 0 && 0 < (q = (rot.ey - eye.y)/dy) && q < qmin &&
             0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
             0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
         q.y = rot.ey;  return q(0,1,0, this, yesnorm), true;
      }

      if (dz < 0) {
         if (0 < (q = -eye.z/dz) && q < qmin &&
             0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
             0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey) {
            q.z = 0;  return q(0,0,-1, this, yesnorm), true;
         }
      } else if (dz > 0 && 0 < (q = (rot.h  - eye.z)/dz) && q < qmin &&
             0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
             0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey) {
         q.z = rot.h;  return q(0,0,1, this, yesnorm), true;
      }

      return false;
   }

   // early return if ray misses bounding sphere
   if (mod2(cross(point<real>(dx,dy,dz), emr)) > modr*modr)
      return false;

   // check the 0-planes
   if (eye.x < 0) {
      if (dx <= 0 || (q = -eye.x/dx) >= qmin) return false;
      if (0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey &&
          0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
         q.x = 0;  return q(-1,0,0, this, yesnorm), true;
      }
   }

   if (eye.y < 0) {
      if (dy <= 0 || (q = -eye.y/dy) >= qmin) return false;
      if (0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
          0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
         q.y = 0;  return q(0,-1,0, this, yesnorm), true;
      }
   }

   if (eye.z < 0) {
      if (dz <= 0 || (q = -eye.z/dz) >= qmin) return false;
      if (0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
          0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey) {
         q.z = 0;  return q(0,0,-1, this, yesnorm), true;
      }
   }

   return false;
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(box)
{
   if (interior) return box<real,tag>::infirst(
      kip_etd,qmin,ints.one(),insub);

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
   kip::inq<real,tag> inq;  // try static[nthreads], here and elsewhere

   // x min
   if (0 < (inq.q = -eye.x/dx) && inq.q < qmin &&
       0 <= (inq.y = eye.y + inq.q*dy) && inq.y <= rot.ey &&
       0 <= (inq.z = eye.z + inq.q*dz) && inq.z <= rot.h) {
      inq.x = 0;
      if (ints.convex(inq(-1,0,0, this, yesnorm))) return true;
   }

   // x max
   if (dx > 0 && 0 < (inq.q = (rot.ex - eye.x)/dx) && inq.q < qmin &&
       0 <= (inq.y = eye.y + inq.q*dy) && inq.y <= rot.ey &&
       0 <= (inq.z = eye.z + inq.q*dz) && inq.z <= rot.h) {
      inq.x = rot.ex;
      if (ints.convex(inq(1,0,0, this, yesnorm))) return true;
   }

   // y min
   if (0 < (inq.q = -eye.y/dy) && inq.q < qmin &&
       0 <= (inq.x = eye.x + inq.q*dx) && inq.x <= rot.ex &&
       0 <= (inq.z = eye.z + inq.q*dz) && inq.z <= rot.h) {
      inq.y = 0;
      if (ints.convex(inq(0,-1,0, this, yesnorm))) return true;
   }

   // y max
   if (dy > 0 && 0 < (inq.q = (rot.ey - eye.y)/dy) && inq.q < qmin &&
       0 <= (inq.x = eye.x + inq.q*dx) && inq.x <= rot.ex &&
       0 <= (inq.z = eye.z + inq.q*dz) && inq.z <= rot.h) {
      inq.y = rot.ey;
      if (ints.convex(inq(0,1,0, this, yesnorm))) return true;
   }

   // z min
   if (0 < (inq.q = -eye.z/dz) && inq.q < qmin &&
       0 <= (inq.x = eye.x + inq.q*dx) && inq.x <= rot.ex &&
       0 <= (inq.y = eye.y + inq.q*dy) && inq.y <= rot.ey) {
      inq.z = 0;
      if (ints.convex(inq(0,0,-1, this, yesnorm))) return true;
   }

   // z max
   if (dz > 0 && 0 < (inq.q = (rot.h  - eye.z)/dz) && inq.q < qmin &&
       0 <= (inq.x = eye.x + inq.q*dx) && inq.x <= rot.ex &&
       0 <= (inq.y = eye.y + inq.q*dy) && inq.y <= rot.ey) {
      inq.z = rot.h;
      if (ints.convex(inq(0,0,1, this, yesnorm))) return true;
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
      addendum("Detected while reading "+description, diagnostic_error);
   }

   obj.a *= real(kip_pi)/180;
   return !s.fail();
}



// kip::ostream
kip_ostream(box) {
   bool okay;

   // stub
   if (kip::format == kip::format_stub)
      okay = k << "box()";

   // one
   // op
   else if (kip::format == kip::format_one ||
            kip::format == kip::format_op)
      okay = k << "box("
               <<  obj.c << ", "
               << (180/real(kip_pi))*obj.a << ", "
               <<  obj.r &&
             kip::write_finish(k, obj, true);

   // full
   else
      okay = k << "box(\n   " &&
             k.indent() <<  obj.c << ",\n   " &&
             k.indent() << (180/real(kip_pi))*obj.a << ",\n   " &&
             k.indent() <<  obj.r &&
             kip::write_finish(k, obj, false);

   kip_ostream_end(box);
}

}

#define   kip_class box
#include "kip-macro-io.h"
