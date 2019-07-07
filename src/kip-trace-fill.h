
#pragma once

namespace kip {

// eps
// 0 = default; means get_eps() will compute something reasonable
inline long double eps = 0;

// get_eps
namespace internal {
   template<class T>
   inline T get_eps()
   {
      return eps
         ? T(eps)
         : std::pow(T(10), T(-0.4)*std::numeric_limits<T>::digits10);
   }
} // namespace internal



/*
    +------------+                                             +------------+
    | .infirst   |                                             | .inall     |
    | (operator) |-----------------------------------------+   | (operator) |
    +------------+                                         |   +------------+
       ^     |  ^                                          |    ^  |
       |     |  |                                          |    |  |
       |     |  |                                          |    |  |
       |     |  |                                          |    |  |
       |     v  |                                          v    |  v
       |  +------------+          +-----------+          +----------+
       |  |  op_first  |--insub-->|  inbound  |<--insub--|  op_all  |
       |  +------------+          +-----------+          +----------+
       |              \                 ^                 /
       |               \                |                /
       |                \               |               /
       |                 \             i,j             /
       |                 \/             |             \/
       |            +-------------+     |     +-------------+
       |            | .infirst    |     |     | .inall      |
       |            | (primitive) |     |     | (primitive) |
       |            +-------------+     |     +-------------+
       |                      /\        |
       |                       \        |
       |                        \       |
       |                         \      |
       |                          \     |
       |                         +-------------+
       +-------------------------|  get_first  |
                                 +-------------+
                                   /\       /\
                                   /         \
                                  /           \
                                 /             \
                                /               \
                   +-------------+             +------------+
                   |  one_plain  |  per-pixel  |  one_anti  |
                   |  two_plain  |     i,j     |  two_anti  |
                   |  max_plain  |   in this   |            |
                   |  any_plain  |     bin     |  any_anti  |
                   +-------------+             +------------+
                          ^                           ^
                          |                           |
                          |                           |
                          |                           |
                          |                           |
               +-------------------+         +------------------+
               |  fill_loop_plain  | per-bin |  fill_loop_anti  |
               +-------------------+         +------------------+
                                /\             /\
                                 \             /
                                  \           /
                                   \         /
                                    \       /
                                 +-------------+
                                 |  trace_bin  |
                                 +-------------+
                                      /\ /\
                                      /   \
                                     /     \
                                    /       \
                                   /         \
                          +----------+     +----------+
                          |  rtrace  |     |  utrace  |
                          +----------+     +----------+
*/

namespace internal {



// -----------------------------------------------------------------------------
// ttclass
// -----------------------------------------------------------------------------

template<template<class,class> class T>
class ttclass { };



// -----------------------------------------------------------------------------
// get_endsorted
// inbound
// -----------------------------------------------------------------------------

// get_endsorted
template<class real>
inline size_t get_endsorted(
   const size_t current,  // already sorted; now sort from [current] on
   const engine<real> &engine,
   const size_t binsize   // total number of values
) {
   return op::min(
      // sort at least sort_min objects, and at least sort_frac fraction...
      current + op::max(
         size_t(1),  // to be safe (although sort_min is >= 1, from fix())
         size_t(engine.sort_min),
         op::rnd<size_t>(engine.sort_frac * real(binsize))
      ),
      // but clip to binsize
      binsize
   );
}



// inbound: are i,j in shape's [min,end)?
template<class real, class tag>
inline bool inbound(
   const shape<real,tag> &obj,
   const size_t i, const size_t j
) {

   // 2017-04-05
   // the four min and end values appear to be largely uninitialized
   /*
   std::cout << "kip: inbound #1" << std::endl;
   std::cout << "kip:    imin == " << obj.mend.imin << std::endl;
   std::cout << "kip:    i    == " << i << std::endl;
   std::cout << "kip:    iend == " << obj.mend.iend << std::endl;
   std::cout << "kip:    jmin == " << obj.mend.jmin << std::endl;
   std::cout << "kip:    j    == " << j << std::endl;
   std::cout << "kip:    jend == " << obj.mend.jend << std::endl;
   */

   if (
     (obj.mend.imin <= i && i < obj.mend.iend &&
      obj.mend.jmin <= j && j < obj.mend.jend)
   ) {
      return true;
   } else {
      return false;
   }

   /*
short version
   return
      obj.mend.imin <= i && i < obj.mend.iend &&
      obj.mend.jmin <= j && j < obj.mend.jend;
   */
}



// inbound: are i,j (from insub) in shape's [min,end)?
template<class real, class tag>
inline bool inbound(
   const shape<real,tag> &obj,
   const subinfo &insub
) {
   // 2017-04-05
   /*
   std::cout << "kip: inbound #2" << std::endl;
   std::cout << "kip:    imin == " << insub.mend.imin << std::endl;
   std::cout << "kip:    i    == " << insub.i << std::endl;
   std::cout << "kip:    iend == " << insub.mend.iend << std::endl;
   std::cout << "kip:    jmin == " << insub.mend.jmin << std::endl;
   std::cout << "kip:    j    == " << insub.j << std::endl;
   std::cout << "kip:    jend == " << insub.mend.jend << std::endl;
   */

   (void)obj;  // ccc Need to make this have valid values for non-root shapes!

   // ccc remove later...
   if (insub.mend.imin <= insub.i && insub.i < insub.mend.iend &&
       insub.mend.jmin <= insub.j && insub.j < insub.mend.jend
   ) {
      return true;
   } else {
      assert(false);///
      return false;
   }

   /*
   // ccc put this back in...
   return inbound(obj,insub.i,insub.j);
   */
}



// -----------------------------------------------------------------------------
// Globally-called:
//    get_first
// -----------------------------------------------------------------------------

template<
   template<class,class> class SHAPE,
   class BIN, class real, class tag
>
inline bool get_first(
   const ttclass<SHAPE> &,

   const BIN &bin,
   const unsigned s,
   const size_t i, const size_t j,
   const size_t zone,

#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> &etd,
#else
   const point<real> &eyeball,
   const point<real> &target,
   const point<real> &diff,
#endif

   const real &qmin, inq<real,tag> &q
) {
   const SHAPE<real,tag> &obj = *(const SHAPE<real,tag> *)bin[s].shape;
   if (!inbound(obj,i,j)) return false;

   #ifdef KIP_FUZZY
      const real eps = get_eps<real>();
      static unsigned num = 0;

      if (qmin == std::numeric_limits<real>::max())
         num = 0;

      if (obj.infirst(
#ifdef KIP_TOGETHER
             etd,
#else
             eyeball, target, diff,
#endif
             qmin, q, subinfo(i,j, unsigned(zone), obj.mend)
          )
      ) {
         q = real(q)*(1 + (2*random<real>()-1)*eps);
         return q < qmin*(1-eps)
            ? (num = 1)
            : ++num == 1 || num*random<real>() < 1;
      }

      return false;
   #else
      return
         obj.infirst(
#ifdef KIP_TOGETHER
            etd,
#else
            eyeball, target, diff,
#endif
            qmin, q, subinfo(i,j, unsigned(zone), obj.mend)
         );
   #endif
}



// -----------------------------------------------------------------------------
// Operator-called:
//    op_first
//    op_all
// -----------------------------------------------------------------------------

// op_first
template<class real, class tag>
inline bool op_first(
   // shape
   const shape<real,tag> *const s,

   // eyeball, target, diff
#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> &etd,
#else
   const point<real> &eyeball,
   const point<real> &target,
   const point<real> &diff,
#endif

   // qmin, q
   const real &qmin,
   inq<real,tag> &q,

   // insub
   const subinfo &insub
) {
   const shape<real,tag> &obj = *s;
   if (!inbound(obj,insub)) return false;

   #ifdef KIP_FUZZY
      const real eps = get_eps<real>();
      if (!(s->infirst(kip_etd, qmin,q, insub)))
         return false;
      return real(q = real(q)*(1 + (2*random<real>()-1)*eps)) < qmin;
   #endif

   // general shape
   return obj.infirst(kip_etd, qmin,q, insub);

   // Equivalent inall() call; useful for comparing first vs. all speed:
   // afew<inq<real,tag>> ints;  ints.reset();
   // return obj.inall(kip_etd, qmin,ints, insub)
   //    ? q = ints[0], true
   //    : false;
}



// op_all
template<class real, class tag>
inline bool op_all(
   // shape
   const shape<real,tag> *const s,

   // eyeball, target, diff
#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> &etd,
#else
   const point<real> &eyeball,
   const point<real> &target,
   const point<real> &diff,
#endif

   // qmin, q
   const real &qmin,
   afew<inq<real,tag>> &q,

   // insub
   const subinfo &insub
) {
   const shape<real,tag> &obj = *s;
   if (!inbound(obj,insub)) return false;

   #ifdef KIP_FUZZY
      const real eps = get_eps<real>();
      if (!(obj.inall(kip_etd, qmin,q, insub)))
         return false;

      const unsigned size = q.size();
      for (unsigned n = size;  n--; )
         q[n] = real(q[n])*(1 + (2*random<real>()-1)*eps);

      q.sort();
      for (unsigned n = size;  n--; )
         if (!(q[n] < qmin)) q.setsize(q.size()-1); else break;

      return q.size();
   #else
      return obj.inall(kip_etd, qmin,q, insub);
   #endif
}



// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

// kip_fill_plain
#define kip_fill_plain(name)\
   template<\
      template<class,class> class SHAPE,\
      class real, class color, class base, class pix\
   >\
   class name {\
   public:\
   inline void operator()(\
      const engine<real> &engine, const image<real,color> &image,\
      std::vector<minimum_and_shape<real,base>> &bin,\
      size_t &endsorted, const size_t binsize, const real &maximum,\
      inq<real,base> &qa, inq<real,base> *qa_ptr,\
      inq<real,base> &qb, inq<real,base> *qb_ptr,\
      color *const ptr,\
      unsigned &prev,\
      const vars<real,base> &vars, const point<real> &light,\
      const point<real> &target, const point<real> &diff,\
      pix &pixel, \
      const size_t i, const size_t j, const size_t zone\
   ) const {\
      (void)engine;  (void)image; (void)bin;    (void)endsorted; (void)binsize;\
      (void)maximum; (void)qa;    (void)qa_ptr; (void)qb;        (void)qb_ptr;\
      (void)prev;    (void)vars;  (void)light;  (void)target;    (void)diff;\
      (void)i;       (void)j;     (void)zone;



// kip_fill_anti
#define kip_fill_anti(name)\
   template<\
      template<class,class> class SHAPE,\
      class real, class color, class base, class pix\
   >\
   class name {\
   public:\
   inline bool operator()(\
      const engine<real> &engine, const image<real,color> &image,\
      std::vector<minimum_and_shape<real,base>> &bin,\
      size_t &endsorted, const size_t binsize, const real &maximum,\
      inq<real,base> &qa, inq<real,base> *qa_ptr,\
      inq<real,base> &qb, inq<real,base> *qb_ptr,\
      const real h, const real v, RGBA<unsigned> &sum,\
      const vars<real,base> &vars, const light<real> &light,\
      pix &pixel, \
      const size_t i, const size_t j, const size_t zone\
   ) const {\
      (void)engine;  (void)image; (void)bin;    (void)endsorted; (void)binsize;\
      (void)maximum; (void)qa;    (void)qa_ptr; (void)qb;        (void)qb_ptr;\
      (void)h;       (void)v;     (void)sum;    (void)vars;      (void)i;\
      (void)j;       (void)zone;



// kip_fill_end
#define kip_fill_end } };

// kip_param
#ifdef KIP_TOGETHER
   #define kip_param(s,qmin,q)\
      ttclass<SHAPE>(), bin, s, i, j, zone,\
      etd, qmin, q
#else
   #define kip_param(s,qmin,q)\
      ttclass<SHAPE>(), bin, s, i, j, zone,\
      vars.eyeball, target, diff, qmin, q
#endif



// -----------------------------------------------------------------------------
// Plain
// Each of these is called with a particular pixel (i,j), and a particular bin;
// and each loops over the bin's objects.
// -----------------------------------------------------------------------------

// one_plain
kip_fill_plain(one_plain) {

#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> etd(vars.eyeball, target, diff);
#endif

   get_first(kip_param(0,maximum,qa))
    ? (*ptr = get_color<color>(vars.eyeball, light, qa, pixel))
    :  *ptr;

} kip_fill_end



// two_plain
kip_fill_plain(two_plain) {

#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> etd(vars.eyeball, target, diff);
#endif

   get_first(kip_param(0,maximum,qa))
 ? bin[1].minimum < qa.q &&
   get_first(kip_param(1,qa.q,   qb))
    ? (*ptr = get_color<color>(vars.eyeball, light, qb, pixel))
    : (*ptr = get_color<color>(vars.eyeball, light, qa, pixel))
 : get_first(kip_param(1,maximum,qb))
    ? (*ptr = get_color<color>(vars.eyeball, light, qb, pixel))
    :  *ptr;

} kip_fill_end



// max_plain
kip_fill_plain(max_plain) {

#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> etd(vars.eyeball, target, diff);
#endif

   for (unsigned s = 0;  s < binsize;  ++s)
      if (get_first(kip_param(s,maximum,qa))) {
         for ( ; ++s < binsize && bin[s].minimum < *qa_ptr ; )
            if (get_first(kip_param(s,real(*qa_ptr),*qb_ptr)))
               std::swap(qa_ptr,qb_ptr);
        *ptr = get_color<color>(vars.eyeball, light, *qa_ptr, pixel);
         return;
      }

} kip_fill_end



// any_plain
kip_fill_plain(any_plain) {

#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> etd(vars.eyeball, target, diff);
#endif

   unsigned s = 0;  bool found;  qa = maximum;
   if (prev > 3) {  // I tried several values, and this number works well

      // examine bin[prev]
      found = get_first(kip_param(prev,real(*qa_ptr),*qb_ptr));
      unsigned newprev = found ? std::swap(qa_ptr,qb_ptr), prev : 0;

      // examine bin[0] through bin[prev-1]; fall-through iff prev == 0
      for ( ;  s < prev;  ++s)
         if (get_first(kip_param(s,real(*qa_ptr),*qb_ptr)))
            std::swap(qa_ptr,qb_ptr), newprev = s, found = true;

      // examine bin[prev+1] forward
      s = prev+1;
      for (;;) {
         for ( ;  s < endsorted && (!found || bin[s].minimum < *qa_ptr);  ++s)
            if (get_first(kip_param(s,real(*qa_ptr),*qb_ptr)))
               std::swap(qa_ptr,qb_ptr), newprev = s, found = true;

         // break if found something, or if there's no more to sort
         if (s != endsorted || endsorted == binsize) break;

         // sort more
         endsorted = get_endsorted(endsorted, engine, binsize);
         std::partial_sort(bin.begin()+s, bin.begin()+long(endsorted),
                           bin.end(), less<real,base>());
      }
      prev = newprev;

   } else {

      // for the present pixel (i,j), loop over objects in the present bin
      found = false;
      for ( ;  s < endsorted;  ++s)
         if (get_first(kip_param(s,real(*qa_ptr),*qb_ptr)))
            { std::swap(qa_ptr,qb_ptr);  prev = s++;  found = true;  break; }

      for (;;) {
         for ( ;  s < endsorted && bin[s].minimum < *qa_ptr;  ++s)
            if (get_first(kip_param(s,real(*qa_ptr),*qb_ptr)))
               std::swap(qa_ptr,qb_ptr), prev = s, found = true;

         // break if found something, or if there's no more to sort
         if (s != endsorted || endsorted == binsize) break;

         // sort more
         endsorted = get_endsorted(endsorted, engine, binsize);
         std::partial_sort(bin.begin()+s, bin.begin()+long(endsorted),
                           bin.end(), less<real,base>());
      }
   }

   // color
   found
    ? (*ptr = get_color<color>(vars.eyeball, light, *qa_ptr, pixel))
    :  *ptr;

} kip_fill_end



// -----------------------------------------------------------------------------
// Anti
// Same remark as for the "Plain" functions above; and in addition, we shoot
// several rays *within* the pixel.
// -----------------------------------------------------------------------------

// one_anti
kip_fill_anti(one_anti) {

   bool found = false;
   for (unsigned k = 0;  k < image.anti;  ++k)
   for (unsigned l = 0;  l < image.anti;  ++l) {
      // initialization
      const point<real> diff = normalize(
         vars.eyeball - vars.t2e.back_0nn(
            h-vars.hhalf+(vars.hhalf+real(k)*vars.hfull)*vars.rec_anti(real()),
            v-vars.vhalf+(vars.vhalf+real(l)*vars.vfull)*vars.rec_anti(real()))
      ),
      target = vars.eyeball - diff;
      #ifdef KIP_TOGETHER
         const kip::eyetardiff<real> etd(vars.eyeball, target, diff);
      #endif

      // examine object
      sum += get_first(kip_param(0,maximum,qa))
          ? (found = true, get_color<color>(vars.eyeball, light[0], qa, pixel))
          :  image.background;
   }
   return found;

} kip_fill_end



// two_anti
kip_fill_anti(two_anti) {

   bool found = false;
   for (unsigned k = 0;  k < image.anti;  ++k)
   for (unsigned l = 0;  l < image.anti;  ++l) {
      // initialization
      const point<real> diff = normalize(
         vars.eyeball - vars.t2e.back_0nn(
            h-vars.hhalf+(vars.hhalf+real(k)*vars.hfull)*vars.rec_anti(real()),
            v-vars.vhalf+(vars.vhalf+real(l)*vars.vfull)*vars.rec_anti(real()))
      ),
      target = vars.eyeball - diff;
      #ifdef KIP_TOGETHER
         const kip::eyetardiff<real> etd(vars.eyeball, target, diff);
      #endif

      // examine objects
      sum += get_first(kip_param(0,maximum,qa))
           ? get_first(kip_param(1,qa.q,   qb))
                // [1] is better
              ? (found=true, get_color<color>(vars.eyeball,light[0],qb,pixel))
                // [0] is better (or only)...
              : (found=true, get_color<color>(vars.eyeball,light[0],qa,pixel))
           : get_first(kip_param(1,maximum,qa))
                // [1] is only...
              ? (found=true, get_color<color>(vars.eyeball,light[0],qa,pixel))
                // neither...
              :  image.background;
   }
   return found;

} kip_fill_end



/*
   const engine<real      > &engine,
   const image <real,color> &image,
   std::vector<minimum_and_shape<real,base>> &bin,
         size_t &endsorted,
   const size_t  binsize,
   const real   &maximum,
   inq<real,base> &qa, inq<real,base> *qa_ptr,
   inq<real,base> &qb, inq<real,base> *qb_ptr,
   const real h,  // "hcent"
   const real v,  // "vcent"
   RGBA<unsigned> &sum,  
   const vars  <real,base > &vars,
   const light <real      > &light,
   pix &pixel,
   const size_t i,
   const size_t j, const size_t zone
*/

// any_anti
kip_fill_anti(any_anti) {

   // zzz Should make this optimization in the earlier two functions.
   // zzz For that matter, all of this could be simplified further,
   // zzz with certain code put/changed elsewhere.
   //    542.91 sec, 1140248 kb
   //    539.93 sec, 1162492 kb
   //    529.91 sec, 1142612 kb
   // zzz IMPORTANT NOTE: still need to figure out what's wrong with
   // this function, then stop using scene.sort_min = 10000000.

   const real hh = h + vars.hhalf*(vars.rec_anti(real())-1);
   const real vv = v + vars.vhalf*(vars.rec_anti(real())-1);

   const real hmult = vars.hfull * vars.rec_anti(real());
   const real vmult = vars.vfull * vars.rec_anti(real());

   bool found = false;

   for (unsigned k = 0;  k < image.anti;  ++k)   // horizontal
   for (unsigned l = 0;  l < image.anti;  ++l) { // vertical
      // initialization
      const point<real>
         diff = normalize(
            vars.eyeball - vars.t2e.back_0nn(
               hh + real(k)*hmult,
               vv + real(l)*vmult
         )),
         target = vars.eyeball - diff;
      #ifdef KIP_TOGETHER
      const kip::eyetardiff<real> etd(vars.eyeball, target, diff);
      #endif

      // loop over objects in this bin
      unsigned s = 0;  bool f = false;
      for ( ;  s < endsorted;  ++s)
         if (get_first(kip_param(s,maximum,*qb_ptr)))
            { std::swap(qa_ptr,qb_ptr);  f = true;  s++;  break; }

      for (;;) {
         for ( ;  s < endsorted && bin[s].minimum < *qa_ptr;  ++s)
            if (get_first(kip_param(s,real(*qa_ptr),*qb_ptr)))
               std::swap(qa_ptr,qb_ptr), f = true;

         // break if found something, or if there's no more to sort
         if (s != endsorted || endsorted == binsize) break;

         // sort more
         endsorted = get_endsorted(endsorted, engine, binsize);
         std::partial_sort(bin.begin()+s, bin.begin()+long(endsorted),
                           bin.end(), less<real,base>());
      }

      if (f) {
         found = true;
         sum += get_color<color>(vars.eyeball,light[0], *qa_ptr, pixel);
      } else {
         sum += image.background;
      }
   }

   return found;

} kip_fill_end



// -----------------------------------------------------------------------------
// fill_loop_plain
// fill_loop_lean (like fill_loop_plain, sans image.prior.targets)
// fill_loop_anti
//
// Loop over pixels i,j with indices [imin,iend) x [jmin,jend)
// -----------------------------------------------------------------------------

// fill_loop_plain
// For each (i,j), calls one of:
//    one_plain()
//    two_plain()
//    max_plain()
//    any_plain()
// Each loops over individual shapes, for a specific pixel.

template<
   template<class,class> class SHAPE,
   class real, class color, class base, class pix,
   class ACTION
>
inline void fill_loop_plain(
   const ttclass<SHAPE> &,
   const engine<real> &engine, image<real,color> &image,
   const vars<real,base> &vars, const light<real> &light,

   const size_t imin, const size_t iend,
   const size_t jmin, const size_t jend,
   const size_t zone, const real &maximum,

   std::vector<minimum_and_shape<real,base>> &bin,
   size_t endsorted, const size_t binsize,
   inq<real,base> &qa, inq<real,base> &qb,

   array<2,pix> &pixel,
   const ACTION &action
) {
   // vertical pixels in the current bin...
   for (size_t j = jmin;  j < jend;  ++j) {
      const point<real> *tar = &image.prior.targets(imin,j);
      color *ptr = &image(imin,j);  unsigned prev = 0;
      pix *p = &pixel(imin,j);

      // horizontal pixels in the current bin...
      for (size_t i = imin;  i < iend;  ++i, ++tar, ++ptr, ++p) {
         const point<real> target = vars.t2e.back(*tar);

         action(
            engine,image, bin,endsorted,binsize,
            maximum, qa,&qa, qb,&qb, ptr,prev,
            vars, light[0],
            target, vars.eyeball-target, *p,
            i,j, zone
         );
      }
   }
}



// fill_loop_lean
template<
   template<class,class> class SHAPE,
   class real, class color, class base, class pix,
   class ACTION
>
inline void fill_loop_lean(
   const ttclass<SHAPE> &,

   const view  <real      > &view,
   const engine<real      > &engine,
         image <real,color> &image,
   const vars  <real,base > &vars,
   const light <real      > &light,

   const size_t imin, const size_t iend,
   const size_t jmin, const size_t jend,
   const size_t zone, const real &maximum,

   std::vector<minimum_and_shape<real,base>> &bin,
   size_t endsorted, const size_t binsize,
   inq<real,base> &qa, inq<real,base> &qb,

   array<2,pix> &pixel,
   const ACTION &action
) {
   // qqq Should compute these at a higher level
   const real hmin = vars.hhalf - vars.hmax;
   const real vmin = vars.vhalf - vars.vmax, dsq = view.d*view.d;

   // vertical pixels in the current bin...
   for (size_t j = jmin;  j < jend;  ++j) {
      const real v = vmin + real(j   )*vars.vfull, tmp = dsq + v*v;
      /* */ real h = hmin + real(imin)*vars.hfull;
      color *ptr = &image(imin,j);  unsigned prev = 0;
      pix   *p   = &pixel(imin,j);

      // horizontal pixels in the current bin...
      for (size_t i = imin;  i < iend;  ++i, h += vars.hfull, ++ptr, ++p) {
         // a=(d,0,0), b=(0,h,v), (x,y,z)=a+(b-a)/mod(b-a)
         const real norm = real(1)/op::sqrt(tmp + h*h);
         const point<real> target =
            vars.t2e.back(view.d*(1-norm), h*norm, v*norm);
         action(
            engine,image, bin,endsorted,binsize,
            maximum, qa,&qa, qb,&qb, ptr,prev,
            vars, light[0],
            target, vars.eyeball-target, *p,
            i,j, zone
         );
      }
   }
}



// fill_loop_anti
// For each (i,j), calls one of:
//    one_anti()
//    two_anti()
//    any_anti()
// Each loops over individual shapes, for a specific pixel.

// qqq Figure out good antialiasing treatment of per-pixel information
template<
  template<class,class> class SHAPE,
  class real, class color, class base, class pix, class size_t, class ACTION
>
inline void fill_loop_anti(
   const ttclass<SHAPE> &,

   const engine<real      > &engine,
         image <real,color> &image,
   const vars  <real,base > &vars,
   const light <real      > &light,

   const real &hcent, const size_t imin, const size_t iend,
         real  v,     const size_t jmin, const size_t jend,

   const size_t zone,
   const real &maximum,

   std::vector<minimum_and_shape<real,base>> &bin,
   size_t endsorted, const size_t binsize,
   inq<real,base> &qa, inq<real,base> &qb,

   array<2,pix> &pixel,
   const ACTION &action
) {
   real h = hcent;

   // vertical pixels in the current bin...
   for (size_t j = jmin;  j < jend;  ++j, v += vars.vfull, h = hcent) {
      color *ptr = &image(imin,j);
      pix   *p   = &pixel(imin,j);

      // horizontal pixels in the current bin...
      for (size_t i = imin;  i < iend;  ++i, h += vars.hfull, ++ptr, ++p) {
         RGBA<unsigned> sum(0,0,0);  // qqq don't hardcode RGBA, here/elsewhere

         // one_anti()
         // two_anti()
         // any_anti()
         if (action(
            engine, image, bin, endsorted, binsize,
            maximum, qa, &qa, qb, &qb, h, v, sum, vars, light, *p,
            i, j, zone
         ))
            *ptr = op::div<unsigned char>(sum,vars.anti2);

         /*
         action(
            engine,image, bin, endsorted,binsize,
            maximum, qa,&qa, qb,&qb, h,v, sum, vars,light, *p,
            i,j, zone
         )
         ? (*ptr = op::round<unsigned char>(sum * vars.rec_anti2(float())))
         :  *ptr;
         */
      }
   }
}



// -----------------------------------------------------------------------------
// bin_border
// trace_bin
// -----------------------------------------------------------------------------

// bin_border
template<class real, class color>
inline void bin_border(
   image<real,color> &image,
   const size_t imin, const size_t iend,
   const size_t jmin, const size_t jend, const color &border
) {
   for (size_t j=jmin; j<jend; ++j) image(imin,j) = image(iend-1,j) = border;
   for (size_t i=imin; i<iend; ++i) image(i,jmin) = image(i,jend-1) = border;
}



// trace_bin
template<
   template<class,class> class SHAPE,
   class real, class color, class base, class pix
>
void trace_bin(
   const engine<real      > &engine,
   const view  <real      > &view,
         image <real,color> &image,
   const vars  <real,base > &vars,
   const light <real      > &light,
   array<2,pix> &pixel,

   size_t imin, size_t iend,
   size_t jmin, size_t jend,
   const size_t zone, const size_t max_binsize,

   std::vector<minimum_and_shape<real,base>> &bin,
   const size_t binsize
) {
   // border?
   if (image.border.bin)
      bin_border(image, imin++,iend--, jmin++,jend--,
                 color::border(binsize, max_binsize));

   // macros
   #define kip_action_plain(functor)\
   fill_loop_plain(ttclass<SHAPE>(),\
      engine,image,vars,light,\
      imin,iend, jmin,jend, zone,maximum,\
      bin, endsorted,binsize, qa,qb, pixel,\
      functor<SHAPE,real,color,base,pix>())

   #define kip_action_lean(functor)\
   fill_loop_lean(ttclass<SHAPE>(),\
      view,engine,image,vars,light,\
      imin,iend, jmin,jend, zone,maximum,\
      bin, endsorted,binsize, qa,qb, pixel,\
      functor<SHAPE,real,color,base,pix>())

   #define kip_action_anti(functor)\
   fill_loop_anti (ttclass<SHAPE>(),\
      engine,image,vars,light,\
      hcent,imin,iend, vcent,jmin,jend, zone,maximum,\
      bin, endsorted,binsize, qa,qb, pixel,\
      functor<SHAPE,real,color,base,pix>())

   // binsize-dependent [partial-]sorting actions
   using diff_t =
      typename std::vector<minimum_and_shape<real,base>>::difference_type;
   size_t endsorted = 0;

   if (binsize == 2) {
      if (bin[1].minimum < bin[0].minimum)
         std::swap(bin[0], bin[1]);
   } else if (binsize > 2) {
      endsorted = get_endsorted(0, engine, binsize);
      std::partial_sort(
         bin.begin(), bin.begin()+diff_t(endsorted), bin.end(),
         less<real,base>()
      );
   }

   // fill each pixel in the bin
   inq<real,base> qa, qb;
   const real maximum = std::numeric_limits<real>::max();

   if (image.anti < 2) {
      // One-ray-per-pixel case

      // action
      if (engine.lean) {
         if (binsize == 1)
            kip_action_lean(one_plain);
         else if (binsize == 2)
            kip_action_lean(two_plain);
         else if (binsize == endsorted)
            kip_action_lean(max_plain);
         else
            kip_action_lean(any_plain);
      } else {
         if (binsize == 1)
            kip_action_plain(one_plain);
         else if (binsize == 2)
            kip_action_plain(two_plain);
         else if (binsize == endsorted)
            kip_action_plain(max_plain);
         else
            kip_action_plain(any_plain);
      }

   } else {
      // Antialiasing case

      /*
      if (binsize == 1) {
         for (size_t j = jmin;  j < jend;  ++j)
         for (size_t i = imin;  i < iend;  ++i)
            image(i,j) = rgb(255,0,0);
      } else if (binsize == 2) {
         for (size_t j = jmin;  j < jend;  ++j)
         for (size_t i = imin;  i < iend;  ++i)
            image(i,j) = rgb(0,255,0);
      } else {
         for (size_t j = jmin;  j < jend;  ++j)
         for (size_t i = imin;  i < iend;  ++i)
            image(i,j) = rgb(0,0,255);
      }
      */

      // center of pixel (imin,jmin)
      const real hcent = real(imin)*vars.hfull - vars.hmax + vars.hhalf;
      const real vcent = real(jmin)*vars.vfull - vars.vmax + vars.vhalf;

      // action
      if (binsize == 1)
         kip_action_anti(one_anti);
      else if (binsize == 2)
         kip_action_anti(two_anti);
      else
         kip_action_anti(any_anti);
   }
}

} // namespace internal
} // namespace kip

#undef kip_action_plain
#undef kip_action_lean
#undef kip_action_anti
#undef kip_fill_plain
#undef kip_fill_anti
#undef kip_fill_end
#undef kip_param
