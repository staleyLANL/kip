
// zzz Need to consider shape.on in this algorithm.

namespace detail {

// -----------------------------------------------------------------------------
// block_setup_shape
// -----------------------------------------------------------------------------

template<class real, class base, class CONTAINER>
void block_setup_shape(
   const model<real,base> &model,
   const engine<real> &engine,
   vars<real,base> &vars,
   const real xfac, const real yfac, const real zfac,
   const CONTAINER &objects
) {
   // To silence compiler warnings if KIP_BLOCK is not #defined
   (void)model;
   (void)engine;
   (void)vars;
   (void)xfac;
   (void)yfac;
   (void)zfac;
   (void)objects;

#ifdef KIP_BLOCK
   // model.aabb was computed earlier, as the objects were entered
   const bbox<real> &aabb = model.aabb;

   // for each object of this shape
   using iter_t = typename CONTAINER::const_iterator;
   const iter_t end = objects.end();
   for (iter_t i = objects.begin();  i != end;  ++i) {

      const bbox<real> b = i->aabb();
      const int  // signed
         xa = op::max(0, int(xfac*(b.xmin - aabb.xmin))),
         ya = op::max(0, int(yfac*(b.ymin - aabb.ymin))),
         za = op::max(0, int(zfac*(b.zmin - aabb.zmin))),
         xb = op::min(int(engine.xzone)-1, int(xfac*(b.xmax - aabb.xmin))),
         yb = op::min(int(engine.yzone)-1, int(yfac*(b.ymax - aabb.ymin))),
         zb = op::min(int(engine.zzone)-1, int(zfac*(b.zmax - aabb.zmin)));

      // drop the current object into the appropriate blocks
      for (int x = xa;  x <= xb;  ++x)
      for (int y = ya;  y <= yb;  ++y)
      for (int z = za;  z <= zb;  ++z)
         vars.block(x,y,z).vec.
            push_back(minimum_and_shape<real,base>(0,*i));
   }

   // Note: neither "reset" for each block(x,y,z), nor "minimum" for each
   // block(x,y,z).vec[*], is initialized here. Those get done when we ray
   // trace --- not during this setup, which just places objects into blocks.
#endif
}



// -----------------------------------------------------------------------------
// bsetup
// -----------------------------------------------------------------------------

template<class real, class base>
inline void bsetup(
   const model<real,base> &model,
   const engine<real> &engine,
   vars<real,base> &vars
) {
   // zzz Could do diag,quad,3060,1575 segmenting of some kind here.

   const real xfac = engine.xzone/(model.aabb.xmax - model.aabb.xmin);
   const real yfac = engine.yzone/(model.aabb.ymax - model.aabb.ymin);
   const real zfac = engine.zzone/(model.aabb.zmax - model.aabb.zmin);

#define kip_make_setup(type)\
   block_setup_shape(model,engine,vars, xfac,yfac,zfac, model.type)
   kip_expand(kip_make_setup,;)
#undef  kip_make_setup
}



// -----------------------------------------------------------------------------
// next
// -----------------------------------------------------------------------------

/*
2019 jul 15
Was there a reason I had these, instead of using
our op::min and op::max functions?

// minref
template<class T>
inline const T &minref(const T &a, const T &b)
   { return a < b ? a : b; }

template<class T>
inline const T &minref(const T &a, const T &b, const T &c)
   { return minref(a,minref(b,c)); }

// maxref
template<class T>
inline const T &maxref(const T &a, const T &b)
   { return a < b ? b : a; }

template<class T>
inline const T &maxref(const T &a, const T &b, const T &c)
   { return maxref(a,maxref(b,c)); }
*/



// next
template<class T>
bool next(
   const T &xmin, const T &xmax, const unsigned xzone, const T &delx,
   const T &ymin, const T &ymax, const unsigned yzone, const T &dely,
   const T &zmin, const T &zmax, const unsigned zzone, const T &delz,
   const T &ax, const T &ay, const T &az,
   const T &bx, const T &by, const T &bz,
   T &qseg, unsigned &i, unsigned &j, unsigned &k, unsigned &status
) {
   static T dx, xbeg, xfac;
   static T dy, ybeg, yfac;
   static T dz, zbeg, zfac;

   if (status == 0) {
      dx = bx - ax;  T qxmin = (xmin - ax)/dx, qxmax = (xmax - ax)/dx;
      dy = by - ay;  T qymin = (ymin - ay)/dy, qymax = (ymax - ay)/dy;
      dz = bz - az;  T qzmin = (zmin - az)/dz, qzmax = (zmax - az)/dz;

      if (qxmax < qxmin) std::swap(qxmin,qxmax);
      if (qymax < qymin) std::swap(qymin,qymax);
      if (qzmax < qzmin) std::swap(qzmin,qzmax);

      // qmin, qmax
      const T qmin = op::max(T(0), maxref(qxmin,qymin,qzmin));
      const T qmax = minref(qxmax,qymax,qzmax);
      if (qmax == T(0) || qmax < qmin) return false;

      // indices of entry point
      i = op::max(0, op::min(int(xzone)-1, int((ax+qmin*dx - xmin)/delx)));
      j = op::max(0, op::min(int(yzone)-1, int((ay+qmin*dy - ymin)/dely)));
      k = op::max(0, op::min(int(zzone)-1, int((az+qmin*dz - zmin)/delz)));

      status = 1;
      return true;
   }

   if (status == 1) {
      // compute xbeg,ybeg,zbeg, xfac,yfac,zfac
      xbeg = (xmin + (dx > T(0))*delx - ax)/dx;  xfac = delx/dx;
      ybeg = (ymin + (dy > T(0))*dely - ay)/dy;  yfac = dely/dy;
      zbeg = (zmin + (dz > T(0))*delz - az)/dz;  zfac = delz/dz;
      status = 2;
   }

   const T x = dx ? xbeg + i*xfac : std::numeric_limits<T>::max();
   const T y = dy ? ybeg + j*yfac : std::numeric_limits<T>::max();
   const T z = dz ? zbeg + k*zfac : std::numeric_limits<T>::max();

   const T *const mm = &minref(x,y,z);
   qseg = *mm;
   return
      (&x != mm || ((dx < T(0) && i--) || (T(0) < dx && ++i < xzone))) &&
      (&y != mm || ((dy < T(0) && j--) || (T(0) < dy && ++j < yzone))) &&
      (&z != mm || ((dz < T(0) && k--) || (T(0) < dz && ++k < zzone)));
}



// -----------------------------------------------------------------------------
// btrace
// Ray trace, using block method.
// -----------------------------------------------------------------------------

// btrace_shape
template<class CONTAINER>
inline void btrace_shape(
   const CONTAINER &objects
) {
   // To silence compiler warnings if KIP_BLOCK is not #defined
   (void)objects;

#ifdef KIP_BLOCK
   using iter_t = typename CONTAINER::const_iterator;
   const iter_t end = objects.end();
   for (iter_t i = objects.begin();  i != end;  ++i)
      i->lastpix = -2;
#endif
}



// btrace
template<class real, class base, class color>
void btrace(
   const model <real,base > &model,
   const view  <real      > &view,
   const light <real      > &light,
   const engine<real      > &engine,
         image <real,color> &image,
         vars  <real,base > &vars
) {
   // To silence compiler warnings if KIP_BLOCK is not #defined
   (void)model;  (void)view;
   (void)light;  (void)engine;
   (void)image;  (void)vars;

#ifdef KIP_BLOCK
   // --------------------------------
   // setup
   // --------------------------------

   // for each block: haven't grabbed object minima, or depth-sorted
   for (unsigned b = 0;  b < engine.xzone*engine.yzone*engine.zzone;  ++b)
      vars.block[b].reset = true;

   // for each object: set lastpix = -2
#define kip_make_trace(type) btrace_shape(model.type)
   kip_expand(kip_make_trace,;)
#undef  kip_make_trace


   // --------------------------------
   // do it   <-- zzz come up with a better comment than this
   // --------------------------------

   const bbox<real> &aabb = model.aabb;
   const real delx = (aabb.xmax - aabb.xmin)/engine.xzone;
   const real dely = (aabb.ymax - aabb.ymin)/engine.yzone;
   const real delz = (aabb.zmax - aabb.zmin)/engine.zzone;

   // because image.hpixel/vpixel are references; compiler might think volatile
   const unsigned hpixel = image.hpixel;
   const unsigned vpixel = image.vpixel;

   // vertical pixels
   real z = -vars.vmax + vars.vhalf;
   for (unsigned v = 0;  v < vpixel;  ++v, z += vars.vfull) {

   // horizontal pixels
   real y = -vars.hmax + vars.hhalf;
   for (unsigned h = 0;  h < hpixel;  ++h, y += vars.hfull) {

      // IN THIS BLOCK FOR EACH INDIVIDUAL PIXEL (h,v)

      // initialization: target, diff
      const point<real> target = vars.eyeball
         + normalize(vars.t2e.back_0nn(y,z) - vars.eyeball);
      const point<real> diff = vars.eyeball - view.target;

      // initialization: q, qmin, ptr
      inq<real,base> q1, *qused = &q1;
      inq<real,base> q2, *qnext = &q2;

      real qmin = std::numeric_limits<real>::max(), qseg = 0;
      const shape<real,base> *ptr = nullptr;

      unsigned i, j, k, status = 0;  const int npix = h + hpixel*v;
      while (
         next(
            aabb.xmin, aabb.xmax, engine.xzone, delx,
            aabb.ymin, aabb.ymax, engine.yzone, dely,
            aabb.zmin, aabb.zmax, engine.zzone, delz,
            vars.eyeball.x, vars.eyeball.y, vars.eyeball.z,
            view.target.x, view.target.y, view.target.z,
            qseg, i, j, k, status
         ) &&
         qmin > qseg
      ) {

         const unsigned seg = i + engine.xzone*(j + engine.yzone*k);

         // bin: the present segment
         using vector_t = std::vector<minimum_and_shape<real,base>>;

         vector_t &bin = vars.block[seg].vec;
         const ulong size = bin.size();
         if (size == 0) continue;

         // sort, if we didn't see this bin with any previous pixels
         if (vars.block[seg].reset) {
            for (ulong obj = 0;  obj < size;  ++obj)
               if (bin[obj].shape->lastpix == -2) {  // was reset only
                  bin[obj].shape->is_operand = false;
                  bin[obj].minimum = bin[obj].shape->minimum =
                     bin[obj].shape->process(vars.eyeball, light[0]);
                  kip_assert(bin[obj].minimum >= 0);

                  bin[obj].shape->lastpix =
                     // Is interior && solid (not just interior) the correct
                     // test here? Lately, I haven't had the block algorithm
                     // working anyway
                        bin[obj].shape->interior && bin[obj].shape->solid
                     ?  std::numeric_limits<int>::max()
                     : -1;
               } else
                  bin[obj].minimum = bin[obj].shape->minimum;

            std::sort(bin.begin(), bin.end(), detail::less<real,base>());
            vars.block[seg].reset = false;
         }

         // zzz 2013-11-12: really call op_first below???
         // loop over objects in this bin
         ulong s = 0;
         for ( ;  s < size;  ++s)
            if (bin[s].shape->lastpix < npix) {
               bin[s].shape->lastpix = npix;
               if (detail::op_first(bin[s].shape,
                                  view.eyeball, view.target, diff,
                                    insub, qmin,*qused)) {
                  const real q = qused->q;
                  kip_assert(0 < q && q < qmin);
                  ptr = bin[s].shape;  qmin = q;  s++;
                  break;
               }
            }

         for ( ;  s < size && bin[s].minimum < qmin;  ++s)
            if (bin[s].shape->lastpix < npix) {
               bin[s].shape->lastpix = npix;
               if (detail::op_first(bin[s].shape,
                                  eyeball, target, diff,
                                    insub, qmin, *qnext)) {
                  const real q = qnext->q;
                  kip_assert(0 < q && q < qmin);
                  ptr = bin[s].shape;  qmin = q;
                  std::swap(qused,qnext);
               }
            }

      }

      // color
      if (ptr)
         image(h,v) = detail::get_color<color>(*qused);
   }
   }
#endif
}

} // namespace detail
