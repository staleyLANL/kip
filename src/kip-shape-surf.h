
// -----------------------------------------------------------------------------
// Helper constructs
// -----------------------------------------------------------------------------

namespace detail {

// min_and_part
template<class SHAPE, class real>
class min_and_part {
public:
   // data
   /*
zzz
see if removing const has any speed effect
see notes file
the point is to see if we can consolidate some things
should vet and check in code soon
been doing lots of work
   */
   /*const*/ SHAPE *ptr;
   real min;

   // min_and_part(min,ptr)
   ///   template<class real>
   explicit min_and_part(const real _min, /*const*/ SHAPE &obj) :
      ptr(&obj), min(_min)
   { }
};

// part_less
template<class SHAPE, class real>
class part_less {
public:
   // for min_and_part
   bool operator()(
      const min_and_part<SHAPE,real> &a,
      const min_and_part<SHAPE,real> &b
   ) const {
      return a.min < b.min;
   }
};

} // namespace detail



// binner
template<class T>
class binner {
   array<1,std::vector<T>> linear;

public:

   // Each surf that's an *operand* contains its own binner. Some bins of some
   // of these surfs may not be reached during a ray trace. Therefore, although
   // each global bin is, obviously, reached during any ray trace, we can defer
   // sorting the corresponding private bins of each surf's binner until+unless
   // that surf is reached in that bin. This deferral does seem to give a small
   // but meaningful speed increase. The following vector<bool> tells us, for
   // each bin of a particular binner, whether or not the bin has been sorted.
   std::vector<bool> sorted;

   // reset
   template<class E>
   void reset(const E &engine)
   {
      // clear
      for (ulong bin = linear.size();  bin--; )  // for each bin
         linear[bin].clear();

      // resize (not upsize) can necessitate fewer above clear()s in future
      // calls, and is a no-op except on the first call, or after someone
      // has changed engine's hzone and/or vzone, which is probably uncommon.
      // And because resize() is typically a no-op, clear()s were necessary
      // so that the previous bin contents aren't there.
      const unsigned num_zone = engine.hzone * engine.vzone;
      linear.resize(num_zone);

      // sorted
      sorted.assign(num_zone,false);
   }

   // operator[]
   std::vector<T> &operator[](const ulong i) { return linear(i); }
};



