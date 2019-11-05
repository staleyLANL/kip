
// -----------------------------------------------------------------------------
// cube
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class cube : public shape<real,tag> {
   bool inside(const point<real> &) const;

   // rotation: clockwise around x, then y, then z; then +c translation
   private: mutable rotate<3,real,op::full,op::unscaled> rot;
   private: mutable point<real> emr;
   public:  mutable point<real> p[8];
   private: mutable real modr2;

public:
   using shape<real,tag>::basic;
   kip_functions(cube);

   // compute_corners
   void compute_corners() const
   {
      // rotation to move corners to their actual locations
      const rotate<3,real,op::full,op::unscaled> move(a.x, a.y, a.z, c);

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
   point<real> back(const point<real> &from) const
      { return rot.back(from); }



   // cube([c[,a[,r[,base]]]])
   explicit cube(
      const point<real> &_c = point<real>(0,0,0),
      const point<real> &_a = point<real>(0,0,0),
      const real _r = 1
   ) :
      shape<real,tag>(this),
      c(_c), a(_a), r(_r)
   { }

   explicit cube(
      const point<real> &_c,
      const point<real> &_a,
      const real _r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(_c), a(_a), r(_r)
   { }



   // cube(c.x,c.y,c.z[,a.x,a.y,a.z[,r[,base]]])
   explicit cube(
      const real cx, const real cy, const real cz
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(0,0,0),
      r(1)
   { }

   explicit cube(
      const real cx, const real cy, const real cz,
      const real ax, const real ay, const real az
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(1)
   { }

   explicit cube(
      const real cx, const real cy, const real cz,
      const real ax, const real ay, const real az,
      const real _r
   ) :
      shape<real,tag>(this),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(_r)
   { }

   explicit cube(
      const real cx, const real cy, const real cz,
      const real ax, const real ay, const real az,
      const real _r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      c(cx,cy,cz),
      a(ax,ay,az),
      r(_r)
   { }



   // cube(cube)
   cube(const cube &from) :
      shape<real,tag>(from),
      c(from.c),
      a(from.a),
      r(from.r)
   { }

   // cube = cube
   cube &operator=(const cube &from)
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
   rot = rotate<3,real,op::full,op::unscaled>(
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
   this->interior = inside(eyeball);

   // minimum distance
   return detail::bbox_minimum(basic.eye(), rot.ex, rot.ey, rot.h);

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
   diagnostic rv = diagnostic::good;

   if (r <= 0) {
      std::ostringstream oss;
      oss << err << "r=" << r;
      rv = error(oss);
   }

   return rv;
} kip_end



// randomize
kip_randomize(cube)
{
   // center
   random_full(obj.c);

   // angles
   obj.a.x = pi<real>*random_unit<real>();
   obj.a.y = pi<real>*random_unit<real>();
   obj.a.z = pi<real>*random_unit<real>();

   // radii
   obj.r = real(0.15)*random_unit<real>();

   // base
   randomize(obj.base());  obj.baseset = true;
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
   if (mod2(cross(point<real>(dx,dy,dz), emr)) > modr2)
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

kip_inall(cube)
{
   if (this->interior)
      return cube<real,tag>::infirst(etd, insub, qmin, ints.one());

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
      addendum("Detected while reading " + description, diagnostic::error);
   }

   obj.a *= pi<real>/180;
   return !s.fail();
}



// kip::ostream
kip_ostream(cube) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "cube()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "cube("
               <<  obj.c << ", "
               << (180/pi<real>)*obj.a << ", "
               <<  obj.r &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << "cube(\n   " &&
             k.indent() <<  obj.c << ",\n   " &&
             k.indent() << (180/pi<real>)*obj.a << ",\n   " &&
             k.indent() <<  obj.r &&
             write_finish(k, obj, false);

   kip_ostream_end(cube);
}

#define   kip_class cube
#include "kip-macro-io.h"
