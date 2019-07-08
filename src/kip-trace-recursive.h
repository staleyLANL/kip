
#pragma once

namespace internal {

// -----------------------------------------------------------------------------
// shape_vectors
// -----------------------------------------------------------------------------

template<class real, class tag>
class shape_vectors {
public:

   // data
   #define kip_make_shape_vector(type)\
      std::vector<minimum_and_ptr<real,kip::type<real,tag>>> type
   kip_expand_semi(kip_make_shape_vector)
   #undef  kip_make_shape_vector

   // size
   inline size_t size() const
   {
      const functor_size f;
      allshape(*this, f);
      return f.size;
   }

   // clear
   inline void clear()
   {
      const functor_clear f;
      allshape(*this, f);
   }

   // reserve
   inline void reserve(const kip::model<real,tag> &model)
   {
   #define kip_make_reserve(type)\
      type.reserve(model.type.size());
   kip_expand_semi(kip_make_reserve)
   #undef  kip_make_reserve
   }
};



// -----------------------------------------------------------------------------
// rprepare
// -----------------------------------------------------------------------------

// zzz KIP_SEGMENTING_DIAG not used in uniform; make more consistent,
// also compare with use of other KIP_SEGMENTING_ values.
#ifdef KIP_SEGMENTING_DIAG
   #define if_segmenting_diag(arg) arg
#else
   #define if_segmenting_diag(arg)
#endif



template<class real, class base, class FROM, class TO>
inline void rprepare(
   const engine<real> &engine,
   const light<real> &light, const vars<real,base> &vars,
   FROM &from, TO &to,
   const bool  // zzz unused for now
) {
   // Prepare the shapes
   const typename FROM::iterator end = from.end();
   for (typename FROM::iterator p = from.begin();  p != end;  ++p) {
      using SHAPE = typename FROM::value_type;
      SHAPE &s = *p;
      if (!s.on) continue;

      s.is_operand = false;
      const real smin = s.SHAPE::process(vars.eyeball, light[0], engine, vars);
      if_kip_assert(smin >= 0);

      if (!s.SHAPE::dry(vars.left  ) && !s.SHAPE::dry(vars.right) &&
          !s.SHAPE::dry(vars.bottom) && !s.SHAPE::dry(vars.top  ) &&
          if_segmenting_diag(
             !s.SHAPE::dry(vars.seg_diag[0]) &&
             !s.SHAPE::dry(vars.seg_diag[2]) &&
             !s.SHAPE::dry(vars.seg_diag[1]) &&
             !s.SHAPE::dry(vars.seg_diag[3]) &&
          )
          // zzz dry(behind) maybe earlier???
          !s.SHAPE::dry(vars.behind) && !(s.get_interior() && s.solid))
      to.push_back(minimum_and_ptr<real,SHAPE>(smin,s));
   }
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// rsetup
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template<class real, class base>
void rsetup(
         model <real,base> &model,
   const light <real     > &light,
   const engine<real     > &engine,
         vars  <real,base> &vars, shape_vectors<real,base> &sv
) {
   // Target rectangle: [-hmax,+hmax] x [-vmax,+vmax]

   // Build left, right, bottom, and top segmenters
   vars.left   = dry_w(vars, -vars.hmax);
   vars.right  = dry_e(vars,  vars.hmax);
   vars.bottom = dry_s(vars, -vars.vmax);
   vars.top    = dry_n(vars,  vars.vmax);

   // Build diagonal segmenters
   if_segmenting_diag(
      vars.seg_diag.upsize(4);
      vars.seg_diag[0] = dry_nw(vars, -vars.hmax,  vars.vmax);
      vars.seg_diag[1] = dry_ne(vars,  vars.hmax,  vars.vmax);
      vars.seg_diag[2] = dry_se(vars,  vars.hmax, -vars.vmax);
      vars.seg_diag[3] = dry_sw(vars, -vars.hmax, -vars.vmax);
   )

   // zzz Couldn't there be some parallelism here, or in rprepare???
   // Prepare shapes of each type
   // zzz Evaluate the effects of "diag", like we did in uprepare()
   rprepare( engine, light, vars, model.kipand,     sv.kipand,     false );
   rprepare( engine, light, vars, model.ands,       sv.ands,       false );
   rprepare( engine, light, vars, model.odd,        sv.odd,        true  );
   rprepare( engine, light, vars, model.even,       sv.even,       false );
   rprepare( engine, light, vars, model.some,       sv.some,       true  );
   rprepare( engine, light, vars, model.one,        sv.one,        false );
   rprepare( engine, light, vars, model.ors,        sv.ors,        true  );
   rprepare( engine, light, vars, model.kipcut,     sv.kipcut,     true  );
   rprepare( engine, light, vars, model.kipnot,     sv.kipnot,     true  );
   rprepare( engine, light, vars, model.kipor,      sv.kipor,      false );
   rprepare( engine, light, vars, model.kipxor,     sv.kipxor,     true  );
   rprepare( engine, light, vars, model.bicylinder, sv.bicylinder, false );
   rprepare( engine, light, vars, model.biwasher,   sv.biwasher,   true  );
   rprepare( engine, light, vars, model.box,        sv.box,        true  );
   rprepare( engine, light, vars, model.cube,       sv.cube,       true  );
   rprepare( engine, light, vars, model.cone,       sv.cone,       false );
   rprepare( engine, light, vars, model.silo,       sv.silo,       true  );
   rprepare( engine, light, vars, model.cylinder,   sv.cylinder,   false );
   rprepare( engine, light, vars, model.pill,       sv.pill,       false );
   rprepare( engine, light, vars, model.ellipsoid,  sv.ellipsoid,  true  );
   rprepare( engine, light, vars, model.paraboloid, sv.paraboloid, false );
   rprepare( engine, light, vars, model.surf,       sv.surf,       false );
   rprepare( engine, light, vars, model.nothing,    sv.nothing,    false );
   rprepare( engine, light, vars, model.everything, sv.everything, false );
   rprepare( engine, light, vars, model.half,       sv.half,       false );
   rprepare( engine, light, vars, model.circle,     sv.circle,     true  );
   rprepare( engine, light, vars, model.xplane,     sv.xplane,     false );
   rprepare( engine, light, vars, model.yplane,     sv.yplane,     false );
   rprepare( engine, light, vars, model.zplane,     sv.zplane,     false );
   rprepare( engine, light, vars, model.sphere,     sv.sphere,     false );
   rprepare( engine, light, vars, model.spheroid,   sv.spheroid,   false );
   rprepare( engine, light, vars, model.triangle,   sv.triangle,   true  );
   rprepare( engine, light, vars, model.polygon,    sv.polygon,    true  );
   rprepare( engine, light, vars, model.washer,     sv.washer,     false );
   rprepare( engine, light, vars, model.tabular,    sv.tabular,    true  );
}



// -----------------------------------------------------------------------------
// to_abstract_then_fill
// -----------------------------------------------------------------------------

template<class real, class base>
class functor_sv2bin {
   std::vector<minimum_and_shape<real,base>> &bin;

public:
   inline functor_sv2bin(std::vector<minimum_and_shape<real,base>> &_bin) :
      bin(_bin) { }

   template<class CONTAINER>
   inline void operator()(CONTAINER &c) const
   {
      const size_t size = c.size();
      for (size_t s = 0;  s < size;  ++s)
         bin.push_back(minimum_and_shape<real,base>(c[s].minimum, *c[s].shape));
   }
};



template<class real, class color, class base, class pix>
inline void to_abstract_then_fill(
   const view<real> &view,
   const engine<real> &engine, image<real,color> &image,
   vars<real,base> &vars, const light<real> &light, array<2,pix> &pixel,

   const size_t imin, const size_t iend,
   const size_t jmin, const size_t jend,

   const shape_vectors<real,base> &sv, const size_t binsize
) {
   std::vector<minimum_and_shape<real,base>> bin;
   bin.reserve(binsize);

   const functor_sv2bin<real,base> f(bin);
   allshape(sv,f);

   /*
   // 2017-04-05
   std::cout << "kip: bin.size() == " << bin.size() << std::endl;
   std::cout << "kip:    imin == " << imin << std::endl;
   std::cout << "kip:    iend == " << iend << std::endl;
   std::cout << "kip:    jmin == " << jmin << std::endl;
   std::cout << "kip:    jend == " << jend << std::endl;
   */

   // 2017-04-05
   /// looked at mend in shape and surf
   /// nothing in particular
   /// look more carefully at the three other files

   // zzzzzz
   // Need to compute specific object bounds, like we do with uniform,
   // in order for recursive-trace to work correctly again. Also, figure
   // out something more intelligent to do than max_binsize == 0; maybe
   // allow for black/white, say, to indicate nothing/something in bin.
   // Also think about allowing different recursion levels to have
   // different number of divisions; maybe both h and v. Oh, and for
   // both uniform and recursive methods, do we (or will we) have an
   // option to automatically make hzone*hsub and vzone*vsub be the image
   // size, so that object bounds are always pixel-tight?

   const size_t zone = 0;  /// zone == 0?
   const size_t max_binsize = 0;  // for border drawing; not precomputed here

   trace_bin<shape>(
      engine, view, image, vars, light, pixel,
      imin,iend, jmin,jend, zone, max_binsize,
      bin,binsize
   );
}



// -----------------------------------------------------------------------------
// grow_portion_specific
// grow_portion
// -----------------------------------------------------------------------------

// grow_portion_specific
template<class SHAPE, class CURRENT, class PORTION, class real>
inline void grow_portion_specific(
   const tclass<SHAPE>,
   const CURRENT &current,
         PORTION &portion,
   const rotate<-3,real> seg[6],
   const char pos
) {
   const size_t size = current.size();
   portion.clear();  portion.reserve(size);

   if (pos == 'L') {
      // low
      for (size_t s = 0;  s < size;  ++s)
         if (                      !current[s].shape->SHAPE::dry(seg[0])
             if_segmenting_diag(&& !current[s].shape->SHAPE::dry(seg[2]))
             if_segmenting_diag(&& !current[s].shape->SHAPE::dry(seg[4]))
         )
            portion.push_back(current[s]);

   } else if (pos == 'C') {
      // center
      for (size_t s = 0;  s < size;  ++s)
         if (                      !current[s].shape->SHAPE::dry(seg[0])
             if_segmenting_diag(&& !current[s].shape->SHAPE::dry(seg[2]))
             if_segmenting_diag(&& !current[s].shape->SHAPE::dry(seg[4]))
                                && !current[s].shape->SHAPE::dry(seg[1])
             if_segmenting_diag(&& !current[s].shape->SHAPE::dry(seg[3]))
             if_segmenting_diag(&& !current[s].shape->SHAPE::dry(seg[5]))
         )
            portion.push_back(current[s]);

   } else  // pos == 'H')
      // high
      for (size_t s = 0;  s < size;  ++s)
         if (                      !current[s].shape->SHAPE::dry(seg[1])
             if_segmenting_diag(&& !current[s].shape->SHAPE::dry(seg[3]))
             if_segmenting_diag(&& !current[s].shape->SHAPE::dry(seg[5]))
         )
            portion.push_back(current[s]);
}



// grow_portion
template<class real, class base>
void grow_portion(
   const shape_vectors<real,base> &current,
         shape_vectors<real,base> &portion,
   const rotate<-3,real> seg[6],
   const char pos
) {
#define kip_make_grow(type)\
   grow_portion_specific\
      (tclass<type<real,base>>(), current.type,portion.type, seg,pos);
   kip_expand_semi(kip_make_grow)
#undef  kip_make_grow
}



// -----------------------------------------------------------------------------
// rtrace_h
// rtrace_v
// -----------------------------------------------------------------------------

// rtrace_h
template<class real, class color, class base, class pix>
inline void rtrace_h(
   const view<real> &view,
   const engine<real> &engine, image<real,color> &image,
   vars<real,base> &vars, const light<real> &light, array<2,pix> &pixel,

   const size_t imin, const size_t iend, const int ipart,
   const size_t jmin, const size_t jend, const real &vmin, const real &vmax,
   const shape_vectors<real,base> &bin
) {
   const size_t ilo = imin + size_t(ipart  )*(iend-imin)/engine.hdivision;
   const size_t ihi = imin + size_t(ipart+1)*(iend-imin)/engine.hdivision;
   if (ilo == ihi) return;

   const real hlo = real(ilo)*vars.hfull - vars.hmax;
   const real hhi = real(ihi)*vars.hfull - vars.hmax;
   const char pos = ipart ? ipart < int(engine.hdivision)-1 ? 'C' : 'L' : 'H';

   rotate<-3,real> seg[6];
   if (pos == 'L' || pos == 'C') {
                         seg[0] = dry_w (vars, hlo);  // 0, 2, 4 - west
      if_segmenting_diag(seg[2] = dry_nw(vars, hlo, vmax));
      if_segmenting_diag(seg[4] = dry_sw(vars, hlo, vmin));
   }
   if (pos == 'C' || pos == 'H') {
                         seg[1] = dry_e (vars, hhi);  // 1, 3, 5 - east
      if_segmenting_diag(seg[3] = dry_ne(vars, hhi, vmax));
      if_segmenting_diag(seg[5] = dry_se(vars, hhi, vmin));
   }

   shape_vectors<real,base> portion;  grow_portion(bin,portion, seg,pos);
   rtrace(view,engine,image, vars,light, pixel,
          hlo,hhi,ilo,ihi, vmin,vmax,jmin,jend, portion, false);
}



// rtrace_v
template<class real, class color, class base, class pix>
inline void rtrace_v(
   const view<real> &view,
   const engine<real> &engine, image<real,color> &image,
   vars<real,base> &vars, const light<real> &light, array<2,pix> &pixel,

   const size_t imin, const size_t iend, const real &hmin, const real &hmax,
   const size_t jmin, const size_t jend, const int jpart,
   const shape_vectors<real,base> &bin
) {
   const size_t jlo = jmin + size_t(jpart  )*(jend-jmin)/engine.vdivision;
   const size_t jhi = jmin + size_t(jpart+1)*(jend-jmin)/engine.vdivision;
   if (jlo == jhi) return;

   const real vlo = real(jlo)*vars.vfull - vars.vmax;
   const real vhi = real(jhi)*vars.vfull - vars.vmax;
   const char pos = jpart ? jpart < int(engine.vdivision)-1 ? 'C' : 'L' : 'H';

   rotate<-3,real> seg[6];
   if (pos == 'L' || pos == 'C') {
                         seg[0] = dry_s (vars, vlo);  // 0, 2, 4 - south
      if_segmenting_diag(seg[2] = dry_se(vars, hmax, vlo));
      if_segmenting_diag(seg[4] = dry_sw(vars, hmin, vlo));
   }
   if (pos == 'C' || pos == 'H') {
                         seg[1] = dry_n (vars, vhi);  // 1, 3, 5 - north
      if_segmenting_diag(seg[3] = dry_ne(vars, hmax, vhi));
      if_segmenting_diag(seg[5] = dry_nw(vars, hmin, vhi));
   }

   shape_vectors<real,base> portion;  grow_portion(bin,portion, seg,pos);
   rtrace(view,engine,image, vars,light, pixel,
          hmin,hmax,imin,iend, vlo,vhi,jlo,jhi, portion, false);
}

#undef if_segmenting_diag



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// rtrace
// Ray trace, using recursive method.
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Screen: [hmin,hmax] x [vmin,vmax]   (real)
// Pixels: [imin,iend) x [jmin,jend)   (unsigned)
template<class real, class color, class base, class pix>
void rtrace(
   const view<real> &view,
   const engine<real> &engine, image<real,color> &image,
   vars<real,base> &vars, const light<real> &light,
   array<2,pix> &pixel,

   const real &hmin, const real &hmax, const size_t imin, const size_t iend,
   const real &vmin, const real &vmax, const size_t jmin, const size_t jend,

   const shape_vectors<real,base> &bin,
   const bool rootlevel
) {
   (void)rootlevel;  // used iff OpenMP

   const size_t binsize = bin.size();
   ///   std::cout << "kip: binsize == " << binsize << std::endl;
   if (binsize == 0) {
      // might have border to draw
      if (image.border.bin)
         bin_border(image, imin,iend, jmin,jend, color::border(0,0));
      return;
   }

   // If we've recursed down far enough
   const size_t inum = iend - imin;  // exact #pixels horizontally
   const size_t jnum = jend - jmin;  // exact #pixels vertically
   if (inum*jnum <= engine.min_area) {
      ///      std::cout << "kip: calling to_abstract_then_fill()" << std::endl;
      to_abstract_then_fill(
         view,engine,image, vars,light, pixel,
         imin,iend,jmin,jend, bin,binsize
      );
      return;
   }

   // Split horizontally
   if (inum > 1 && (
       jnum < 2 ||
       inum > op::rnd<unsigned>(real(jnum)*image.aspect)
   )) {
      ///      std::cout << "kip: split horizontally" << std::endl;

      // +-----+-----+
      // |     |     |
      // |     |     |
      // |     |     |
      // +-----+-----+

      #define kip_loop\
      for (int ipart = 0;  ipart < int(engine.hdivision);  ++ipart)\
         rtrace_h(view,engine,image, vars,light, pixel,            \
                  imin,iend,ipart, jmin,jend,vmin,vmax, bin)

      #ifdef _OPENMP
         if (rootlevel)
            #pragma omp parallel for
            kip_loop;
         else
      #endif
            kip_loop;
      #undef kip_loop

   // Split vertically
   } else {
      ///      std::cout << "kip: split vertically" << std::endl;

      // +------+
      // |      |
      // |      |
      // +------+
      // |      |
      // |      |
      // +------+

      #define kip_loop\
      for (int jpart = 0;  jpart < int(engine.vdivision);  ++jpart)\
         rtrace_v(view,engine,image, vars,light, pixel,            \
                  imin,iend,hmin,hmax, jmin,jend,jpart, bin)

      #ifdef _OPENMP
         if (rootlevel)
            #pragma omp parallel for
            kip_loop;
         else
      #endif
            kip_loop;
      #undef kip_loop
   }
}

} // namespace internal
