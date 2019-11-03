
// -----------------------------------------------------------------------------
// allshape
// rshhhd
// -----------------------------------------------------------------------------

namespace detail {

// forward: less
template<class real, class tag> class less;


// allshape
template<class AGGREGATE, class FUNCTOR>
void allshape(AGGREGATE &obj, FUNCTOR &f)
{
#define kip_call_f(type) f(obj.type)
   kip_expand(kip_call_f,;)
#undef  kip_call_f
}


// rshhhd
// Contains precomputed values for tabular objects
template<class real>
class rshhhd {
public:
   // for each point
   real rsq;

   // for each segment
   real slope;
   real h1, h2, h3;
   real dmins;
};

} // namespace detail



// -----------------------------------------------------------------------------
// xypoint
// xrpoint
// -----------------------------------------------------------------------------

// xypoint
template<class real>
class xypoint {
public:
   real x, y;
   explicit xypoint() { }
   explicit xypoint(const real _x, const real _y) : x(_x), y(_y) { }
};

// xrpoint
template<class real>
class xrpoint {
public:
   real x, r;
   explicit xrpoint() { }
   explicit xrpoint(const real _x, const real _r) : x(_x), r(_r) { }
};



// -----------------------------------------------------------------------------
// inq
//    i for intersection
//    n for normal
//    q for parameter
// -----------------------------------------------------------------------------

// normalized
enum class normalized { yes, no };

// inq
template<class real, class base>
class inq {
public:

   // data
   point<real> inter;
   point<real> n;
   real q;
   real fac;
   const kip::shape<real,base> *shape;
   const base *color;
   normalized isnormalized;

   // inq()
   // inq(real)
   explicit inq() { }
   explicit inq(const real _q) : q(_q) { }

   // assignment from real
   inq &operator=(const real from)
   {
      q = from;
      return *this;
   }

   // conversion to real
   operator       real&()       { return q; }
   operator const real&() const { return q; }

   // set
   inq &set(
      const point<real> &_n,
      const kip::shape<real,base> *const _shape,
      const normalized _isnormalized,
      const real _fac = -1
   ) {
      n = _n;
      shape = _shape;
      color = &_shape->base();
      isnormalized = _isnormalized;
      fac = _fac;
      return *this;
   }

   inq &set(
      const real nx, const real ny, const real nz,
      const kip::shape<real,base> *const _shape,
      const normalized _isnormalized,
      const real _fac = -1
   ) {
      return set(point<real>(nx,ny,nz), _shape, _isnormalized, _fac);
   }

   inq &set(
      const real nx, const real ny, const real nz,
      const kip::shape<real,base> *const _shape,
      const base *const _color, // for xplane etc. (w/ two colors)
      const normalized _isnormalized,
      const real _fac = -1
   ) {
      set(nx,ny,nz, _shape, _isnormalized, _fac);
      color = _color;
      return *this;
   }

   // reverse
   inq &reverse(const bool doit)  // conditional
   {
      if (doit) {
         n.x = -n.x;
         n.y = -n.y;
         n.z = -n.z;
      }
      return *this;
   }
   inq &reverse() { return reverse(true); }  // unconditional
};



// -----------------------------------------------------------------------------
// afew
// -----------------------------------------------------------------------------

template<class real, class base>
class afew {

   // Note: length...
   //    must be >= 2 for convex() to work
   //    must be >= 4 for four() to work
   static const ulong length = 12;
   inq<real,base> buffer[length], *ptr;
   ulong bufsize, num;

public:

   // constructor
   explicit afew() : ptr(buffer), bufsize(length), num(0) { }

   // destructor
   ~afew()
   {
      if (ptr != buffer)
         delete[] ptr;
   }

   // copy constructor
   afew(const afew &from) :
      num(from.num)
   {
      if (from.ptr == from.buffer) {
         bufsize = length;
         ptr = buffer;
      } else
         ptr = new inq<real,base>[bufsize = from.bufsize];

      for (ulong i = 0;  i < num;  ++i)
         ptr[i] = from.ptr[i];
   }

   // copy assignment
   afew &operator=(const afew &from)
   {
      return assign(from);
   }