// -----------------------------------------------------------------------------
// surf
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class surf : public shape<real,tag> {

   // inside()
   bool inside(const point<real> &) const;

   // bounding box
   mutable real xmin, ymin, zmin;
   mutable real xmax, ymax, zmax;

   // mint
   // minimum eyeball-to-tri distances, and (to-be-)depth-sorted tri access
   mutable binner<detail::min_and_part<kip::tri<real,tag>,real>> mint;

public:

   // degenerate, back(), some functions
   using shape<real,tag>::degenerate;
   point<real> back(const point<real> &from) const { return from; }
   kip_functions(surf);

   // tri_t
   using tri_t = kip::tri<real,tag>;

   // nodes and tris
   std::vector<point<real>> node;
   mutable std::vector<tri_t> tri;
   mutable std::vector<bool> used;


   // ------------------------
   // Push
   // ------------------------

   // node
   point<real> &push(const point<real> &n)
   {
      return node.push_back(n), node.back();
   }

   // tri
   tri_t &push(const tri_t &t)
   {
      return tri.push_back(t), tri.back();
   }


   // ------------------------
   // Constructors
   // ------------------------

   // surf()
   explicit surf() :
      shape<real,tag>(this)
   {
      this->eyelie = false;
   }

   // surf(tag)
   explicit surf(const tag &thetag) :
      shape<real,tag>(this,thetag)
   {
      this->eyelie = false;
   }

   // surf(node, tri)
   explicit surf(
      const std::vector<point<real>> &_node, const std::vector<tri_t> &_tri
   ) :
      shape<real,tag>(this),
      node(_node), tri(_tri)
   {
      this->eyelie = false;
   }

   // surf(node, tri, tag)
   explicit surf(
      const std::vector<point<real>> &_node, const std::vector<tri_t> &_tri,
      const tag &thetag
   ) :
      shape<real,tag>(this,thetag),
      node(_node), tri(_tri)
   {
      this->eyelie = false;
   }

   // surf(surf)
   surf(const surf &from) :
      shape<real,tag>(from), node(from.node), tri(from.tri)
   {
   }


   // ------------------------
   // Assignment
   // ------------------------

   // surf=surf
   surf &operator=(const surf &from)
   {
      this->shape<real,tag>::operator=(from);
      node = from.node;
      tri  = from.tri;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// forward: uprepare_tri
namespace detail {
   template<class element_t, class real, class base, class BINS>
   inline real uprepare_tri(
      const engine<real> &, const vars<real,base> &,
      BINS &, const surf<real,base> &, const bool
   );
}



// process
kip_process(surf)
{
   // xmin,ymin,zmin, xmax,ymax,zmax, degenerate
   surf<real,tag>::aabb();

   // interior
   this->interior = surf<real,tag>::inside(eyeball);

   // ----------------
   // For global surf
   // ----------------

   // minimum needed iff surf is operand
   if (!this->isoperand) return 0;

   // ----------------
   // For operand surf
   // ----------------

   // process the tris
   mint.reset(engine);
   return detail::uprepare_tri<
      detail::min_and_part<kip::tri<real,tag>,real>
   >(
      engine,
      vars,
      mint,
      *this,
      false  // "object border", false from here
   );
   // zzz
   // Note, 2013-11-17: I'm confused as to the interaction of this uprepare_tri
   // call versus the one called from uprepare_surf elsewhere. In principle, it
   // doesn't seem like this call should be done here. Were we attempting to
   // reuse code? Is everything done by uprepare_tri really needed here? It's
   // confusing, in retrospect, how surf/tri was fit into a system that really
   // wasn't designed to handle one object (surf) made up of another (tri). We
   // should think about how to clean up this whole system. I like having surfs,
   // which are regarded as solid (unlike an unrelated set of tri objects), and
   // which can work naturally with our unary, binary, and nary operators. But,
   // still, we should figure out precisely what we were doing/thinking.

} kip_end



// aabb
kip_aabb(surf)
{
   // bookkeeping
   const ulong ntri = tri.size();
   if ((degenerate = ntri == 0))
      return bbox<real>(false,1,0,false, false,1,0,false, false,1,0,false);

   // identify nodes that are used by at least one triangle
   const ulong nnode = node.size();
   assert(nnode > 0); // or ntri (see above) can't legitimately be != 0

   used.assign(nnode,false);
   for (ulong t = 0;  t < ntri;  ++t)
      used[tri[t].u] = used[tri[t].v] = used[tri[t].w] = true;

   // compute bounds, based on identified nodes
   ulong n = 0;

   for ( ; n < nnode;  ++n)
      if (used[n]) {
         xmin = xmax = node[n].x;
         ymin = ymax = node[n].y;
         zmin = zmax = node[n].z;
         break;
      }

   n++;
   for ( ; n < nnode;  ++n)
      if (used[n]) {
         const real x = node[n].x;
         const real y = node[n].y;
         const real z = node[n].z;
         if (x < xmin) xmin = x; else if (x > xmax) xmax = x;
         if (y < ymin) ymin = y; else if (y > ymax) ymax = y;
         if (z < zmin) zmin = z; else if (z > zmax) zmax = z;
      }

   // done
   return bbox<real>(
      true,xmin, xmax,true,
      true,ymin, ymax,true,
      true,zmin, zmax,true
   );
} kip_end



// inside
kip_inside(surf)
{
   // inside is definitely false if the eyeball is outside of the surf's
   // bounding box, or if the surf is degenerate
   if (eyeball.x < xmin || xmax < eyeball.x ||
       eyeball.y < ymin || ymax < eyeball.y ||
       eyeball.z < zmin || zmax < eyeball.z || degenerate)
      return false;

   // zzz Must compute for general eyeball position. Be careful; when inside()
   // is called from process() for a global (non-operand) surf, we haven't done
   // tri.process() for the tris yet! So, be sure we don't assume preprocessed
   // tris in this computation (or preprocess them before coming here).
   return false;
} kip_end



// dry
kip_dry(surf)
{
   // degenerate?
   if (degenerate) return true;

   // check bounding box
   if (seg.ge(point<real>(xmin,ymin,zmin)) &&
       seg.ge(point<real>(xmax,ymax,zmax)) &&
       seg.ge(point<real>(xmin,ymin,zmax)) &&
       seg.ge(point<real>(xmax,ymax,zmin)) &&
       seg.ge(point<real>(xmin,ymax,zmin)) &&
       seg.ge(point<real>(xmax,ymin,zmax)) &&
       seg.ge(point<real>(xmin,ymax,zmax)) &&
       seg.ge(point<real>(xmax,ymin,zmin)))
      return true;

   // check the points
   // Note: we could also require used[n] in the conditional, but in the normal
   // case of surfs that don't have extraneous nodes, doing so just wastes time.
   for (ulong n = node.size();  n--; )
      if (seg.lt(node[n]))
         return false;
   return true;
} kip_end



// check
kip_check(surf)
{
   const ulong nnode = node.size();
   const ulong ntri  = tri .size();
   diagnostic rv = diagnostic::good;

   // Check each tri...
   for (ulong t = 0;  t < ntri;  ++t) {
      const ulong u = tri[t].u;
      const ulong v = tri[t].v;
      const ulong w = tri[t].w;


      // Require: u < nnode,  v < nnode,  w < nnode
      #define kip_surf_check(c)\
         if (!(c < nnode)) {\
            std::ostringstream oss;\
            oss << "Surf has tri[" << t << "]." #c "() == " << c\
                << " >= #nodes == " << nnode;\
            rv = error(oss);\
         }
      kip_surf_check(u)
      kip_surf_check(v)
      kip_surf_check(w)
      #undef kip_surf_check


      // Require: u != v,  u != w,  v != w
      #define kip_surf_check(a,b)\
         if (a == b) {\
            std::ostringstream oss;\
            oss << "Surf's tri has coincident vertices: tri[" << t\
                << "]." #a "() == tri[" << t << "]." #b "() == " << a;\
            rv = op::min(rv, warning(oss));\
         }
      kip_surf_check(u, v)
      kip_surf_check(u, w)
      kip_surf_check(v, w)
      #undef kip_surf_check


      // Require: node[u] != node[v], node[u] != node[w], and
      // node[v] != node[w]. But don't bother if u == v, u == w,
      // or v == w, respectively, as these were already diagnosed.
      #define kip_surf_check(a,b)\
         if (a != b && node[a] == node[b]) {\
            std::ostringstream oss;\
            oss << "Surf's tri has coincident vertices: node[tri[" << t\
                << "]." #a "()] == node[tri[" << t << "]." #b "()] == " << a;\
            rv = op::min(rv, warning(oss));\
         }
      kip_surf_check(u, v)
      kip_surf_check(u, w)
      kip_surf_check(v, w)
      #undef kip_surf_check
   }

   return rv;
} kip_end



// randomize
kip_randomize(surf)
{
   using tri_t = typename surf<real,tag>::tri_t;
   const ulong nsurf = 6;

   obj.node.clear();
   obj.tri .clear();

   // Center
   point<real> center;
   random_full(center);

   // Radius
   const real r = real(0.15)*random_unit<real>();

   // Number of longitude and latitude segments
   const unsigned
      nlong = 3+unsigned(real(nsurf)*random_unit<real>()),  // want >= 3
      nlat  = 2+unsigned(real(nsurf)*random_unit<real>());  // want >= 2

   // Nodes
   const unsigned nnode = nlong*(nlat-1) + 2;   // 2 is for poles
   obj.node.reserve(nnode);

   obj.push(point<real>(center.x,center.y,center.z-r));  // node [0]: south pole
   obj.push(point<real>(center.x,center.y,center.z+r));  // node [1]: north pole

   const real dt = op::twice(pi<real>/real(nlong));
   const real dp = pi<real>/real(nlat);

   for (unsigned j = 1;  j < nlat ;  ++j) {
      const real phi   = pi<real>-real(j)*dp;
   for (unsigned i = 0;  i < nlong;  ++i) {
      const real theta = real(i)*dt;
      obj.push(center + r*point<real>(
         std::cos(theta)*std::sin(phi),
         std::sin(theta)*std::sin(phi), std::cos(phi)
      ));
   }}

   // Tris
   const unsigned ntri = 2*nlong*(nlat-1), start = nnode - nlong;
   obj.tri.reserve(ntri);

   for (unsigned i = 0;  i < nlong;  ++i) {
      obj.push(tri_t( 0, 2+(i+1) % nlong, i+2 ));          // off south
      obj.push(tri_t( 1, start+i, start+(i+1) % nlong ));  // off north

      for (unsigned j = 2;  j < nlat;  ++j) {
         const unsigned a = (j-2)*nlong + 2 + i;
         const unsigned b = (j-2)*nlong + 2 +(i+1) % nlong;
         obj.push(tri_t(b, a+nlong, a));
         obj.push(tri_t(b, b+nlong, a+nlong));
      }
   }

   // Tag
   randomize(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// in*
// -----------------------------------------------------------------------------

// infirst
kip_infirst(surf)
{
   // bookkeeping
   std::vector<detail::min_and_part<kip::tri<real,tag>,real>> &bin =
      mint[insub.nzone];
   const ulong ntri = bin.size();  if (ntri == 0) return false;

   // depth-sort bin, if necessary
   if (!mint.sorted[insub.nzone]) {
      std::sort(bin.begin(), bin.end(),
                detail::part_less<kip::tri<real,tag>,real>());
      mint.sorted[insub.nzone] = true;
   }

   // examine tris
   q = qmin;  bool found = false;
   for (unsigned t = 0;  t < ntri && bin[t].min < q;  ++t) {
      inq<real,tag> qnew;
      const minend &fine = bin[t].ptr->mend;

      if (fine.imin <= insub.i && insub.i < fine.iend &&
          fine.jmin <= insub.j && insub.j < fine.jend &&
          bin[t].ptr->tri_t::infirst(etd, insub, q, qnew))
         q = qnew, found = true;
   }

   // tri normals compute as "most toward" eyeball; for surf may need reverse...
   return found ? this->interior ? q.reverse(), true : true : false;
} kip_end



// inall
kip_inall(surf)
{
   // bookkeeping
   std::vector<detail::min_and_part<kip::tri<real,tag>,real>> &bin =
      mint[insub.nzone];
   const ulong ntri = bin.size();  if (ntri == 0) return false;

   // depth-sort bin, if necessary
   if (!mint.sorted[insub.nzone]) {
      std::sort(bin.begin(), bin.end(),
                detail::part_less<kip::tri<real,tag>,real>());
      mint.sorted[insub.nzone] = true;
   }

   // examine tris
   bool found = false;
   for (unsigned t = 0;  t < ntri && bin[t].min < qmin;  ++t) {
      inq<real,tag> q;
      const minend &fine = bin[t].ptr->mend;

      if (fine.imin <= insub.i && insub.i < fine.iend &&
          fine.jmin <= insub.j && insub.j < fine.jend &&
          bin[t].ptr->tri_t::infirst(etd, insub, qmin, q))
         ints.push(q), found = true;
   }

   // reverse normals, as necessary
   if (found) {
      ints.sort();  const ulong size = ints.size();
      for (ulong i = !this->interior;  i < size;  i += 2)
         ints[i].reverse();
   }
   return found;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(surf) {

   // (
   //    nnode,
   //    x, y, z,  // node 0
   //    x, y, z,  // node 1
   //    ...
   //    x, y, z,  // node nnode-1
   //
   //    ntri,
   //    u, v, w,  // tri 0
   //    u, v, w,  // tri 1
   //    ...
   //    u, v, w,  // tri ntri-1
   //
   //    tag
   // )

   obj.node.clear();  using tri_t = typename surf<real,tag>::tri_t;
   obj.tri .clear();  unsigned nnode=0, ntri=0;  s.bail = false;

   // nnode
   bool okay = read_left(s) && read_value(s, nnode, "number of nodes");

   // nodes
   obj.node.reserve(nnode);
   point<real> node;

   for (unsigned n = 0;  okay && n < nnode;  ++n) {
      std::ostringstream oss;  oss << "node[" << n << ']';
      if ((okay = read_comma(s) && read_value(s, node, oss.str().c_str())))
         obj.push(node);
   }

   // ntri
   okay = okay && read_comma(s) && read_value(s, ntri, "number of tris");

   // tris
   obj.tri.reserve(ntri);
   tri_t tri;

   for (unsigned t = 0;  okay && t < ntri;  ++t) {
      std::ostringstream oss;  oss << "tri[" << t << ']';
      if ((okay = read_comma(s) && read_value(s, tri, oss.str().c_str())))
         obj.push(tri);
   }

   if (!(okay && read_done(s, obj))) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(surf) {
   bool okay;

   const ulong nnode = obj.node.size();
   const ulong ntri  = obj.tri .size();

   // stub
   if (format == format_t::format_stub)
      okay = k << "surf()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op) {
      // nodes
      okay = k << "surf(" << nnode;
      for (ulong n = 0;  n < nnode && okay;  ++n)
         okay = k << ", " << obj.node[n];

      // tris
      okay = okay && k << ", " << ntri;
      for (ulong t = 0;  t < ntri  && okay;  ++t)
         okay = k << ", " << obj.tri [t];

      // finish
      okay = okay && write_finish(k, obj, true);
   }

   // full
   else {
      // nodes
      okay = k << "surf(\n   " && k.indent() << nnode;
      for (ulong n = 0;  n < nnode && okay;  ++n)
         okay = k << ",\n   " && k.indent() << obj.node[n];

      // tris
      okay = okay && k << ",\n   " && k.indent() << ntri;
      for (ulong t = 0;  t < ntri  && okay;  ++t)
         okay = k << ",\n   " && k.indent() << obj.tri [t];

      // finish
      okay = okay && write_finish(k, obj, false);
   }

   kip_ostream_end(surf);
}

#define   kip_class surf
#include "kip-macro-io.h"
