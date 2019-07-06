
#pragma once

// For uniform:
//    KIP_SEGMENTING_LINEAR
//    KIP_SEGMENTING_BINARY
//
//    If neither: uses linear (default)
//    If linear : uses linear
//    If binary : uses binary
//    If both   : uses binary for [hv]min, linear for [hv]max

namespace kip {

// diagonal (user-settable)
namespace internal {
   inline bool &diagonal_function(void)
   {
      static bool value = false;
      return value;
   }
}
namespace { bool &diagonal = internal::diagonal_function(); }



// -----------------------------------------------------------------------------
// segment_h
// segment_v
// segment_various
// -----------------------------------------------------------------------------

namespace internal {

// segment_h: compute horizontal segmenters
template<class real, class base>
inline void segment_h(const engine<real> &engine, vars<real,base> &vars)
{
   const int htotal = int(engine.hzone * engine.hsub);
   vars.seg_hneg.resize(htotal);
   vars.seg_hpos.resize(htotal);

   #ifdef _OPENMP
      #pragma omp parallel for
   #endif
   for (int hseg = 0;  hseg < htotal;  ++hseg) {
      const real
         hmin = real(op::rnd<unsigned>(vars.hratsub*real(hseg  )))*vars.hfull -
                vars.hmax,
         hmax = real(op::rnd<unsigned>(vars.hratsub*real(hseg+1)))*vars.hfull -
                vars.hmax;
      vars.seg_hneg[hseg] = dry_w(vars, hmin);
      vars.seg_hpos[hseg] = dry_e(vars, hmax);
   }
}



// segment_v: compute vertical segmenters
template<class real, class base>
inline void segment_v(const engine<real> &engine, vars<real,base> &vars)
{
   const int vtotal = int(engine.vzone * engine.vsub);
   vars.seg_vneg.resize(vtotal);
   vars.seg_vpos.resize(vtotal);

   #ifdef _OPENMP
      #pragma omp parallel for
   #endif
   for (int vseg = 0;  vseg < vtotal;  ++vseg) {
      const real
         vmin = real(op::rnd<unsigned>(vars.vratsub*real(vseg  )))*vars.vfull -
                vars.vmax,
         vmax = real(op::rnd<unsigned>(vars.vratsub*real(vseg+1)))*vars.vfull -
                vars.vmax;
      vars.seg_vneg[vseg] = dry_s(vars, vmin);
      vars.seg_vpos[vseg] = dry_n(vars, vmax);
   }
}



// segment_various
template<class real, class base>
inline void segment_various(const engine<real> &engine, vars<real,base> &vars)
{
   // number of bins
   const int nbin = int(engine.hzone * engine.vzone);

   // resize
   vars.seg_diag.resize(4*nbin);
   #ifdef KIP_SEGMENTING_QUAD
      vars.seg_quad.resize(8*nbin);
   #endif
   #ifdef KIP_SEGMENTING_3060
      vars.seg_3060.resize(8*nbin);
   #endif
   #ifdef KIP_SEGMENTING_1575
      vars.seg_1575.resize(8*nbin);
   #endif

   // for each bin
   #ifdef _OPENMP
      #pragma omp parallel for
   #endif
   for (int b = 0;  b < nbin;  ++b) {
      const int hseg = b % int(engine.hzone);
      const int vseg = b / int(engine.hzone);

      const real
         hmin = real(op::rnd<unsigned>(vars.hrat*real(hseg  )))*vars.hfull -
                vars.hmax,
         hmax = real(op::rnd<unsigned>(vars.hrat*real(hseg+1)))*vars.hfull -
                vars.hmax,
         vmin = real(op::rnd<unsigned>(vars.vrat*real(vseg  )))*vars.vfull -
                vars.vmax,
         vmax = real(op::rnd<unsigned>(vars.vrat*real(vseg+1)))*vars.vfull -
                vars.vmax;

      // diag: compute diagonal (45-degree) segmenters
      int n = 4*b;
      vars.seg_diag[n++] = dry_sw(vars, hmin,vmin);
      vars.seg_diag[n++] = dry_ne(vars, hmax,vmax);
      vars.seg_diag[n++] = dry_se(vars, hmax,vmin);
      vars.seg_diag[n++] = dry_nw(vars, hmin,vmax);

      // quad: compute 22.5- and 67.5-degree segmenters
      #ifdef KIP_SEGMENTING_QUAD
         n = 8*b;
         vars.seg_quad[n++] = dry_sssw(vars, hmin,vmin);
         vars.seg_quad[n++] = dry_wwsw(vars, hmin,vmin);
         vars.seg_quad[n++] = dry_nnne(vars, hmax,vmax);
         vars.seg_quad[n++] = dry_eene(vars, hmax,vmax);
         vars.seg_quad[n++] = dry_ssse(vars, hmax,vmin);
         vars.seg_quad[n++] = dry_eese(vars, hmax,vmin);
         vars.seg_quad[n++] = dry_nnnw(vars, hmin,vmax);
         vars.seg_quad[n++] = dry_wwnw(vars, hmin,vmax);
      #endif

      // 3060: compute 30- and 60-degree segmenters
      #ifdef KIP_SEGMENTING_3060
         n = 8*b;
         vars.seg_3060[n++] = dry_ssw(vars, hmin, vmin);
         vars.seg_3060[n++] = dry_wsw(vars, hmin, vmin);
         vars.seg_3060[n++] = dry_nne(vars, hmax, vmax);
         vars.seg_3060[n++] = dry_ene(vars, hmax, vmax);
         vars.seg_3060[n++] = dry_sse(vars, hmax, vmin);
         vars.seg_3060[n++] = dry_ese(vars, hmax, vmin);
         vars.seg_3060[n++] = dry_nnw(vars, hmin, vmax);
         vars.seg_3060[n++] = dry_wnw(vars, hmin, vmax);
      #endif

      // 1575: compute 15- and 75-degree segmenters
      #ifdef KIP_SEGMENTING_1575
         n = 8*b;
         vars.seg_1575[n++] = dry_sw75(vars, hmin,vmin);  // - -
         vars.seg_1575[n++] = dry_sw15(vars, hmin,vmin);  // * -
         vars.seg_1575[n++] = dry_ne75(vars, hmax,vmax);  // - *
         vars.seg_1575[n++] = dry_ne15(vars, hmax,vmax);  // - -
         vars.seg_1575[n++] = dry_se75(vars, hmax,vmin);  // - -
         vars.seg_1575[n++] = dry_se15(vars, hmax,vmin);  // - *
         vars.seg_1575[n++] = dry_nw75(vars, hmin,vmax);  // * -
         vars.seg_1575[n++] = dry_nw15(vars, hmin,vmax);  // - -
      #endif
   }
}



// -----------------------------------------------------------------------------
// seg_minmax
// Find [imin,iend) x [jmin,jend)
// -----------------------------------------------------------------------------

// General version
template<class real, class base, class SHAPE, class ij>
inline bool seg_minmax(
   const engine<real> &engine, const vars<real,base> &vars,
   const SHAPE &i, ij &imin, ij &iend, ij &jmin, ij &jend
) {
   // Note: loops in this function cannot be parallelized, but this whole
   // function can, in fact, be called in parallel; see later in the file.
   const unsigned htotal = engine.hzone * engine.hsub;
   const unsigned vtotal = engine.vzone * engine.vsub;

   #ifdef KIP_SEGMENTING_BINARY
      // Binary search
      int m, hi, lo;

      // horizontal
      hi = int(htotal);  lo = -1;
      do if (i.SHAPE::dry(vars.seg_hpos[m = (lo+hi)/2])) lo = m; else hi = m;
      while (hi > lo+1);
      if ((imin = ij(lo+1)) == htotal) return false;
      #ifdef KIP_SEGMENTING_LINEAR
         iend = imin;
         while (iend < htotal && !(i.SHAPE::dry(vars.seg_hneg[iend]))) iend++;
         if (imin == iend) return false;
      #else
         hi = htotal;  lo = imin-1;
         do if (i.SHAPE::dry(vars.seg_hneg[m = (lo+hi)/2])) hi = m; else lo = m;
         while (hi > lo+1);
         if ((iend = lo+1) == imin) return false;
      #endif

      // vertical
      hi = int(vtotal);  lo = -1;
      do if (i.SHAPE::dry(vars.seg_vpos[m = (lo+hi)/2])) lo = m; else hi = m;
      while (hi > lo+1);
      if ((jmin = ij(lo+1)) == vtotal) return false;
      #ifdef KIP_SEGMENTING_LINEAR
         jend = jmin;
         while (jend < vtotal && !(i.SHAPE::dry(vars.seg_vneg[jend]))) jend++;
         if (jmin == jend) return false;
      #else
         hi = vtotal;  lo = jmin-1;
         do if (i.SHAPE::dry(vars.seg_vneg[m = (lo+hi)/2])) hi = m; else lo = m;
         while (hi > lo+1);
         if ((jend = lo+1) == jmin) return false;
      #endif

   #else
      // Linear search

      // horizontal
      imin = 0;
      while (imin < htotal &&   i.SHAPE::dry(vars.seg_hpos[imin]) ) imin++;
      if (imin == htotal) return false;  // past the right
      iend = imin;
      while (iend < htotal && !(i.SHAPE::dry(vars.seg_hneg[iend]))) iend++;
      if (imin == iend) return false;


      // vertical
      jmin = 0;
      while (jmin < vtotal &&   i.SHAPE::dry(vars.seg_vpos[jmin]) ) jmin++;
      if (jmin == vtotal) return false;  // above the top
      jend = jmin;
      while (jend < vtotal && !(i.SHAPE::dry(vars.seg_vneg[jend]))) jend++;
      if (jmin == jend) return false;
   #endif

   return true;
}



// dry() for point
template<class real>
inline bool dry(const point<real> &p, const rotate<-3,real> &seg)
   { return seg.ge(p); }



// Version for point
template<class real, class base, class ij>
inline void seg_minmax(
   const engine<real> &engine, const vars<real,base> &vars,
   const point<real> &p, ij &imin, ij &iend, ij &jmin, ij &jend
) {
   const unsigned htotal = engine.hzone * engine.hsub;
   const unsigned vtotal = engine.vzone * engine.vsub;  int m, hi, lo;

   // i (horizontal)
   hi = int(htotal);  lo = -1;
   do if (dry(p, vars.seg_hpos[m = (lo+hi)/2])) lo = m; else hi = m;
   while (hi > lo+1);
   iend = imin = ij(lo+1);
   while (iend < htotal && !(dry(p, vars.seg_hneg[iend]))) iend++;

   // j (vertical)
   hi = int(vtotal);  lo = -1;
   do if (dry(p, vars.seg_vpos[m = (lo+hi)/2])) lo = m; else hi = m;
   while (hi > lo+1);
   jend = jmin = ij(lo+1);
   while (jend < vtotal && !(dry(p, vars.seg_vneg[jend]))) jend++;
}



// -----------------------------------------------------------------------------
// argsub
// -----------------------------------------------------------------------------

// <0> - primitive
// <1> - unary
// <2> - binary
// <3> - nary

/*
// Later split; should not be generic
template<unsigned code, class real, class base, class ORIGINAL>
inline void argsub(const engine<real> &engine, vars<real,base> &vars, SHAPE &s)
{
   (void)engine;
   (void)vars;
   (void)s;

   minend<> sub;
   if (seg_minmax(engine,vars,s, sub.imin,sub.iend,sub.jmin,sub.jend) &&
      !s.SHAPE::dry(vars.behind));
      continue;

   s.mend.imin = op::rnd<unsigned>(vars.hratsub * sub.imin);
   s.mend.iend = op::rnd<unsigned>(vars.hratsub * sub.iend);
   s.mend.jmin = op::rnd<unsigned>(vars.vratsub * sub.jmin);
   s.mend.jend = op::rnd<unsigned>(vars.vratsub * sub.jend);
}
*/



// -----------------------------------------------------------------------------
// test_diag
// test_quad
// test_3060
// test_1575
// -----------------------------------------------------------------------------

// test_diag
template<class real, class base, class SHAPE>
inline char test_diag(
   const vars<real,base> &vars, const SHAPE &p, const size_t val,
   minend/*<>*/ &bin, const size_t hseg
) {
   const size_t n = 4*val;

   if (p.SHAPE::dry(vars.seg_diag[n  ])) return bin.iend = hseg,   'b';  // sw
   if (p.SHAPE::dry(vars.seg_diag[n+3])) return                    'b';  // nw
   if (p.SHAPE::dry(vars.seg_diag[n+2])) return bin.imin = hseg+1, 'c';  // se
   if (p.SHAPE::dry(vars.seg_diag[n+1])) return                    'c';  // ne

   return '\0';
}



// test_quad
template<class real, class base, class SHAPE>
inline char test_quad(
   const vars<real,base> &vars, const SHAPE &p, const size_t val
) {
   #ifdef KIP_SEGMENTING_QUAD
      const unsigned a = 8*val;
      if (p.SHAPE::dry(vars.seg_quad[a  ]) ||  // sw
          p.SHAPE::dry(vars.seg_quad[a+2]) ||  // ne
          p.SHAPE::dry(vars.seg_quad[a+1]) ||  // sw
          p.SHAPE::dry(vars.seg_quad[a+3]) ||  // ne
          p.SHAPE::dry(vars.seg_quad[a+4]) ||  // se
          p.SHAPE::dry(vars.seg_quad[a+6]) ||  // nw
          p.SHAPE::dry(vars.seg_quad[a+5]) ||  // se
          p.SHAPE::dry(vars.seg_quad[a+7]))    // nw
         return 'c';
   #else
      (void)vars;  (void)p;  (void)val;
   #endif

   return '\0';
}



// test_3060
template<class real, class base, class SHAPE>
inline char test_3060(
   const vars<real,base> &vars, const SHAPE &p, const size_t val
) {
   #ifdef KIP_SEGMENTING_3060
      const unsigned b = 8*val;

      if (p.SHAPE::dry(vars.seg_3060[b  ]) ||
          p.SHAPE::dry(vars.seg_3060[b+2]) ||
          p.SHAPE::dry(vars.seg_3060[b+1]) ||
          p.SHAPE::dry(vars.seg_3060[b+3]) ||
          p.SHAPE::dry(vars.seg_3060[b+4]) ||
          p.SHAPE::dry(vars.seg_3060[b+6]) ||
          p.SHAPE::dry(vars.seg_3060[b+5]) ||
          p.SHAPE::dry(vars.seg_3060[b+7]))
         return 'c';
   #else
      (void)vars;  (void)p;  (void)val;
   #endif

   return '\0';
}



// test_1575
template<class real, class base, class SHAPE>
inline char test_1575(
   const vars<real,base> &vars, const SHAPE &p, const size_t val
) {
   #ifdef KIP_SEGMENTING_1575
      const unsigned c = 8*val;

      if (p.SHAPE::dry(vars.seg_1575[c  ]) ||
          p.SHAPE::dry(vars.seg_1575[c+2]) ||
          p.SHAPE::dry(vars.seg_1575[c+1]) ||
          p.SHAPE::dry(vars.seg_1575[c+3]) ||
          p.SHAPE::dry(vars.seg_1575[c+4]) ||
          p.SHAPE::dry(vars.seg_1575[c+6]) ||
          p.SHAPE::dry(vars.seg_1575[c+5]) ||
          p.SHAPE::dry(vars.seg_1575[c+7]))
         return 'c';
   #else
      (void)vars;  (void)p;  (void)val;
   #endif

   return '\0';
}



// -----------------------------------------------------------------------------
// uprepare: general
// -----------------------------------------------------------------------------

// merge_bins - helper
template<class real, class base>
inline void merge_bins(
   const int nbin, const unsigned hzone,
   vars<real,base> &vars, const int nthreads,
   const array<3, std::vector< minimum_and_shape<real,base> > > &per_zone
) {
   #ifdef _OPENMP
      #pragma omp parallel for
      for (int b = 0;  b < nbin;  ++b) {
         const unsigned hseg = b % hzone;
         const unsigned vseg = b / hzone;

         for (int t = 1;  t < nthreads;  ++t)
            vars.uniform[b].insert(
               vars.uniform[b].end(),
               per_zone(hseg, vseg, t-1).begin(),
               per_zone(hseg, vseg, t-1).end  ()
            );
      }
   #else
      (void)nbin;  (void)hzone;  (void)vars;
      (void)nthreads;  (void)per_zone;
   #endif
}



// uprepare
template<unsigned code, class real, class base, class SHAPEVEC>
inline void uprepare(
   const light<real> &light, const engine<real> &engine,
   vars<real,base> &vars, const int nbin,
   SHAPEVEC &vec,  // std::vector of model.sphere, model.ands, etc.
   const bool _diag
) {
   // number of this particular type of shape
   const int numobj = int(vec.size());  // int, for OpenMP
   if (numobj == 0) return;  // none of this type of shape

   // kip::diagonal is a "global" user-settable flag
   const bool diag = diagonal || _diag;

   // per_zone(hzone,vzone,nthreads-1) (-1 because thread 0 uses vars.uniform)
   static array<3, std::vector< minimum_and_shape<real,base> > > per_zone;
   const int nthreads = get_nthreads();
   #ifdef _OPENMP
      for (size_t z = per_zone.size();  z--; )
         per_zone[z].clear();
      if (nthreads > 1)
         per_zone.resize(engine.hzone, engine.vzone, nthreads-1);
   #endif

   // for each object of this type...
   #ifdef _OPENMP
      #pragma omp parallel for
   #endif
   for (int i = 0;  i < numobj;  ++i) {
      ///      const size_t i = iint;
      using SHAPE = typename SHAPEVEC::value_type;
      SHAPE &p = vec[size_t(i)];
      if (!p.on) continue;

      // minimum distance from eyeball
      p.is_operand = false;  // or we wouldn't be in the object's std::vector
      const real pmin = p.SHAPE::process(vars.eyeball, light[0], engine, vars);
      if_kip_assert(pmin >= 0);

      // behind us, off-screen, or we're inside
      minend/*<>*/ sub, bin;
      if (p.SHAPE::dry(vars.behind) ||
         !seg_minmax(engine,vars,p, sub.imin,sub.iend,sub.jmin,sub.jend) ||
         (p.get_interior() && p.solid))
         continue;

      // fine boundaries (used later, when shooting rays)
      p.mend.imin = op::rnd<unsigned>(vars.hratsub * real(sub.imin));
      p.mend.iend = op::rnd<unsigned>(vars.hratsub * real(sub.iend));
      p.mend.jmin = op::rnd<unsigned>(vars.vratsub * real(sub.jmin));
      p.mend.jend = op::rnd<unsigned>(vars.vratsub * real(sub.jend));

      // coarse bins (used shortly, when binning objects)
      bin.imin =  sub.imin / engine.hsub;
      bin.iend = (sub.iend + engine.hsub - 1)/engine.hsub;
      bin.jmin =  sub.jmin / engine.vsub;
      bin.jend = (sub.jend + engine.vsub - 1)/engine.vsub;

      const int thread = this_thread();
      std::vector< minimum_and_shape<real,base> > *const ptr = thread
       ? &per_zone[nbin*(thread-1)]
       : &vars.uniform[0];

      // drop into bins
      for (size_t vseg = bin.jmin;  vseg < bin.jend;  ++vseg) {
         size_t val = vseg*engine.hzone + bin.imin;

         for (size_t hseg = bin.imin;  hseg < bin.iend;  ++hseg, ++val) {
            // diag?
            if (diag) {
               const char rv = test_diag(vars, p, val, bin, hseg);
               if (rv == 'b') break;
               if (rv == 'c') continue;
            }

            // quad, 3060, 1575?
            if (test_quad(vars, p, val) == 'c' ||
                test_3060(vars, p, val) == 'c' ||
                test_1575(vars, p, val) == 'c') continue;

            // push to bin
            ptr[val].push_back(minimum_and_shape<real,base>(pmin,p));
         }
      }
   }

   merge_bins(nbin, engine.hzone, vars, nthreads, per_zone);
}



// -----------------------------------------------------------------------------
// uprepare_tri
// uprepare_surf
// -----------------------------------------------------------------------------

// uprepare_tri
template<class element_t, class real, class base, class BINS>
inline real uprepare_tri(
   const engine<real> &engine, const vars<real,base> &vars,
   BINS &bins, const surf<real,base> &surf, const bool object_border
) {
   real min_overall = std::numeric_limits<real>::max();

   // per-node "behind" and "minend"
   const size_t nnode = surf.node.size();
   std::vector< bool > behind(nnode);
   std::vector< minend/*<>*/ > me(nnode);

   // for each node...
   for (size_t n = nnode;  n--; ) {
      behind[n] = vars.behind.ge(surf.node[n]);
      seg_minmax(
         engine, vars, surf.node[n],
         me[n].imin, me[n].iend,
         me[n].jmin, me[n].jend
      );
   }

   // for each tri
   using tri_t = tri<real,base>;
   for (size_t it = surf.tri.size();  it--; ) {
      tri_t &t = surf.tri[it];  // this particular tri

      // process
      const real m = t.tri_t::process(surf.node, vars.eyeball, engine, vars);
      const size_t u = t.u(), v = t.v(), w = t.w();

      // degenerate / coplanar with eye, or entirely behind eye
      if (t.degenerate || (behind[u] && behind[v] && behind[w])) continue;

      // tri min and end, based on those of its vertices
      const size_t imin = op::min(me[u].imin, me[v].imin, me[w].imin);
      const size_t iend = op::max(me[u].iend, me[v].iend, me[w].iend);
      const size_t jmin = op::min(me[u].jmin, me[v].jmin, me[w].jmin);
      const size_t jend = op::max(me[u].jend, me[v].jend, me[w].jend);
      if (imin == iend || jmin == jend) continue;

      // fine boundaries
      t.mend.imin = op::rnd<unsigned>(vars.hratsub * real(imin));
      t.mend.iend = op::rnd<unsigned>(vars.hratsub * real(iend));
      t.mend.jmin = op::rnd<unsigned>(vars.vratsub * real(jmin));
      t.mend.jend = op::rnd<unsigned>(vars.vratsub * real(jend));

      if (object_border) {
         surf.mend.imin = op::min(surf.mend.imin, t.mend.imin);
         surf.mend.iend = op::max(surf.mend.iend, t.mend.iend);
         surf.mend.jmin = op::min(surf.mend.jmin, t.mend.jmin);
         surf.mend.jend = op::max(surf.mend.jend, t.mend.jend);
      }

      // coarse bins
      const size_t bimin =  imin / engine.hsub;
      const size_t biend = (iend + engine.hsub - 1)/engine.hsub;
      const size_t bjmin =  jmin / engine.vsub;
      const size_t bjend = (jend + engine.vsub - 1)/engine.vsub;

      // bring over base
      t.base() = surf.base();

      // drop into bins
      for (size_t j = bjmin;  j < bjend;  ++j) {
         size_t val = j*engine.hzone + bimin;
         for (size_t i = bimin;  i < biend;  ++i)
            bins[val++].push_back(element_t(m,t));
      }

      // overall minimum for the enclosing surf
      if (m < min_overall) min_overall = m;
   }

   return min_overall;
}



// uprepare_surf
template<class real, class base>
inline void uprepare_surf(
   const light<real> &light, const engine<real> &engine, vars<real,base> &vars,
   const int nbin, const bool object_border, std::vector< surf<real,base> > &vec
) {
   // number of surfs
   const int nsurf = int(vec.size());  // int, for OpenMP
   if (nsurf == 0) return;

   // per_zone(hzone, vzone, nthreads-1) (-1 because thread 0 uses vars.uniform)
   static array<3, std::vector< minimum_and_shape<real,base> > > per_zone;
   const int nthreads = get_nthreads();
   #ifdef _OPENMP
      for (size_t z = per_zone.size();  z--; )
         per_zone[z].clear();
      if (nthreads > 1)
         per_zone.resize(engine.hzone, engine.vzone, nthreads-1);
   #endif

   // for each surf...
   #ifdef _OPENMP
      #pragma omp parallel for
   #endif
   for (int s = 0;  s < nsurf;  ++s) {
      const surf<real,base> &p = vec[size_t(s)];  // this surf
      if (!p.on) continue;

      p.is_operand = false;  // global (not as operand) surf
      p.surf<real,base>::process(vars.eyeball, light[0], engine, vars);
      if ((p.get_interior() && p.solid) || p.degenerate)
         continue;

      if (object_border ) {
         p.mend.imin = std::numeric_limits<size_t>::max();
         p.mend.iend = std::numeric_limits<size_t>::min();
         p.mend.jmin = std::numeric_limits<size_t>::max();
         p.mend.jend = std::numeric_limits<size_t>::min();
      }

      const int thread = this_thread();
      std::vector< minimum_and_shape<real,base> > *const ptr = thread
       ? &per_zone[nbin*(thread-1)]
       : &vars.uniform[0];
      uprepare_tri< minimum_and_shape<real,base> >
         (engine,vars, ptr,p, object_border);
   }

   merge_bins(nbin, engine.hzone, vars, nthreads, per_zone);
}



// -----------------------------------------------------------------------------
// usetup
// -----------------------------------------------------------------------------

template<class real, class base>
void usetup(
         model <real,base> &model,
   const light <real     > &light,
   const engine<real     > &engine,
         vars  <real,base> &vars,
   const bool object_border
) {
   // Build segmenters
   segment_h(engine,vars);
   segment_v(engine,vars);  segment_various(engine,vars);

   // Prepare most shapes
   const int nbin = int(engine.hzone * engine.vzone);
   #define kip_lev light, engine, vars, nbin, model.

   uprepare<1>  ( kip_lev kipnot,     true );

   uprepare<2>  ( kip_lev kipand,     true );
   uprepare<2>  ( kip_lev kipcut,     true );
   uprepare<2>  ( kip_lev kipor,      true );
   uprepare<2>  ( kip_lev kipxor,     true );

   uprepare<3>  ( kip_lev ands,       true );
   uprepare<3>  ( kip_lev even,       true );
   uprepare<3>  ( kip_lev odd,        true );
   uprepare<3>  ( kip_lev one,        true );
   uprepare<3>  ( kip_lev ors,        true );
   uprepare<3>  ( kip_lev some,       true );

   uprepare<0>  ( kip_lev bicylinder, true );
   uprepare<0>  ( kip_lev biwasher,   true );
   uprepare<0>  ( kip_lev box,        true );
   uprepare<0>  ( kip_lev cube,       true );
   uprepare<0>  ( kip_lev circle,     true );
   uprepare<0>  ( kip_lev cone,       true );
   uprepare<0>  ( kip_lev cylinder,   true );
   uprepare<0>  ( kip_lev ellipsoid,  true );
   uprepare<0>  ( kip_lev half,       true );
   uprepare<0>  ( kip_lev paraboloid, true );
   uprepare<0>  ( kip_lev nothing,    true );
   uprepare<0>  ( kip_lev everything, true );
   uprepare<0>  ( kip_lev pill,       true );
   uprepare<0>  ( kip_lev polygon,    true );
   uprepare<0>  ( kip_lev silo,       true );
   uprepare<0>  ( kip_lev sphere,     true );
   uprepare<0>  ( kip_lev spheroid,   true );
   uprepare<0>  ( kip_lev tabular,    true );
   uprepare<0>  ( kip_lev triangle,   true );
   uprepare<0>  ( kip_lev washer,     true );
   uprepare<0>  ( kip_lev xplane,     true );
   uprepare<0>  ( kip_lev yplane,     true );
   uprepare<0>  ( kip_lev zplane,     true );

   #undef kip_lev

   // Prepare surfs; creates "tri" objects
   uprepare_surf(light, engine, vars, nbin, object_border, model.surf);
}



// -----------------------------------------------------------------------------
// utrace_do
// Helper for utrace()
// -----------------------------------------------------------------------------

template<
   template<class,class> class SHAPE,
   class real, class tag, class color, class pix
>
class utrace_do {
public:

