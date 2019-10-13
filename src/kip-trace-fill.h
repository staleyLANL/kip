
/*
    +------------+                                             +------------+
    | .infirst   |                                             | .inall     |
    | (operator) |----------------------------------------+    | (operator) |
    +------------+                                        |    +------------+
       ^     |  ^                                         |     ^  |
       |     |  |                                         |     |  |
       |     v  |                                         v     |  v
       |  +------------+          +-----------+          +----------+
       |  |  op_first  |--insub-->|  inbound  |<--insub--|  op_all  |
       |  +------------+          +-----------+          +----------+
       |              \                 ^                 /
       |               \                |                /
       |                v              i,j              v
       |            +-------------+     |     +-------------+
       |            | .infirst    |     |     | .inall      |
       |            | (primitive) |     |     | (primitive) |
       |            +-------------+     |     +-------------+
       |                        ^       |
       |                         \      |
       |                          \     |
       |                         +-------------+
       +-------------------------|  get_first  |
                                 +-------------+
                                  ^           ^ 
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
               +-------------------+         +------------------+
               |  fill_loop_plain  | per-bin |  fill_loop_anti  |
               +-------------------+         +------------------+
                                 ^             ^
                                  \           /
                                   \         /
                                 +-------------+
                                 |  trace_bin  |
                                 +-------------+
                                     ^     ^ 
                                    /       \
                                   /         \
                          +----------+     +----------+
                          |  rtrace  |     |  utrace  |
                          +----------+     +----------+
*/



// -----------------------------------------------------------------------------
// misc
// -----------------------------------------------------------------------------

// eps
// 0 = default; means epsilon(), below, will compute something reasonable
inline long double eps = 0;

