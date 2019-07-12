
#pragma once

// -----------------------------------------------------------------------------
// misc
// -----------------------------------------------------------------------------

// initialize_pixel: array<2,nothing_per_pixel>
// no action
inline void initialize_pixel(
   array<2,nothing_per_pixel> &, const size_t, const size_t
) {
}



// initialize_pixel: array<2,pix>
template<class pix>
inline void initialize_pixel(
   array<2,pix> &pixel, const size_t hpixel, const size_t vpixel
) {
   pixel.upsize(hpixel,vpixel);

   const int size = int(hpixel*vpixel);
   #ifdef _OPENMP
      #pragma omp parallel for
   #endif
   for (int n = 0;  n < size;  ++n)
      pixel[n].initialize();
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// trace: helpers
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

namespace internal {

// angle_tweak
template<class real>
inline real angle_tweak(const real angle)
{
   static const real fac = std::pow(
      real(10),
     -std::numeric_limits<real>::digits10 / real(2)
   );
   return std::fmod(angle,real(360)) + real(random<real>() - 0.5)*fac;
}



// -----------------------------------------------------------------------------
// trace_vars
// -----------------------------------------------------------------------------

template<class base, class real, class color>
inline void trace_vars(
         model <real,base > &model,   // input
   const view  <real      > &view,    // input
   const light <real      > &light,   // input
   const engine<real      > &engine,  // input
         image <real,color> &image,   // input/output
         vars  <real,base > &vars     // auxiliary
) {
   (void)model;  (void)light;
   const size_t hpixel = image.hpixel;
   const size_t vpixel = image.vpixel;

   // t2e: target/eyeball rotate
   //    fore: to x-axis
   //    back: to final position
   vars.t2e = rotate<3,real>(
      #ifdef KIP_ANGLE_TWEAK
         // With slight angle randomization. This is a temporary solution
         // to the cut-plane issue Laura encountered.
         pi<real>/180 * angle_tweak(view.roll),
         pi<real>/180 * angle_tweak(view.phi),
         pi<real>/180 * angle_tweak(view.theta),
      #else
         // Normal computation
         pi<real>/180 * view.roll,
         pi<real>/180 * view.phi,
         pi<real>/180 * view.theta,
      #endif
      view.target
   );

   // eyeball
   vars.eyeball = vars.t2e.back_n00(view.d);

   // target rectangle: [-hmax,+hmax] x [-vmax,+vmax]
   vars.hmax = view.d * std::tan(view.fov * (pi<real>/360));
   vars.vmax = image.aspect * real(vpixel)/real(hpixel) * vars.hmax;

   // pixels: half & full sizes (remember, domain length is 2*max (-max..max))
   vars.hhalf = vars.hmax/real(hpixel), vars.hfull = op::twice(vars.hhalf);
   vars.vhalf = vars.vmax/real(vpixel), vars.vfull = op::twice(vars.vhalf);

   // heps, veps: slightly less (based on the fudge factor) than the half-sizes
   // of the antialiasing subpixels
   vars.heps = engine.fudge * vars.hhalf / real(image.anti);
   vars.veps = engine.fudge * vars.vhalf / real(image.anti);

   // behind: right-hand-rule thumb points behind eyeball
   vars.behind = rotate<-3,real>(
      vars.eyeball,
      vars.t2e.back_n01(view.d),  // up from eyeball
      vars.t2e.back_nm0(view.d)   // left of eyeball
   );

   // anti2
   vars.anti2 = image.anti * image.anti;

   // rec_anti*
   vars.rec_anti (real ()) = 1 / real (image.anti);
   vars.rec_anti2(real ()) = 1 / real (image.anti * image.anti);
   vars.rec_anti (float()) = 1 / float(image.anti);
   vars.rec_anti2(float()) = 1 / float(image.anti * image.anti);
}



// -----------------------------------------------------------------------------
// trace_vipt
// -----------------------------------------------------------------------------

template<class real, class base, class color>
inline void trace_vipt(
   const view  <real      > &view,    // input
   const engine<real      > &engine,  // input
         image <real,color> &image,   // input/output
   const vars  <real,base > &vars     // auxiliary
) {
   if (engine.lean) {
      image.prior.targets.resize(0,0);
      return;
   }
   if (image.anti > 1) return;

   const size_t hpixel = image.hpixel;
   const size_t vpixel = image.vpixel;
   const real   aspect = image.aspect;

   // New, vs. prior view/image?
   const bool new_d      = view .prior.first || view.d   != view .prior.d;
   const bool new_fov    = view .prior.first || view.fov != view .prior.fov;
   const bool new_hpixel = image.prior.first || hpixel   != image.prior.hpixel;
   const bool new_vpixel = image.prior.first || vpixel   != image.prior.vpixel;
   const bool new_aspect = image.prior.first || aspect   != image.prior.aspect;

   if (!(new_d || new_fov || new_hpixel || new_vpixel || new_aspect))
      return;

   // As necessary: (0,h,v), make distance=1 from (d,0,0), save new targets
   image.prior.targets.upsize(hpixel,vpixel);

   if (!(new_fov || new_hpixel || new_vpixel || new_aspect)) {
      // new_d only
      const size_t npixel = hpixel*vpixel;
      const real diff_d = view.d - view.prior.d;
      #ifdef _OPENMP
         #pragma omp parallel for
      #endif
      for (size_t n = 0;  n < npixel;  ++n)
         image.prior.targets[n].x += diff_d;
   } else {
      // general change
      const real hmin = vars.hhalf - vars.hmax;
      const real vmin = vars.vhalf - vars.vmax, dsq = view.d*view.d;

      #ifdef _OPENMP
         #pragma omp parallel for
      #endif
      for (size_t j = 0;  j < vpixel;  ++j) {
         const real v = vmin + real(j)*vars.vfull, tmp = dsq + v*v;
         real h = hmin;  size_t n = j*hpixel;

         for (size_t i = 0;  i < hpixel;  ++i, h += vars.hfull) {
            // a = (d,0,0),  b = (0,h,v),  (x,y,z) = a+(b-a)/mod(b-a)
            const real norm = real(1)/std::sqrt(tmp + h*h);
            image.prior.targets[n++](view.d*(1-norm), h*norm, v*norm);
         }
      }
   }

   // Update prior view/image
   view .prior.first  = false;
   view .prior.d      = view.d;
   view .prior.fov    = view.fov;
   image.prior.first  = false;
   image.prior.aspect = image.aspect;
}



// -----------------------------------------------------------------------------
// trace_bitmap
// -----------------------------------------------------------------------------

template<class real, class color>
inline void trace_bitmap(
   image<real,color> &image  // input/output
) {
   // quick-initialize bitmap to background
   const size_t size = image.hpixel * image.vpixel;
   #ifdef _OPENMP
      #pragma omp parallel for
   #endif
   for (size_t n = 0;  n < size;  ++n)
      image.bitmap[n] = image.background;
}



// -----------------------------------------------------------------------------
// trace_pixel
// -----------------------------------------------------------------------------

template<class real, class color, class pix>
inline void trace_pixel(
   image<real,color> &image,   // input/output
   array<2,pix> &pixel    // per-pixel information
) {
   // quick-initialize shape pointers to NULL
   const size_t hpixel = image.hpixel;
   const size_t vpixel = image.vpixel;

   initialize_pixel(pixel, hpixel, vpixel);
}



// -----------------------------------------------------------------------------
// object_border_begin
// object_border_end
// -----------------------------------------------------------------------------

// object_border_shape, for object_border_begin
template<class SHAPEVEC>
inline void object_border_shape(const SHAPEVEC &shape)
{
   for (size_t n = shape.size();  n--; ) {
      const typename SHAPEVEC::value_type &s = shape[n];
      s.mend.imin = s.mend.iend = 0;
      s.mend.jmin = s.mend.jend = 0;
   }
}

// object_border_begin
template<class real, class base, class color>
inline void object_border_begin(
   const model<real,base> &model,
   image<real,color> &image
) {
   if (image.border.object) {
      #define kip_border(type) object_border_shape(model.type)
      kip_expand_semi(kip_border)
      #undef kip_border
   }
}



// object_border_shape, for object_border_end
template<class SHAPEVEC, class real, class color>
inline void object_border_shape(const SHAPEVEC &shape, image<real,color> &image)
{
   // re: dotted-line computation
   const unsigned small = image.border.small;
   const unsigned large = image.border.large;

   // for each shape
   for (size_t n = shape.size();  n--; ) {
      // the shape
      const typename SHAPEVEC::value_type &s = shape[n];

      // its bounds: min and end
      const size_t imin = s.mend.imin, iend = s.mend.iend;
      const size_t jmin = s.mend.jmin, jend = s.mend.jend;

      // if we want to draw its bounds...
      if (s.on && imin < iend && jmin < jend) {
         const size_t imax = iend-1;  assert(imax < image.hpixel);
         const size_t jmax = jend-1;  assert(jmax < image.vpixel);

         for (size_t i = imin;  i < iend;  ++i)
            if ((i % large) < small)
               image(i,jmin) = image(i,jmax) = color(s.base());///color::border();
         for (size_t j = jmin;  j < jend;  ++j)
            if ((j % large) < small)
               image(imin,j) = image(imax,j) = color(s.base());///color::border();
      }
   }
}

// object_border_end
template<class real, class base, class color>
inline void object_border_end(
   const model<real,base> &model,
   image<real,color> &image
) {
   if (image.border.object) {
      #define kip_border(type) object_border_shape(model.type, image)
      kip_expand_semi(kip_border)
      #undef kip_border
   }
}



// -----------------------------------------------------------------------------
// trace_uniform
// Prepare vars.uniform (the bins)
// Note: vars.uniform = array<2> of vector of {min,shape}
// -----------------------------------------------------------------------------

template<class base, class real, class color, class pix>
inline void trace_uniform(
         model <real,base > &model,   // input
   const view  <real      > &view,    // input
   const light <real      > &light,   // input
   const engine<real      > &engine,  // input
         image <real,color> &image,   // input/output
         vars  <real,base > &vars,    // auxiliary
         array<2,pix>  &pixel    // per-pixel information
) {
   // clear *all* bins (each contains an array; could be lots of memory
   // waste if we cleared *only* up to the currently needed size)
   for (size_t b = vars.uniform.size();  b-- ; )
      vars.uniform[b].clear();

   // resize (not upsize); speeds future clears above; note that separate
   // clear loop above was necessary, as resize does nothing if same size
   vars.uniform.resize(engine.hzone, engine.vzone);

   // compute vars.[hv]rat[sub]
   vars.hrat = real(image.hpixel)/real(engine.hzone);
   vars.vrat = real(image.vpixel)/real(engine.vzone);
   vars.hratsub = vars.hrat/real(engine.hsub);
   vars.vratsub = vars.vrat/real(engine.vsub);

   // preprocess, trace
   usetup(model,       light, engine, vars, image.border.object);
   utrace(model, view, light, engine, vars, image, pixel);
}



// -----------------------------------------------------------------------------
// trace_recursive
// -----------------------------------------------------------------------------

template<class base, class real, class color, class pix>
inline void trace_recursive(
         model <real,base > &model,   // input
   const view  <real      > &view,    // input
   const light <real      > &light,   // input
   const engine<real      > &engine,  // input
         image <real,color> &image,   // input/output
         vars  <real,base > &vars,    // auxiliary
         array<2,pix>  &pixel    // per-pixel information
) {
   const size_t hpixel = image.hpixel;
   const size_t vpixel = image.vpixel;

   // bookkeeping
   static shape_vectors<real,base> sv;
   sv.clear();
   sv.reserve(model);

   // preprocess, trace
   rsetup(model,light, engine,vars, sv);
   /*
   // 2017-04-05
   std::cout << "kip: trace_recursive, sv.size() == " << sv.size() << std::endl;
   */
   rtrace(
      view, engine,image, vars,light, pixel,
     -vars.hmax, vars.hmax, 0, hpixel,
     -vars.vmax, vars.vmax, 0, vpixel, sv, true  // true = rootlevel
   );
}



// -----------------------------------------------------------------------------
// trace_block
// -----------------------------------------------------------------------------

template<class base, class real, class color, class pix>
inline void trace_block(
         model <real,base > &model,   // input
   const view  <real      > &view,    // input
   const light <real      > &light,   // input
   const engine<real      > &engine,  // input
         image <real,color> &image,   // input/output
         vars  <real,base > &vars,    // auxiliary
         array<2,pix>  &pixel    // per-pixel information
) {
   (void)model;  (void)view;  (void)light;  (void)engine;
   (void)image;  (void)vars;  (void)pixel;

   // qqq This case needs lots of updating. We should have another boolean
   // parameter, sent through the trace() functions, to indicate new scene.
   // Also, we should have "first" to indicate first-time.
   assert(false);

   const size_t cap = vars.block.size();
   if (cap == 0) {
      // initialize block --- should just do once for a given model.
      // qqq Do this better later; should check that size and xzone,
      // yzone, and zzone are consistent if size != 0.
      vars.block.resize(engine.xzone, engine.yzone, engine.zzone);
      #ifdef KIP_BLOCK
         bsetup(model, engine, vars);
      #endif
   }

   #ifdef KIP_BLOCK
      btrace(model, view, light, engine, image, vars, pixel);
   #endif
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// trace
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
// trace (called internally)
// -----------------------------------------------------------------------------

template<class base, class real, class color, class pix>
bool trace(
         model <real,base > &model,   // input
   const view  <real      > &view,    // input
   const light <real      > &light,   // input
   const engine<real      > &engine,  // input
         image <real,color> &image,   // input/output
         vars  <real,base > &vars,    // auxiliary
         array<2,pix>  &pixel    // per-pixel information
) {
   // Set number of threads
   set_nthreads(get_nthreads());

   // Initializations
   trace_vars  (model, view, light, engine, image, vars);
   ///   trace_vipt  (model,view,light,engine,image, vars);
   trace_vipt  (view, engine, image, vars);
   trace_bitmap(image);
   trace_pixel (image, pixel);

   // Initialize object bounds, if appropriate
   object_border_begin(model,image);

   // Select method...
   if (engine.method == method_t::uniform)
      trace_uniform  (model,view,light,engine,image, vars,pixel);
   else if (engine.method == method_t::recursive)
      trace_recursive(model,view,light,engine,image, vars,pixel);
   else
      trace_block    (model,view,light,engine,image, vars,pixel);

   // Draw object bounds, if appropriate
   object_border_end(model,image);

   // Done
   return true;
}

} // namespace internal



// -----------------------------------------------------------------------------
// trace (user-called)
// -----------------------------------------------------------------------------

// trace(model, view, light, engine, image)
template<class real, class base, class color>
inline bool trace(
         model <real,base > &model,   // input
   const view  <real      > &view,    // input
   const light <real      > &light,   // input
   const engine<real      > &engine,  // input
         image <real,color> &image    // input/output
) {
   static array<2,nothing_per_pixel> pixel;

   // qqq Consider putting vars into another class, as "static" makes this
   // particular function non-thread-safe. Then again, perhaps thread safety
   // in this context isn't particularly important.
   static internal::vars<real,base> vars;

   return internal::trace(
      model,  // has no fix()
      view  .fix(),
      light .fix(),
      engine.fix(image.hpixel, image.vpixel),
      image .fix(),
      vars,   // has no fix()
      pixel   // has no fix()
   );
}

// trace(model, view, light, engine, image, pixel)
template<class real, class base, class color, class pix>
inline bool trace(
         model <real,base > &model,   // input
   const view  <real      > &view,    // input
   const light <real      > &light,   // input
   const engine<real      > &engine,  // input
         image <real,color> &image,   // input/output
         array<2,pix>  &pixel    // per-pixel information
) {
   // qqq See above remark
   static internal::vars<real,base> vars;

   return internal::trace(
      model,
      view  .fix(),
      light .fix(),
      engine.fix(image.hpixel, image.vpixel),
      image .fix(),
      vars,
      pixel
   );
}



// trace(scene)
template<class real, class base, class color>
inline bool trace(scene<real,base,color> &s)
{
   return trace(
      s, s, s, s, s  // model, view, light, engine, image
   );
}

// trace(scene,array)
template<class real, class base, class color, class pix>
inline bool trace(scene<real,base,color> &s, array<2,pix> &pixel)
{
   return trace(
      s, s, s, s, s,  // model, view, light, engine, image
      pixel
   );
}