   // ------------------------
   // operator()
   // ------------------------

   inline void operator()(
      const int nzone,
      const view <real> &view,
      const light<real> &light, const engine<real> &engine,
      vars<real,tag> &vars, image<real,color> &image, array<2,pix> &pixel
   ) const {

      // Compute maximum bin size
      // qqq should parallelize, if openmp
      size_t max_binsize = 0;
      if (image.border.bin) {
         for (int zone = 0;  zone < nzone;  ++zone)
            max_binsize = std::max(max_binsize, vars.uniform[zone].size());
      }

      // Loop over the bins
      #if defined(_OPENMP)
         #pragma omp parallel for
      #endif
      for (size_t zone = 0;  zone < size_t(nzone);  ++zone) {
         const size_t binsize = vars.uniform[zone].size();
         if (binsize == 0 && !image.border.bin) continue;

         /*
         vars.hrat = real(image.hpixel)/real(engine.hzone);
         vars.vrat = real(image.vpixel)/real(engine.vzone);
         */
         unsigned
            imin = op::rnd<unsigned>(vars.hrat*real (zone % engine.hzone)),
            iend = op::rnd<unsigned>(vars.hrat*real((zone % engine.hzone)+1)),
            jmin = op::rnd<unsigned>(vars.vrat*real (zone / engine.hzone)),
            jend = op::rnd<unsigned>(vars.vrat*real((zone / engine.hzone)+1));

         if (binsize == 0)
            bin_border(image,imin,iend,jmin,jend,color::border(0,max_binsize));
         else {
            trace_bin<SHAPE>(
               engine, view, image, vars, light, pixel,
               imin,iend, jmin,jend, zone, max_binsize,
               vars.uniform[zone], binsize  // latter = former.size()
            );
         }
      }
   }
};



// -----------------------------------------------------------------------------
// utrace
// Ray trace, using uniform method.
// -----------------------------------------------------------------------------

template<class real, class tag, class color, class pix>
void utrace(
   const model <real,tag  > &model,
   const view  <real      > &view,
   const light <real      > &light,
   const engine<real      > &engine,
         vars  <real,tag  > &vars,
         image <real,color> &image,
   array<2,pix> &pixel
) {
   (void)model;
   const size_t nzone = engine.hzone*engine.vzone;

   /*
   const size_t nobjects = model.size();
   #define kip_single_shape(shape)\
      if (nobjects == model.shape.size()) {\
         utrace_do<shape, real, tag, color, pix>()\
            (nzone, light, engine, vars, image, pixel);\
         return;\
      }
   kip_expand_plain(kip_single_shape)
   #undef kip_single_shape
   */

   // general model
   {
      utrace_do<shape, real, tag, color, pix>()
         (int(nzone), view, light, engine, vars, image, pixel);
   }
}

}
}