// for the rest of this file...
namespace detail {

// epsilon
template<class real>
inline real epsilon()
{
   // fixme 2019-07-07.
   // Look more closely, but this may be called in a loop;
   // we should handle it more efficiently
   return eps
      ? real(eps)
      : std::pow(real(10), real(-0.4)*std::numeric_limits<real>::digits10);
}



// -----------------------------------------------------------------------------
// get_endsorted
// inbound
// -----------------------------------------------------------------------------

// get_endsorted
template<class real>
inline ulong get_endsorted(
   const ulong current,  // already sorted; now sort from [current] on
   const engine<real> &engine,
   const ulong binsize   // total number of values
) {
   return op::min(
      // sort at least sort_min objects, and at least sort_frac fraction...
      current + op::max(
         ulong(1),  // to be safe (although sort_min is >= 1, from fix())
         ulong(engine.sort_min),
         op::round<ulong>(engine.sort_frac * real(binsize))
      ),
      // but clip to binsize
      binsize
   );
}



// inbound: are i,j in shape's [min,end)?
template<class real, class tag>
inline bool inbound(
   const shape<real,tag> &obj,
   const ulong i, const ulong j
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

   return
      obj.mend.imin <= i && i < obj.mend.iend &&
      obj.mend.jmin <= j && j < obj.mend.jend;
}



// inbound: are i,j (from insub) in shape's [min,end)?
template<class real, class tag>
inline bool inbound(
   const shape<real,tag> &obj,
   const subinfo &insub
) {
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

template<class BIN, class real, class tag>
inline bool get_first(
   const BIN &bin,
   const unsigned s,
   const ulong i, const ulong j,
   const ulong zone,
   const eyetardiff<real> &etd,
   const real qmin, inq<real,tag> &q
) {
   const shape<real,tag> &obj = *bin[s].shape;
   if (!inbound(obj,i,j)) return false;

   #ifdef KIP_FUZZY
      const real eps = epsilon<real>();
      static unsigned num = 0;

      if (qmin == std::numeric_limits<real>::max())
         num = 0;

      if (obj.infirst(etd, qmin, q, subinfo(i,j, unsigned(zone), obj.mend))) {
         q = real(q)*(1 + eps*random_full<real>());
         return q < qmin*(1-eps)
            ? (num = 1)
            : ++num == 1 || num*random_unit<real>() < 1;
      }

      return false;
   #else
      return obj.infirst(etd, qmin, q, subinfo(i,j, unsigned(zone), obj.mend));
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
   const eyetardiff<real> &etd,

   // qmin, q
   const real qmin,
   inq<real,tag> &q,

   // insub
   const subinfo &insub
) {
   const shape<real,tag> &obj = *s;
   if (!inbound(obj,insub)) return false;

   #ifdef KIP_FUZZY
      const real eps = epsilon<real>();
      if (!(s->infirst(kip_etd, qmin,q, insub)))
         return false;
      return real(q = real(q)*(1 + eps*random_full<real>())) < qmin;
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
   const eyetardiff<real> &etd,

   // qmin, q
   const real qmin,
   afew<inq<real,tag>> &q,

   // insub
   const subinfo &insub
) {
   const shape<real,tag> &obj = *s;
   if (!inbound(obj,insub)) return false;

   #ifdef KIP_FUZZY
      const real eps = epsilon<real>();
      if (!(obj.inall(kip_etd, qmin,q, insub)))
         return false;

      const unsigned size = q.size();
      for (unsigned n = size;  n--; )
         q[n] = real(q[n])*(1 + eps*random_full<real>());

      q.sort();
      for (unsigned n = size;  n--; )
         if (!(q[n] < qmin)) q.setsize(q.size()-1); else break;

      return q.size();
   #else
      return obj.inall(kip_etd, qmin,q, insub);
   #endif
}



// -----------------------------------------------------------------------------
// Plain
// Each of these is called with a particular pixel (i,j), and a particular bin;
// and each loops over the bin's objects.
// -----------------------------------------------------------------------------

// one_plain
template<class real, class base, class color, class pix>
class one_plain {
public:
   void operator()(
      const engine<real> &engine, const image<real,color> &image,
      std::vector<minimum_and_shape<real,base>> &bin,
      ulong &endsorted, const ulong binsize, const real maximum,
      inq<real,base> &qa, inq<real,base> *qa_ptr,
      inq<real,base> &qb, inq<real,base> *qb_ptr,
      color *const ptr,
      unsigned &prev,
      const vars<real,base> &vars, const point<real> &light,
      const point<real> &target, const point<real> &diff,
      pix &pixel,
      const ulong i, const ulong j, const ulong zone
   ) const {

      (void)engine;
      (void)image;
      (void)endsorted;
      (void)binsize;
      (void)qa_ptr;
      (void)qb;
      (void)qb_ptr;
      (void)prev;

      const eyetardiff<real> etd(vars.eyeball, target, diff);

      get_first(bin, 0, i, j, zone, etd, maximum, qa)
         ? (*ptr = get_color<color>(vars.eyeball, light, qa, pixel))
         :  *ptr;
   }
};



// two_plain
template<class real, class base, class color, class pix>
class two_plain {
public:
   void operator()(
      const engine<real> &engine, const image<real,color> &image,
      std::vector<minimum_and_shape<real,base>> &bin,
      ulong &endsorted, const ulong binsize, const real maximum,
      inq<real,base> &qa, inq<real,base> *qa_ptr,
      inq<real,base> &qb, inq<real,base> *qb_ptr,
      color *const ptr,
      unsigned &prev,
      const vars<real,base> &vars, const point<real> &light,
      const point<real> &target, const point<real> &diff,
      pix &pixel,
      const ulong i, const ulong j, const ulong zone
   ) const {

      (void)engine;
      (void)image;
      (void)endsorted;
      (void)binsize;
      (void)qa_ptr;
      (void)qb_ptr;
      (void)prev;

      const eyetardiff<real> etd(vars.eyeball, target, diff);

      get_first(bin, 0, i, j, zone, etd, maximum, qa)
         ? bin[1].minimum < qa.q &&
      get_first(bin, 1, i, j, zone, etd, qa.q, qb)
         ? (*ptr = get_color<color>(vars.eyeball, light, qb, pixel))
         : (*ptr = get_color<color>(vars.eyeball, light, qa, pixel))
         : get_first(bin, 1, i, j, zone, etd, maximum, qb)
         ? (*ptr = get_color<color>(vars.eyeball, light, qb, pixel))
         :  *ptr;
   }
};



// max_plain
template<class real, class base, class color, class pix>
class max_plain {
public:
   void operator()(
      const engine<real> &engine, const image<real,color> &image,
      std::vector<minimum_and_shape<real,base>> &bin,
      ulong &endsorted, const ulong binsize, const real maximum,
      inq<real,base> &qa, inq<real,base> *qa_ptr,
      inq<real,base> &qb, inq<real,base> *qb_ptr,
      color *const ptr,
      unsigned &prev,
      const vars<real,base> &vars, const point<real> &light,
      const point<real> &target, const point<real> &diff,
      pix &pixel,
      const ulong i, const ulong j, const ulong zone
   ) const {

      (void)engine;
      (void)image;
      (void)endsorted;
      (void)qb;
      (void)prev;

      const eyetardiff<real> etd(vars.eyeball, target, diff);

      for (unsigned s = 0;  s < binsize;  ++s)
         if (get_first(bin, s, i, j, zone, etd, maximum, qa)) {
            for ( ; ++s < binsize && bin[s].minimum < *qa_ptr ; )
               if (get_first(bin, s, i, j, zone, etd, real(*qa_ptr), *qb_ptr))
                  std::swap(qa_ptr,qb_ptr);
           *ptr = get_color<color>(vars.eyeball, light, *qa_ptr, pixel);
            return;
         }
   }
};



// any_plain
template<class real, class base, class color, class pix>
class any_plain {
public:
   void operator()(
      const engine<real> &engine, const image<real,color> &image,
      std::vector<minimum_and_shape<real,base>> &bin,
      ulong &endsorted, const ulong binsize, const real maximum,
      inq<real,base> &qa, inq<real,base> *qa_ptr,
      inq<real,base> &qb, inq<real,base> *qb_ptr,
      color *const ptr,
      unsigned &prev,
      const vars<real,base> &vars, const point<real> &light,
      const point<real> &target, const point<real> &diff,
      pix &pixel,
      const ulong i, const ulong j, const ulong zone
   ) const {

      (void)image;
      (void)qb;

      const eyetardiff<real> etd(vars.eyeball, target, diff);

      unsigned s = 0;  bool found;  qa = maximum;
      if (prev > 3) {  // I tried several values, and this number works well

         // examine bin[prev]
         found = get_first(bin, prev, i, j, zone, etd, real(*qa_ptr), *qb_ptr);
         unsigned newprev = found ? std::swap(qa_ptr,qb_ptr), prev : 0;

         // examine bin[0] through bin[prev-1]; fall-through iff prev == 0
         for ( ;  s < prev;  ++s)
            if (get_first(bin, s, i, j, zone, etd, real(*qa_ptr), *qb_ptr))
               std::swap(qa_ptr,qb_ptr), newprev = s, found = true;

         // examine bin[prev+1] forward
         s = prev+1;
         for (;;) {
            for ( ;  s < endsorted && (!found || bin[s].minimum < *qa_ptr);  ++s)
               if (get_first(bin, s, i, j, zone, etd, real(*qa_ptr), *qb_ptr))
                  std::swap(qa_ptr,qb_ptr), newprev = s, found = true;

            // break if found something, or if there's no more to sort
            if (s != endsorted || endsorted == binsize) break;

            // sort more
            endsorted = get_endsorted(endsorted, engine, binsize);
            std::partial_sort(
               bin.begin()+s,
               bin.begin()+endsorted,
               bin.end(),
               less<real,base>()
            );
         }
         prev = newprev;

      } else {

         // for the present pixel (i,j), loop over objects in the present bin
         found = false;
         for ( ;  s < endsorted;  ++s)
            if (get_first(bin, s, i, j, zone, etd, real(*qa_ptr), *qb_ptr))
               { std::swap(qa_ptr,qb_ptr);  prev = s++;  found = true;  break; }

         for (;;) {
            for ( ;  s < endsorted && bin[s].minimum < *qa_ptr;  ++s)
               if (get_first(bin, s, i, j, zone, etd, real(*qa_ptr), *qb_ptr))
                  std::swap(qa_ptr,qb_ptr), prev = s, found = true;

            // break if found something, or if there's no more to sort
            if (s != endsorted || endsorted == binsize) break;

            // sort more
            endsorted = get_endsorted(endsorted, engine, binsize);
            std::partial_sort(
               bin.begin()+s,
               bin.begin()+endsorted,
               bin.end(),
               less<real,base>()
            );
         }
      }

      // color
      found
       ? (*ptr = get_color<color>(vars.eyeball, light, *qa_ptr, pixel))
       :  *ptr;
   }
};



// -----------------------------------------------------------------------------
// Anti
// Same remark as for the "Plain" functions above; and in addition, we shoot
// several rays *within* the pixel.
// -----------------------------------------------------------------------------

// one_anti
template<class real, class base, class color, class pix>
class one_anti {
public:
   bool operator()(
      const engine<real> &engine, const image<real,color> &image,
      std::vector<minimum_and_shape<real,base>> &bin,
      ulong &endsorted, const ulong binsize, const real maximum,
      inq<real,base> &qa, inq<real,base> *qa_ptr,
      inq<real,base> &qb, inq<real,base> *qb_ptr,
      const real h, const real v, RGBA<unsigned> &sum,
      const vars<real,base> &vars, const light<real> &light,
      pix &pixel,
      const ulong i, const ulong j, const ulong zone
   ) const {

      (void)engine;
      (void)endsorted;
      (void)binsize;
      (void)qa_ptr;
      (void)qb;
      (void)qb_ptr;

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
         const eyetardiff<real> etd(vars.eyeball, target, diff);

         // examine object
         sum += get_first(bin, 0, i, j, zone, etd, maximum, qa)
             ? (found = true, get_color<color>(vars.eyeball, light[0], qa, pixel))
             :  image.background;
      }

      return found;
   }
};



// two_anti
template<class real, class base, class color, class pix>
class two_anti {
public:
   bool operator()(
      const engine<real> &engine, const image<real,color> &image,
      std::vector<minimum_and_shape<real,base>> &bin,
      ulong &endsorted, const ulong binsize, const real maximum,
      inq<real,base> &qa, inq<real,base> *qa_ptr,
      inq<real,base> &qb, inq<real,base> *qb_ptr,
      const real h, const real v, RGBA<unsigned> &sum,
      const vars<real,base> &vars, const light<real> &light,
      pix &pixel,
      const ulong i, const ulong j, const ulong zone
   ) const {

      (void)engine;
      (void)endsorted;
      (void)binsize;
      (void)qa_ptr;
      (void)qb_ptr;

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
         const eyetardiff<real> etd(vars.eyeball, target, diff);

         // examine objects
         sum += get_first(bin, 0, i, j, zone, etd, maximum, qa)
              ? get_first(bin, 1, i, j, zone, etd, qa.q, qb)
                   // [1] is better
                 ? (found=true, get_color<color>(vars.eyeball,light[0],qb,pixel))
                   // [0] is better (or only)...
                 : (found=true, get_color<color>(vars.eyeball,light[0],qa,pixel))
              : get_first(bin, 1, i, j, zone, etd, maximum, qa)
                   // [1] is only...
                 ? (found=true, get_color<color>(vars.eyeball,light[0],qa,pixel))
                   // neither...
                 :  image.background;
      }

