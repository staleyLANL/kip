
#pragma once

namespace kip {

// ntabular
namespace internal {
   inline size_t &ntabular_function(void)
      { static size_t value = 16;  return value; }
}
namespace { size_t &ntabular = internal::ntabular_function(); }



// -----------------------------------------------------------------------------
// tabular
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class tabular : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   typedef std::vector< point_xr<real> > table_t;

private:
   mutable rotate<3,real> rot;

   // actual begin/end (along a --> b) of tabular
   mutable point<real> atabular, btabular;

   mutable array_simple< internal::rshhhd<real> > pre;
   mutable real hdr, rmaxsq, f, left, right;

private:
   mutable size_t npts;

   // endcaps
   inline bool cap_lo(
      const real, const real, inq<real,tag> &, const real, const real
   ) const;

   inline bool cap_hi(
      const real, const real, inq<real,tag> &, const real, const real,
      const size_t
   ) const;

   // hit_bounding_cylinder, get_bounds
   inline bool hit_bounding_cylinder(
      const real, const real, real &, real &
   ) const;

   inline bool get_bounds(
      const real, const real, const real, const real, const real,
      const size_t, const size_t,
      size_t &, size_t &
   ) const;

   // segment, registerq, segment2
   inline bool segment(
      const point<real> &, const real, inq<real,tag> &,
      const real, const real, const real, const real, const size_t
   ) const;

   inline void registerq(
      const real, const real, const real, const real, const real, const size_t,
      afew< inq<real,tag> > &
   ) const;

   inline void segment2(
      const real, const real, afew< inq<real,tag> > &,
      const real, const real, const real, const real, const size_t
   ) const;

public:
   using shape<real,tag>::basic;

   // point a, point b; these effectively define origin and +x axis
   point<real> a;
   point<real> b;

   // (x,r) coordinates
   table_t table;
   inline size_t size(void) const { return table.size(); }

   kip_functions(tabular);

   inline point<real> back(const point<real> &from) const
      { return rot.back(from); }



   // tabular([a[,b[,(x,r)[,base]]]])
   inline explicit tabular(
      const point<real> &_a = point<real>(real(0),real(0),real(0)),
      const point<real> &_b = point<real>(real(1),real(0),real(0)),
      const table_t &_table = table_t()
   ) :
      shape<real,tag>(this),
      a(_a), b(_b), table(_table)
   { }