   // assign
   // zzz depending on usage, we might be able to get away with a shallow
   // copy; for now, do a deep copy
   afew &assign(const afew &from)
   {
      if (from.ptr == from.buffer) {
         // "from" uses buffer memory; do the same here
         if (ptr != buffer)
            delete[] ptr;
         bufsize = length;
         ptr = buffer;
      } else {
         // "from" uses allocated memory; do the same here
         if (ptr == buffer) {
            bufsize = from.bufsize;
            ptr = new inq<real,base>[bufsize];
         } else {
            if (bufsize == from.bufsize) {
               // nothing
            } else {
               delete[] ptr;
               bufsize = from.bufsize;
               ptr = new inq<real,base>[bufsize];
            }
         }
      }

      num = from.num;
      for (ulong i = 0;  i < num;  ++i)
         ptr[i] = from.ptr[i];
      return *this;
   }

   // size()
   ulong size() const { return num; }

   // setsize(n) - use only if you know what you're doing
   void setsize(const ulong n) { num = n; }

   // operator[]
   inq<real,base> &operator[](const ulong i)
      { kip_assert_index(i < num);  return ptr[i]; }
   const inq<real,base> &operator[](const ulong i) const
      { kip_assert_index(i < num);  return ptr[i]; }

   // push
   inq<real,base> &push(const inq<real,base> &value)
   {
      kip_assert_index(num <= bufsize);
      if (num == bufsize) {
         bufsize += bufsize;
         inq<real,base> *const newptr = new inq<real,base>[bufsize];
         for (ulong i = 0;  i < num;  ++i)
            newptr[i] = ptr[i];
         if (ptr != buffer)
            delete[] ptr;
         ptr = newptr;
      }
      return ptr[num++] = value;
   }

   // one()
   inq<real,base> &one()
   {
      num = 1;
      return ptr[0];
   }

   // reset()
   void reset()
   {
      num = 0;
   }

   // convex()
   void convex()
   {
      num = 0;
   }

   bool convex(const kip::inq<real,base> &inq)
   {
      if (num) {
         kip_assert(num == 1);

         if (inq.q > ptr[0].q)
            ptr[1] = inq;
         else
            ptr[1] = ptr[0], ptr[0] = inq;
         num = 2;
         return true;   // now have second entry

      } else {
         ptr[0] = inq;
         num = 1;
         return false;  // only have first entry
      }
   }

   // four()
   void four()
   {
      num = 0;
   }

   bool four(const kip::inq<real,base> &inq)
   {
      if (num < 4)
         ptr[num++] = inq;
      return num == 4;  // done (true) if 4
   }

   bool sort()
   {
      // this is kind of mindless; think up something better
      if (num > 1) {
         if (num == 2) {
            if (ptr[0].q > ptr[1].q) std::swap(ptr[0], ptr[1]);
         } else if (num == 3) {
            if (ptr[0].q > ptr[1].q) std::swap(ptr[0], ptr[1]);
            if (ptr[0].q > ptr[2].q) std::swap(ptr[0], ptr[2]);
            if (ptr[1].q > ptr[2].q) std::swap(ptr[1], ptr[2]);
         } else if (num == 4) {
            if (ptr[0].q > ptr[1].q) std::swap(ptr[0], ptr[1]);
            if (ptr[0].q > ptr[2].q) std::swap(ptr[0], ptr[2]);
            if (ptr[0].q > ptr[3].q) std::swap(ptr[0], ptr[3]);
            if (ptr[1].q > ptr[2].q) std::swap(ptr[1], ptr[2]);
            if (ptr[1].q > ptr[3].q) std::swap(ptr[1], ptr[3]);
            if (ptr[2].q > ptr[3].q) std::swap(ptr[2], ptr[3]);
         } else
            std::sort(ptr, ptr+num, detail::less<real,base>());
      }

      return num > 0;
   }

   // reverse (reverses normals, not container of inq<>s)
   afew &reverse(const bool doit)
   {
      if (doit)
         for (ulong i = 0;  i < num;  ++i)
            ptr[i].reverse();
      return *this;
   }

   afew &reverse()
   {
      return reverse(true);
   }

   // conversion to bool (always returns true)
   operator bool() const { return true; }
};