      return found;
   }
};



// any_anti
template<class real, class base, class color, class pix>
class any_anti {
public:
   bool operator()(
      const engine<real> &engine, const image<real,color> &image,
      std::vector<minimum_and_shape<real,base>> &bin,
      ulong &endsorted, const ulong binsize, const real maximum,
      inq<real,base> &qa, inq<real,base> *qa_ptr,
      inq<real,base> &qb, inq<real,base> *qb_ptr,
      const real h, const real v, RGBA<unsigned> &sum,
      const vars<real,base> &vars, const light<real> &light,
      pix &pixel,
      const ulong i, const ulong j, const ulong zone
   ) const {

      (void)qa;
      (void)qb;

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
         const eyetardiff<real> etd(vars.eyeball, target, diff);

         // loop over objects in this bin
         unsigned s = 0;  bool f = false;
         for ( ;  s < endsorted;  ++s)
            if (get_first(bin, s, i, j, zone, etd, maximum, *qb_ptr))
               { std::swap(qa_ptr,qb_ptr);  f = true;  s++;  break; }

         for (;;) {
            for ( ;  s < endsorted && bin[s].minimum < *qa_ptr;  ++s)
               if (get_first(bin, s, i, j, zone, etd, real(*qa_ptr), *qb_ptr))
                  std::swap(qa_ptr,qb_ptr), f = true;

            // break if found something, or if there's no more to sort
            if (s != endsorted || endsorted == binsize) break;

            // sort more
            endsorted = get_endsorted(endsorted, engine, binsize);
            std::partial_sort(
               bin.begin()+s,
               bin.begin()+endsorted,
               bin.end(),
               less<real,base>()
            );
         }

         if (f) {
            found = true;
            sum += get_color<color>(vars.eyeball,light[0], *qa_ptr, pixel);
         } else {
            sum += image.background;
         }
      }

