
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// cube
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class cube : public shape<real,tag> {
   inline bool inside(const point<real> &) const;
   using shape<real,tag>::interior;

   // rotation: clockwise around x, then y, then z; then +c translation
   private: mutable rotate<3,real> rot;
   private: mutable point<real> emr;
   public:  mutable point<real> p[8];
   private: mutable real modr2;

public:
   using shape<real,tag>::basic;
   kip_functions(cube);

   // compute_corners
   inline void compute_corners() const
   {
      // rotation to move corners to their actual locations
      const rotate<3,real> move(a.x, a.y, a.z, c);

      // p: the actual corners
      p[0] = move.back(-r, -r, -r);
      p[1] = move.back( r, -r, -r);
      p[2] = move.back( r,  r, -r);
      p[3] = move.back(-r,  r, -r);
      p[4] = move.back(-r, -r,  r);
      p[5] = move.back( r, -r,  r);
      p[6] = move.back( r,  r,  r);
      p[7] = move.back(-r,  r,  r);
   }

   // center, angles, radius (half-size)
   point<real> c;
   point<real> a;
   real r;

   // back()
   inline point<real> back(const point<real> &from) const
      { return rot.back(from); }



   // cube([c[,a[,r[,base]]]])
   inline explicit cube(
      const point<real> &_c = point<real>(real(0), real(0), real(0)),
      const point<real> &_a = point<real>(real(0), real(0), real(0)),
      const real &_r = real(1)
   ) :
      shape<real,tag>(this),
      c(_c), a(_a), r(_r)
   { }

   inline explicit cube(
      const point<real> &_c,
      const point<real> &_a,
      const real &_r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(_c), a(_a), r(_r)
   { }



   // cube(c.x,c.y,c.z[,a.x,a.y,a.z[,r[,base]]])
   inline explicit cube(
      const real &cx, const real &cy, const real &cz
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(real(0), real(0), real(0)),
      r(real(1))
   { }

   inline explicit cube(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(real(1))
   { }

   inline explicit cube(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az,
      const real &_r
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(_r)
   { }

   inline explicit cube(
      const real &cx, const real &cy, const real &cz,
      const real &ax, const real &ay, const real &az,
      const real &_r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(_r)
   { }



   // cube(cube)
   inline cube(const cube &from) :
      shape<real,tag>(from),
      c(from.c),
      a(from.a),
      r(from.r)
   { }

   // cube = cube
   inline cube &operator=(const cube &from)
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

kip_process(cube)
{
   compute_corners();  // computes p[0..7], the actual corner locations

   // rot, rot.h, eye, emr
   // note: rot.h is changed for convenience; original is not otherwise needed

   unsigned ppos = 0;  // index of closest point to eyeball, to be determined
   real pmin = mod2(p[0] - eyeball);

   for (unsigned n = 1;  n < 8;  ++n) {
      const real newmin = mod2(p[n] - eyeball);
      if (newmin < pmin) {
         ppos = n;
         pmin = newmin;
      }
   }

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

   // Theoretical statistically-optimal (for speed) 3-rotation point
   // orderings, given ray check against x=0, y=0, and z=0 planes,
   // in that order.
   static const unsigned order[8][3] = {
      { 0, 4, 5 }, // 0 z [0][4][5] zxy
      { 1, 5, 6 }, // 1 z [1][5][6] zyx
      { 2, 6, 7 }, // 2 z [2][6][7] zxy
      { 3, 7, 4 }, // 3 z [3][7][4] zyx
      { 4, 0, 3 }, // 4 z [4][0][3] zyx
      { 5, 1, 0 }, // 5 z [5][1][0] zxy
      { 6, 2, 1 }, // 6 z [6][2][1] zyx
      { 7, 3, 2 }  // 7 z [7][3][2] zxy
   };

   // rot
   rot = rotate<3,real>(
      p[order[ppos][0]],
      p[order[ppos][1]],
      p[order[ppos][2]]
   );

   // eye, emr, rot.h
   rot.h = r + r;
   basic.eye() = rot.fore(eyeball);
   emr = basic.eye() - point<real>(r,r,r);

   // lie, modr2, interior
   basic.lie() = point<float>(rot.fore(light));
   modr2 = 3*r*r;
   interior = inside(eyeball);

   // minimum distance
   return internal::bbox_minimum(basic.eye(), rot.ex, rot.ey, rot.h);

} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(cube)
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
kip_inside(cube)
{
   return
      0 <= basic.eye().x &&
      0 <= basic.eye().y &&
      0 <= basic.eye().z;
} kip_end



// dry
kip_dry(cube)
{
   return
      seg.ge(p[0]) && seg.ge(p[6]) && seg.ge(p[1]) && seg.ge(p[7]) &&
      seg.ge(p[2]) && seg.ge(p[4]) && seg.ge(p[3]) && seg.ge(p[5]);
} kip_end



// check
kip_check(cube)
{
   static const char *const err = "Cube has non-positive half-size ";
   diagnostic_t rv = diagnostic_t::diagnostic_good;

   if (r <= real(0)) {
      std::ostringstream oss;
      oss << err << "r=" << r;
      rv = error(oss);
   }

   return rv;
} kip_end



// random
kip_random(cube)
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
   obj.r = real(0.15)*random<real>();

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(cube)
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
            q.x = 0;  return q(-1,0,0, this, normalized_t::yesnorm), true;
         }
      } else if (dx > 0 && 0 < (q = (rot.ex - eye.x)/dx) && q < qmin &&
             0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey &&
             0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
         q.x = rot.ex;  return q(1,0,0, this, normalized_t::yesnorm), true;
      }

      if (dy < 0) {
         if (0 < (q = -eye.y/dy) && q < qmin &&
             0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
             0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
            q.y = 0;  return q(0,-1,0, this, normalized_t::yesnorm), true;
         }
      } else if (dy > 0 && 0 < (q = (rot.ey - eye.y)/dy) && q < qmin &&
             0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
             0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
         q.y = rot.ey;  return q(0,1,0, this, normalized_t::yesnorm), true;
      }

      if (dz < 0) {
         if (0 < (q = -eye.z/dz) && q < qmin &&
             0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
             0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey) {
            q.z = 0;  return q(0,0,-1, this, normalized_t::yesnorm), true;
         }
      } else if (dz > 0 && 0 < (q = (rot.h  - eye.z)/dz) && q < qmin &&
             0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
             0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey) {
         q.z = rot.h;  return q(0,0,1, this, normalized_t::yesnorm), true;
      }

      return false;
   }

   // early return if ray misses bounding sphere
   if (mod2(cross(point<real>(dx,dy,dz), emr)) > modr2)
      return false;

   // check the 0-planes
   if (eye.x < 0) {
      if (dx <= 0 || (q = -eye.x/dx) >= qmin) return false;
      if (0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey &&
          0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
         q.x = 0;  return q(-1,0,0, this, normalized_t::yesnorm), true;
      }
   }

   if (eye.y < 0) {
      if (dy <= 0 || (q = -eye.y/dy) >= qmin) return false;
      if (0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
          0 <= (q.z = eye.z + q*dz) && q.z <= rot.h) {
         q.y = 0;  return q(0,-1,0, this, normalized_t::yesnorm), true;
      }
   }

   if (eye.z < 0) {
      if (dz <= 0 || (q = -eye.z/dz) >= qmin) return false;
      if (0 <= (q.x = eye.x + q*dx) && q.x <= rot.ex &&
          0 <= (q.y = eye.y + q*dy) && q.y <= rot.ey) {
         q.z = 0;  return q(0,0,-1, this, normalized_t::yesnorm), true;
      }
   }

   return false;
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(cube)
{
   if (interior) return cube<real,tag>::infirst(
      kip_etd,qmin,ints.one(),insub);

   // Outside...
   const point<real>  tar = rot.fore(target);
   const point<real> &eye = basic.eye();

   const real dx = tar.x - eye.x;
   const real dy = tar.y - eye.y;
   const real dz = tar.z - eye.z;

   // early return if ray misses bounding sphere
   if (mod2(cross(point<real>(dx,dy,dz), emr)) > modr2)
      return false;

   // check each plane...
   ints.convex();
   kip::inq<real,tag> inq;  // try static[nthreads], here and elsewhere

   // x min
   if (0 < (inq.q = -eye.x/dx) && inq.q < qmin &&
       0 <= (inq.y = eye.y + inq.q*dy) && inq.y <= rot.ey &&
       0 <= (inq.z = eye.z + inq.q*dz) && inq.z <= rot.h) {
      inq.x = 0;
      if (ints.convex(inq(-1,0,0, this, normalized_t::yesnorm))) return true;
   }

   // x max
   if (dx > 0 && 0 < (inq.q = (rot.ex - eye.x)/dx) && inq.q < qmin &&
       0 <= (inq.y = eye.y + inq.q*dy) && inq.y <= rot.ey &&
       0 <= (inq.z = eye.z + inq.q*dz) && inq.z <= rot.h) {
      inq.x = rot.ex;
      if (ints.convex(inq(1,0,0, this, normalized_t::yesnorm))) return true;
   }

   // y min
   if (0 < (inq.q = -eye.y/dy) && inq.q < qmin &&
       0 <= (inq.x = eye.x + inq.q*dx) && inq.x <= rot.ex &&
       0 <= (inq.z = eye.z + inq.q*dz) && inq.z <= rot.h) {
      inq.y = 0;
      if (ints.convex(inq(0,-1,0, this, normalized_t::yesnorm))) return true;
   }

   // y max
   if (dy > 0 && 0 < (inq.q = (rot.ey - eye.y)/dy) && inq.q < qmin &&
       0 <= (inq.x = eye.x + inq.q*dx) && inq.x <= rot.ex &&
       0 <= (inq.z = eye.z + inq.q*dz) && inq.z <= rot.h) {
      inq.y = rot.ey;
      if (ints.convex(inq(0,1,0, this, normalized_t::yesnorm))) return true;
   }

   // z min
   if (0 < (inq.q = -eye.z/dz) && inq.q < qmin &&
       0 <= (inq.x = eye.x + inq.q*dx) && inq.x <= rot.ex &&
       0 <= (inq.y = eye.y + inq.q*dy) && inq.y <= rot.ey) {
      inq.z = 0;
      if (ints.convex(inq(0,0,-1, this, normalized_t::yesnorm))) return true;
   }

   // z max
   if (dz > 0 && 0 < (inq.q = (rot.h  - eye.z)/dz) && inq.q < qmin &&
       0 <= (inq.x = eye.x + inq.q*dx) && inq.x <= rot.ex &&
       0 <= (inq.y = eye.y + inq.q*dy) && inq.y <= rot.ey) {
      inq.z = rot.h;
      if (ints.convex(inq(0,0,1, this, normalized_t::yesnorm))) return true;
   }

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(cube) {

   // (
   //    c.x, c.y, c.z,
   //    a.x, a.y, a.z,
   //    r,
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.c, "center") && read_comma(s) &&
      read_value(s, obj.a, "angles") && read_comma(s) &&
      read_value(s, obj.r, "half-size") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic_t::diagnostic_error);
   }

   obj.a *= real(kip_pi)/180;
   return !s.fail();
}



// kip::ostream
kip_ostream(cube) {
   bool okay;

   // stub
   if (kip::format == kip::format_t::format_stub)
      okay = k << "cube()";

   // one
   // op
   else if (kip::format == kip::format_t::format_one ||
            kip::format == kip::format_t::format_op)
      okay = k << "cube("
               <<  obj.c << ", "
               << (180/real(kip_pi))*obj.a << ", "
               <<  obj.r &&
             kip::write_finish(k, obj, true);

   // full
   else
      okay = k << "cube(\n   " &&
             k.indent() <<  obj.c << ",\n   " &&
             k.indent() << (180/real(kip_pi))*obj.a << ",\n   " &&
             k.indent() <<  obj.r &&
             kip::write_finish(k, obj, false);

   kip_ostream_end(cube);
}

}

#define   kip_class cube
#include "kip-macro-io.h"