   inline explicit tabular(
      const point<real> &_a,
      const point<real> &_b,
      const table_t &_table,
      const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      a(_a), b(_b), table(_table)
   { }



   // tabular([a.x,a.y,a.z[,b.x,b.y,b.z[,(x,r)[,base]]]])
   inline explicit tabular(
      const real &ax, const real &ay, const real &az
   ) :
      shape<real,tag>(this), a(ax,ay,az), b(1,0,0) { }

   /*
   inline explicit tabular(
      const real &ax, const real &ay, const real &az,
      const real &bx, const real &by, const real &bz
   ) :
      shape<real,tag>(this), a(ax,ay,az), b(bx,by,bz) { }
   */

   inline explicit tabular(
      const real &ax, const real &ay, const real &az,
      const real &bx, const real &by, const real &bz,
      const table_t &_table = table_t()
   ) :
      shape<real,tag>(this), a(ax,ay,az), b(bx,by,bz), table(_table) { }

   inline explicit tabular(
      const real &ax, const real &ay, const real &az,
      const real &bx, const real &by, const real &bz,
      const table_t &_table, const tag &thebase
   ) :
      shape<real,tag>(this,thebase), a(ax,ay,az), b(bx,by,bz), table(_table) { }



   // tabular(a.x,a.y,a.z,b.x,b.y,b.z,base)
   inline explicit tabular(
      const real &ax, const real &ay, const real &az,
      const real &bx, const real &by, const real &bz, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      a(ax,ay,az), b(bx,by,bz)
   { }



   // tabular(tabular)
   inline tabular(const tabular &from) :
      shape<real,tag>(from),
      a(from.a),
      b(from.b), table(from.table)
   { }

   // tabular = tabular
   inline tabular &operator=(const tabular &from)
   {
      this->shape<real,tag>::operator=(from);
      a = from.a;
      b = from.b;  table = from.table;
      return *this;
   }



   // push
   inline point_xr<real> &push(const real &x, const real &r)
   {
      return table.push_back(point_xr<real>(x,r)), table.back();
   }
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(tabular)
{
   rot = rotate<3,real>(a, b, eyeball);
   basic.eye()(rot.ex, rot.ey, 0);
   basic.lie() = point<float>(rot.fore(light));

   npts = size();

   // true positions of end-centers of tabular
   atabular = rot.back_n00(table[0].x);
   btabular = rot.back_n00(table[npts-1].x);

   pre.resize(npts);  // in fact, rsq needs npts; slope...dmins only need npts-1

   left  = table[0     ].x - rot.ex;
   right = table[npts-1].x - rot.ex;

   // For the rotated and translated tabular, the first segment's lower
   // x coordinate is table[0].x (which can be -, 0, or +), and the last
   // segment's upper x coordinate is table[npts-1].x. There are npts
   // points, ([0]..[npts-1]), and npts-1 segments, ([0]..[npts-2]).

   // Example: npts=6 (therefore: 6 points, 5 segments)
   //
   //                 r
   //                 |
   //                 |         *
   //                 |         |  *
   // table[0].r      |    *    |  |             *
   // table[npts-1].r |    |    |  |             |    *
   //                 |    |    |  |      *      |    |
   //                 +-- #0   #1 #2     #3     #4   #5 ----- x
   //                      |                          |
   //                      |                          |
   //                  table[0].x              table[npts-1].x

   // We'll compute rsq for each point, slope..dmins for each segment.

   pre[0].rsq = op::sq(table[0].r);
   real rmax = table[0].r;  // initially

   real rv = std::numeric_limits<real>::max();  // initially

   interior = false;  // possibly modified to true below
   for (unsigned i = 1;  i < npts;  ++i) {
      const point_xr<real>
         &last = table[i-1],  // point to the left  of segment
         &next = table[i  ];  // point to the right of segment
      internal::rshhhd<real> &seg = pre[i-1];  // segment

      // Computations for point...
      pre[i].rsq = op::sq(next.r);
      if (next.r > rmax) rmax = next.r;  // possibly update rmax

      // Computations for segment...

      // like with bicylinder; note that h2 and h3 have "-last.x" shift
      seg.slope = (next.r-last.r)/(next.x-last.x);
      const real val = last.r + seg.slope*(rot.ex - last.x);

      seg.h1 = op::sq(seg.slope) + 1;
      seg.h2 = seg.slope * val;
      seg.h3 = (rot.ey - val)*(rot.ey + val);

      // dmins = minimum distance from eyeball for this segment
      seg.dmins =
          rot.ex <= last.x && rot.ey <= last.r
       ?  last.x - rot.ex  // west
       :  rot.ex >= next.x && rot.ey <= next.r
       ?  rot.ex - next.x  // east
       :  last.x <= rot.ex && rot.ex <= next.x && seg.h3 <= 0
       ?  (interior = true, op::min(  // inside
             rot.ex - last.x,
             next.x - rot.ex,
             op::abs(seg.slope*(rot.ex-last.x)+last.r-rot.ey)/op::sqrt(seg.h1)
          ))
       : (last.r - next.r)*(rot.ey - last.r) >=
         (next.x - last.x)*(rot.ex - last.x)
       ?  op::sqrt(op::sq(rot.ex - last.x) + op::sq(rot.ey - last.r))  // nw
       : (last.r - next.r)*(rot.ey - next.r) <=
         (next.x - last.x)*(rot.ex - next.x)
       ?  op::sqrt(op::sq(rot.ex - next.x) + op::sq(rot.ey - next.r))  // ne
       :  op::abs(seg.slope*(rot.ex - last.x) +
             last.r - rot.ey) / op::sqrt(seg.h1)  // north
      ;

      // possibly update minimum
      if (seg.dmins < rv) rv = seg.dmins;
   }

   // miscellaneous
   rmaxsq = rmax*rmax;
   rot.h = table[npts-1].x - table[0].x;  // "h" of whole tabular
   hdr = rot.h/rmax;
   const real tmp = rot.ey/rmax;
   f = (1 + tmp)*(1 - tmp);

   return rv;
} kip_end



// -----------------------------------------------------------------------------
// aabb, dry
// -----------------------------------------------------------------------------

// aabb
kip_aabb(tabular)
{
   if ((npts = size()) == 0)
      // make out-of-order (thus !valid()) in each dimension
      return bbox<real>(false,1,0,false, false,1,0,false, false,1,0,false);

   const rotate<2,real> r2(a,b);

   const point<real> diff(
      op::sqrt(r2.m2x*r2.m2x + r2.m3.x*r2.m3.x),
      op::sqrt(r2.m2y*r2.m2y + r2.m3.y*r2.m3.y), r2.m3.z
   ), along = normalize(b-a);

   real xmin =  std::numeric_limits<real>::max(), ymin = xmin, zmin = xmin;
   real xmax = -std::numeric_limits<real>::max(), ymax = xmax, zmax = xmax;

   for (unsigned i = 0;  i < npts;  ++i) {
      const point<real> cent = a + table[i].x*along, away = table[i].r * diff;

      const point<real> min_i = cent - away;
      const point<real> max_i = cent + away;

      xmin = op::min(xmin, min_i.x),   xmax = op::max(xmax, max_i.x);
      ymin = op::min(ymin, min_i.y),   ymax = op::max(ymax, max_i.y);
      zmin = op::min(zmin, min_i.z),   zmax = op::max(zmax, max_i.z);
   }

   return bbox<real>(
      true,xmin, xmax,true,
      true,ymin, ymax,true,
      true,zmin, zmax,true
   );
} kip_end



// dry
kip_dry(tabular)
{
   // Bounding cylinder version seems to be faster than full version, probably
   // because it's so much simpler --- and lots of culling happens in infirst()
   real az;  if (seg.lt(atabular,az)) return false;
   real bz;  if (seg.lt(btabular,bz)) return false;
   return op::sq(hdr*(op::min(az,bz)-seg.c)) >= (rot.h-az+bz)*(rot.h+az-bz);
} kip_end



// -----------------------------------------------------------------------------
// cap_lo
// cap_hi
// -----------------------------------------------------------------------------

// cap_lo
template<class real, class tag>
inline bool tabular<real,tag>::cap_lo(
   const real tar_z, const real qmin, inq<real,tag> &q,
   const real dx, const real dy
) const {
   if (dx == 0) return false;

   q = left/dx;
   if (!(0 < q && q < qmin)) return false;

   q.y = rot.ey + q*dy;
   q.z = q*tar_z;

   return q.y*q.y + q.z*q.z <= pre[0].rsq
      ? q.x = table[0].x, q(-1,0,0, this, yesnorm), true
      : false;
}



// cap_hi
template<class real, class tag>
inline bool tabular<real,tag>::cap_hi(
   const real tar_z, const real qmin, inq<real,tag> &q,
   const real dx, const real dy, const size_t P
) const {
   if (dx == 0) return false;

   q = right/dx;
   if (!(0 < q && q < qmin)) return false;

   q.y = rot.ey + q*dy;
   q.z = q*tar_z;

   return q.y*q.y + q.z*q.z <= pre[P].rsq
      ? q.x = table[P].x, q( 1,0,0, this, yesnorm), true
      : false;
}



// -----------------------------------------------------------------------------
// hit_bounding_cylinder
// get_bounds
// -----------------------------------------------------------------------------

// hit_bounding_cylinder
template<class real, class tag>
inline bool tabular<real,tag>::hit_bounding_cylinder(
   const real dy, const real tar_z,  // input
   real &s, real &den  // output
) const {
   const real c = dy*dy, h = tar_z*tar_z;
   return (s = c + h*f) >= 0 && (s *= rmaxsq, den = c+h, true);
}



// get_bounds
template<class real, class tag>
inline bool tabular<real,tag>::get_bounds(
   const real s, const real den, const real eydy, const real dx, const real qmin,
   const size_t P, const size_t C,
   size_t &cmin, size_t &cmax
) const {
   // Begin by determining xmin, xmax of ray intersection with bounding cylinder
   real xmin, xmax;

   if (den != 0) {
      const real val = op::sqrt(s);

      // by construction, this q is the smaller one...
      const real qsmall = (-val - eydy)/den;
      if (qsmall >= qmin) return false;  // smaller is already too large
      xmin = rot.ex + op::max(qsmall,real(0))*dx;

      // by construction, this q is the larger one...
      const real qlarge = ( val - eydy)/den;
      if (qlarge <= 0) return false;  // larger is already too small
      xmax = rot.ex + op::min(qlarge,qmin)*dx;

      if (xmin > xmax) std::swap(xmin,xmax);  // put x's in order
      if (xmin > table[P].x || xmax < table[0].x) return false;

   } else {
      xmin = table[0].x;
      xmax = table[P].x;
      // zzz perhaps want: if (xmin > xmax) std::swap(xmin,xmax);
   }

   // Determine the starting and ending "cell-wise" indices cmin and cmax
   // for examining the curved portion. We want:
   //    cmin = minimum i for which table[i+1].x >= xmin,
   //    cmax = maximum i for which table[i  ].x <= xmax.  Note: ], not )
   // In principle, the following would be faster if we allowed for good
   // initial guesses for cmin and cmax, but my tests indicate that this
   // code accounts for under 1% of the total run-time, so I won't worry
   // about it.

   // cmin
   cmin = 0;
   if (table[1].x < xmin) {
      cmin = 1;  size_t hi = C;
      do {
         const size_t mid = (cmin + hi)/2 + 1;
         if (table[mid].x < xmin) cmin = mid; else hi = mid-1;
      } while (hi > cmin);
   }

   // cmax
   cmax = C;
   if (table[C].x > xmax) {  // [C] or [P-1]
      cmax = cmin;  size_t hi = C-1;
      do {
         const size_t mid = (cmax + hi)/2 + 1;
         if (table[mid].x > xmax) hi = mid-1; else cmax = mid;
      } while (hi > cmax);
   }

   return true;
}



// -----------------------------------------------------------------------------
// segment
// segment2
// -----------------------------------------------------------------------------

// segment
template<class real, class tag>
inline bool tabular<real,tag>::segment(
   const point<real> &tar, const real qmin, inq<real,tag> &q,
   const real dx, const real dy, const real tmp1, const real tmp2,
   const size_t i
) const {
   // We're dealing with cell i, which is between points table[i] and table[i+1]
   const internal::rshhhd<real> &cell = pre[i];

   if (cell.dmins < qmin) {
      const real m = 1 - tmp1*cell.h1;
      const real g = dx*cell.h2 - tmp2;
      const real s = g*g - m*cell.h3;
      if (s < 0 || m == 0) return false;

      // q
      q = interior ? (g + op::sqrt(s))/m : (g - op::sqrt(s))/m;
      if (!(0 < q && q < qmin)) return false;

      q.x = rot.ex + q*dx;
      if (table[i].x <= q.x && q.x <= table[i+1].x)
         return q(
            cell.slope*(cell.slope*(table[i].x-q.x) - table[i].r),
            q.y = rot.ey + q*dy,
            q.z = q*tar.z,
            this, nonorm
         ), true;
   }
   return false;
}



// registerq
template<class real, class tag>
inline void tabular<real,tag>::registerq(
   const real q, const real dx, const real dy,
   const real tar_z, const real slope, const size_t i,
   afew< inq<real,tag> > &ints
) const {
   inq<real,tag> qtmp;
   qtmp.x = rot.ex + q*dx;

   if (table[i].x <= qtmp.x && qtmp.x <= table[i+1].x)
      ints.push(
         qtmp(
            slope*(slope*(table[i].x - qtmp.x) - table[i].r),
            qtmp.y = rot.ey + q*dy,
            qtmp.z = q*tar_z,
            this, nonorm
         ) = q
      );
}



// segment2
template<class real, class tag>
inline void tabular<real,tag>::segment2(
   const real tar_z, const real qmin, afew< inq<real,tag> > &ints,
   const real dx, const real dy, const real tmp1, const real tmp2,
   const size_t i
) const {
   // We're dealing with cell i, which is between points table[i] and table[i+1]
   const internal::rshhhd<real> &cell = pre[i];

   if (cell.dmins < qmin) {
      const real m = 1 - tmp1*cell.h1;
      const real g = dx*cell.h2 - tmp2;
      const real s = g*g - m*cell.h3;
      if (s < 0 || m == 0) return;

      const real root = op::sqrt(s);
      real q1 = (g - root)/m;
      real q2 = (g + root)/m;  if (q2 < q1) std::swap(q2,q1);

      if (0 < q1 && q1 < qmin) registerq(q1, dx,dy, tar_z,cell.slope, i,ints);
      if (0 < q2 && q2 < qmin) registerq(q2, dx,dy, tar_z,cell.slope, i,ints);
   }
}



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(tabular)
{
   const point<real> tar = rot.fore(target);
   const real dx = tar.x - rot.ex;
   const real dy = tar.y - rot.ey;

   const size_t P = npts-1;  // last valid point
   const size_t C = npts-2;  // last valid cell

   // If outside, examine end caps
   if (left > 0) {
      // x = table[0].x
      if (dx <= 0) return false;
      if (cap_lo(tar.z,qmin,q,dx,dy  )) return true;
      if (!(q < qmin)) return false;
   } else if (right < 0) {
      // x = table[P].x
      if (dx >= 0) return false;
      if (cap_hi(tar.z,qmin,q,dx,dy,P)) return true;
      if (!(q < qmin)) return false;
   }

   // Examine bounding cylinder
   real s, den;
   if (!hit_bounding_cylinder(dy,tar.z, s,den)) return false;

   // Determine starting and ending cell-wise indices
   size_t cmin, cmax;
   if (!get_bounds(s,den, rot.ey*dy,dx, qmin, P,C, cmin,cmax)) return false;

   // Examine curved portion
   const real tmp1 = dx*dx, tmp2 = dy*rot.ey;
   if (dx > 0)
      for (size_t i = cmin;  i <= cmax;  ++i) {
         if (segment(tar,qmin, q, dx,dy, tmp1,tmp2, i)) return true;
      }
   else
      for (size_t i = cmax;  i >= cmin;  --i)
         if (segment(tar,qmin, q, dx,dy, tmp1,tmp2, i)) return true; else
         if (i == 0) break;  // so we can't wrap around to maximum unsigned

   // If inside, examine end caps
   return interior && (
      (dx < 0 && cap_lo(tar.z,qmin,q,dx,dy  )) ||
      (dx > 0 && cap_hi(tar.z,qmin,q,dx,dy,P)));
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(tabular)
{
   const real dx = rot.forex(target) - rot.ex;
   if ((left > 0 && dx <= 0) || (right < 0 && dx >= 0)) return false;
   const real dy = rot.forey(target) - rot.ey;

   // Examine bounding cylinder
   real s, den;  const real tar_z = rot.forez(target);
   if (!hit_bounding_cylinder(dy, tar_z, s,den)) return false;

   const size_t P = npts-1;  // last valid point
   const size_t C = npts-2;  // last valid cell

   // Determine starting and ending cell-wise indices
   size_t cmin, cmax;
   if (!get_bounds(s,den, rot.ey*dy,dx, qmin, P,C, cmin,cmax)) return false;

   // Examine "0" endcap, curved portions "c", and "h" endcap, as appropriate,
   // and in the appropriate order
   const real tmp1 = dx*dx, tmp2 = dy*rot.ey;
   inq<real,tag> qtmp;
   ints.reset();

   if (dx < 0) {
      // Check h, c, 0
      if (cap_hi(tar_z,qmin,ints[0],dx,dy,P)) ints.setsize(1);  // h
      for (size_t i = cmax;  i >= cmin;  --i) {
         segment2(tar_z,qmin, ints, dx,dy, tmp1,tmp2, i);       // c
         if (i == 0) break;  // so we can't wrap around to maximum unsigned
      }
      if (cap_lo(tar_z,qmin,qtmp,   dx,dy  )) ints.push(qtmp);  // 0

   } else if (dx > 0) {
      // Check 0, c, h
      if (cap_lo(tar_z,qmin,ints[0],dx,dy  )) ints.setsize(1);  // 0
      for (size_t i = cmin;  i <= cmax;  ++i)
         segment2(tar_z,qmin, ints, dx,dy, tmp1,tmp2, i);       // c
      if (cap_hi(tar_z,qmin,qtmp,   dx,dy,P)) ints.push(qtmp);  // h

   } else
      // Check c
      segment2(tar_z,qmin, ints, dx,dy, tmp1,tmp2, cmin);       // c

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// check
// -----------------------------------------------------------------------------

kip_check(tabular)
{
   diagnostic_t rv = diagnostic_good;
   npts = size();

   // npts
   if (npts < 2)
      // must have at least two points
      rv = error("tabular has number of points < 2");

   // points
   for (size_t i = 0;  i < npts;  ++i)
      // radii must be nonnegative
      if (table[i].r < real(0)) {
         std::ostringstream oss;
         oss << "tabular has negative radius table[" << i
             << "].r == " << table[i].r;
         rv = error(oss);
      }

   // segments
   for (size_t i = 1;  i < npts;  ++i) {
      const point_xr<real>
         &last = table[i-1],  // point to the left
         &next = table[i  ];  // point to the right

      // x values must be strictly increasing
      if (!(last.x < next.x)) {
         std::ostringstream oss;
         oss << "tabular has non-increasing x values: "
            "table[" << i-1 << "].x == " << last.x << ", "
            "table[" << i   << "].x == " << next.x;
         rv = error(oss);
      }

      // cannot have two successive zero radii
      // note: we could relax this condition
      if (last.r == real(0) && next.r == real(0)) {
         std::ostringstream oss;
         oss << "tabular has successive zero radii: "
            "table[" << i-1 << "].r == " << last.r << ", "
            "table[" << i   << "].r == " << next.r;
         rv = error(oss);
      }
   }

   // require a != b
   if (a == b) {
      std::ostringstream oss;
      oss << "tabular has coincident points a == b == " << a;
      rv = error(oss);
   }

   return rv;
} kip_end



// -----------------------------------------------------------------------------
// random
// -----------------------------------------------------------------------------

kip_random(tabular)
{
   // number of segments (+1 == number of (x,r) points)
   const size_t nseg = (kip::ntabular < 2)
      ? 1
      : 1 + size_t(real(kip::ntabular)*random<real>());
   const size_t npts = nseg+1;

   // a
   obj.a.x = op::twice(random<real>() - real(0.5));
   obj.a.y = op::twice(random<real>() - real(0.5));
   obj.a.z = op::twice(random<real>() - real(0.5));

   // b
   obj.b.x = obj.a.x + random<real>() - real(0.5);
   obj.b.y = obj.a.y + random<real>() - real(0.5);
   obj.b.z = obj.a.z + random<real>() - real(0.5);

   // table
   obj.table.clear();
   obj.table.reserve(npts);

   for (size_t i = 0;  i < npts;  ++i) {
      point_xr<real> p;

      p.x = i == 0
         ? real(0.3)*random<real>()/nseg
         : real(0.3)*random<real>()/nseg + obj.table[i-1].x;
      p.r = real(0.15)*random<real>();
      /*
      do
         p.r = i == 0
            ? real(0.15)*random<real>()
            : obj.table[i-1].r + real(0.03)*(random<real>() - real(0.5));
      while (p.r <= 0);
      */

      obj.table.push_back(p);
   }

   // tag
   random(obj.base());  obj.baseset = true;
   return obj;

} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(tabular) {

   // (
   //    a.x, a.y, a.z,
   //    b.x, b.y, b.z,
   //    npts,
   //    x, r,   // point 0
   //    x, r,   // point 1
   //    ...
   //    x, r,   // point npts-1
   //    base
   // )

   obj.table.clear();
   s.bail = false;
   unsigned n = 0;

   bool okay =
      read_left(s) &&
      read_value(s, obj.a, "\"a\" endpoint") && read_comma(s) &&
      read_value(s, obj.b, "\"b\" endpoint") && read_comma(s) &&
      read_value(s, n);  // don't necessarily read comma; n == 0 is possible

   if (okay && n) {
      obj.table.reserve(n);
      point_xr<real> p;

      for (unsigned i = 0;  i < n && okay;  ++i)
         if ((okay =
              read_comma(s) && read_value(s, p.x) &&
              read_comma(s) && read_value(s, p.r)))
            obj.table.push_back(p);
   }

   if (!(okay && read_done(s, obj))) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic_error);
   }
   return !s.fail();
}



namespace internal {

// tabular_write
template<class real, class tag, template<class,class> class OBJ>
kip::ostream &tabular_write(
   kip::ostream &k, const OBJ<real,tag> &obj,
   const char *const name
) {
   const size_t npts = obj.size();
   bool okay;

   // stub
   if (kip::format == kip::format_stub)
      okay = k << name << "()";

   // one
   // op
   else if (kip::format == kip::format_one ||
            kip::format == kip::format_op) {
      okay = k << name << '(' <<  obj.a << ", " <<  obj.b << ", " <<  npts;
      for (unsigned i = 0;  i < npts && okay;  ++i)
         okay = k << ", " << obj.table[i].x << ',' << obj.table[i].r;
      okay = okay && kip::write_finish(k, obj, true);
   }

   // full
   else {
      okay = k << name << "(\n   " &&
             k.indent() << obj.a << ",\n   " &&
             k.indent() << obj.b << ",\n   " &&
             k.indent() << npts;
      for (unsigned i = 0;  i < npts && okay;  ++i)
         okay = k << ",\n   " &&
                k.indent() << obj.table[i].x << ',' << obj.table[i].r;
      okay = okay && kip::write_finish(k, obj, false);
   }

   // done
   if (!okay)
      (void)kip::error(std::string("Could not write ") + name);
   return k;
}

}



// kip::ostream
kip_ostream(tabular) {
   return kip::internal::tabular_write(k, obj, "tabular");
}

}

#define   kip_class tabular
#include "kip-macro-io.h"