      return found;
   }
};



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
   class real, class base, class color, class pix,
   class ACTION
>
inline void fill_loop_plain(
   const engine<real> &engine, image<real,color> &image,
   const vars<real,base> &vars, const light<real> &light,

   const ulong imin, const ulong iend,
   const ulong jmin, const ulong jend,
   const ulong zone, const real maximum,

   std::vector<minimum_and_shape<real,base>> &bin,
   ulong endsorted, const ulong binsize,
   inq<real,base> &qa, inq<real,base> &qb,

   array<2,pix> &pixel,
   const ACTION &action
) {
   // vertical pixels in the current bin...
   for (ulong j = jmin;  j < jend;  ++j) {
      const point<real> *tar = &image.prior.targets(imin,j);
      color *ptr = &image(imin,j);  unsigned prev = 0;
      pix *p = &pixel(imin,j);

      // horizontal pixels in the current bin...
      for (ulong i = imin;  i < iend;  ++i, ++tar, ++ptr, ++p) {
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
   class real, class base, class color, class pix,
   class ACTION
>
inline void fill_loop_lean(
   const view  <real      > &view,
   const engine<real      > &engine,
         image <real,color> &image,
   const vars  <real,base > &vars,
   const light <real      > &light,

   const ulong imin, const ulong iend,
   const ulong jmin, const ulong jend,
   const ulong zone, const real maximum,

   std::vector<minimum_and_shape<real,base>> &bin,
   ulong endsorted, const ulong binsize,
   inq<real,base> &qa, inq<real,base> &qb,

   array<2,pix> &pixel,
   const ACTION &action
) {
   // qqq Should compute these at a higher level
   const real hmin = vars.hhalf - vars.hmax;
   const real vmin = vars.vhalf - vars.vmax, dsq = view.d*view.d;

   // vertical pixels in the current bin...
   for (ulong j = jmin;  j < jend;  ++j) {
      const real v = vmin + real(j   )*vars.vfull, tmp = dsq + v*v;
      /* */ real h = hmin + real(imin)*vars.hfull;
      color *ptr = &image(imin,j);  unsigned prev = 0;
      pix   *p   = &pixel(imin,j);

      // horizontal pixels in the current bin...
      for (ulong i = imin;  i < iend;  ++i, h += vars.hfull, ++ptr, ++p) {
         // a=(d,0,0), b=(0,h,v), (x,y,z)=a+(b-a)/mod(b-a)
         const real norm = real(1)/std::sqrt(tmp + h*h);
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
  class real, class base, class color, class pix, class ulong, class ACTION
>
inline void fill_loop_anti(
   const engine<real      > &engine,
         image <real,color> &image,
   const vars  <real,base > &vars,
   const light <real      > &light,

   const real hcent, const ulong imin, const ulong iend,
         real v,     const ulong jmin, const ulong jend,

   const ulong zone,
   const real maximum,

   std::vector<minimum_and_shape<real,base>> &bin,
   ulong endsorted, const ulong binsize,
   inq<real,base> &qa, inq<real,base> &qb,

   array<2,pix> &pixel,
   const ACTION &action
) {
   real h = hcent;

   // vertical pixels in the current bin...
   for (ulong j = jmin;  j < jend;  ++j, v += vars.vfull, h = hcent) {
      color *ptr = &image(imin,j);
      pix   *p   = &pixel(imin,j);

      // horizontal pixels in the current bin...
      for (ulong i = imin;  i < iend;  ++i, h += vars.hfull, ++ptr, ++p) {
         RGBA<unsigned> sum(0,0,0);  // qqq don't hardcode RGBA, here/elsewhere

         // one_anti()
         // two_anti()
         // any_anti()
         if (action(
            engine, image, bin, endsorted, binsize,
            maximum, qa, &qa, qb, &qb, h, v, sum, vars, light, *p,
            i, j, zone
         ))
            *ptr = op::div<uchar>(sum,vars.anti2);
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
   const ulong imin, const ulong iend,
   const ulong jmin, const ulong jend, const color &border
) {
   for (ulong j=jmin; j<jend; ++j) image(imin,j) = image(iend-1,j) = border;
   for (ulong i=imin; i<iend; ++i) image(i,jmin) = image(i,jend-1) = border;
}



// trace_bin
template<class real, class base, class color, class pix>
void trace_bin(
   const engine<real      > &engine,
   const view  <real      > &view,
         image <real,color> &image,
   const vars  <real,base > &vars,
   const light <real      > &light,
   array<2,pix> &pixel,

   ulong imin, ulong iend,
   ulong jmin, ulong jend,
   const ulong zone, const ulong max_binsize,

   std::vector<minimum_and_shape<real,base>> &bin,
   const ulong binsize
) {
   // border?
   if (image.border.bin)
      bin_border(image, imin++,iend--, jmin++,jend--,
                 color::border(binsize, max_binsize));

   // binsize-dependent [partial-]sorting actions
   using diff_t =
      typename std::vector<minimum_and_shape<real,base>>::difference_type;
   ulong endsorted = 0;

   if (binsize == 2) {
      if (bin[1].minimum < bin[0].minimum)
         std::swap(bin[0], bin[1]);
   } else if (binsize > 2) {
      endsorted = get_endsorted(0, engine, binsize);
      std::partial_sort(
         bin.begin(),
         bin.begin()+diff_t(endsorted),
         bin.end(),
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
         if (binsize == 1) {
            fill_loop_lean(
               view, engine, image, vars, light, imin, iend, jmin, jend,
               zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
               one_plain<real,base,color,pix>() // one
            );
         } else if (binsize == 2) {
            fill_loop_lean(
               view, engine, image, vars, light, imin, iend, jmin, jend,
               zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
               two_plain<real,base,color,pix>() // two
            );
         } else if (binsize == endsorted) {
            fill_loop_lean(
               view, engine, image, vars, light, imin, iend, jmin, jend,
               zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
               max_plain<real,base,color,pix>() // max
            );
         } else {
            fill_loop_lean(
               view, engine, image, vars, light, imin, iend, jmin, jend,
               zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
               any_plain<real,base,color,pix>() // any
            );
         }
      } else {
         if (binsize == 1) {
            fill_loop_plain(
               engine, image, vars, light, imin, iend, jmin, jend,
               zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
               one_plain<real,base,color,pix>() // one
            );
         } else if (binsize == 2) {
            fill_loop_plain(
               engine, image, vars, light, imin, iend, jmin, jend,
               zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
               two_plain<real,base,color,pix>() // two
            );
         } else if (binsize == endsorted) {
            fill_loop_plain(
               engine, image, vars, light, imin, iend, jmin, jend,
               zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
               max_plain<real,base,color,pix>() // max
            );
         } else {
            fill_loop_plain(
               engine, image, vars, light, imin, iend, jmin, jend,
               zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
               any_plain<real,base,color,pix>() // any
            );
         }
      }

   } else {
      // Antialiasing case

      // center of pixel (imin,jmin)
      const real hcent = real(imin)*vars.hfull - vars.hmax + vars.hhalf;
      const real vcent = real(jmin)*vars.vfull - vars.vmax + vars.vhalf;

      // action
      if (binsize == 1) {
         fill_loop_anti(
            engine, image, vars, light, hcent, imin, iend, vcent, jmin, jend,
            zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
            one_anti<real,base,color,pix>() // one
         );
      } else if (binsize == 2) {
         fill_loop_anti(
            engine, image, vars, light, hcent, imin, iend, vcent, jmin, jend,
            zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
            two_anti<real,base,color,pix>() // two
         );
      } else {
         fill_loop_anti(
            engine, image, vars, light, hcent, imin, iend, vcent, jmin, jend,
            zone, maximum, bin, endsorted, binsize, qa, qb, pixel,
            any_anti<real,base,color,pix>() // any
         );
      }
   }
}

} // namespace detail
