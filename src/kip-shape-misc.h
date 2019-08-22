
#pragma once

// This file provides miscellaneous functionality related to shapes.

// -----------------------------------------------------------------------------
// allshape
// rshhhd
// -----------------------------------------------------------------------------

namespace internal {

// less - forward declaration
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

} // namespace internal



// -----------------------------------------------------------------------------
// xypoint
// xrpoint
// -----------------------------------------------------------------------------

// xypoint
template<class real = default_real>
class xypoint {
public:
   real x, y;
   inline explicit xypoint() { }
   inline explicit xypoint(const real _x, const real _y) : x(_x), y(_y) { }
};

// xrpoint
template<class real = default_real>
class xrpoint {
public:
   real x, r;
   inline explicit xrpoint() { }
   inline explicit xrpoint(const real _x, const real _r) : x(_x), r(_r) { }
};



// -----------------------------------------------------------------------------
// array_simple
// -----------------------------------------------------------------------------

template<class value_t>
class array_simple {
   value_t *ptr;
   ulong size;

public:

   // array_simple()
   inline explicit array_simple() : ptr(nullptr), size(0) { }

   // destructor
   inline ~array_simple()
   {
      delete[] ptr;
   }

   // resize
   inline void resize(const ulong newsize)
   {
      // does *not* copy old values
      if (size != newsize) {
         delete[] ptr;
         ptr = new value_t[size = newsize];
      }
   }

   // operator[]
   inline       value_t &operator[](const ulong n)       { return ptr[n]; }
   inline const value_t &operator[](const ulong n) const { return ptr[n]; }
};



// -----------------------------------------------------------------------------
// inq
//    i for intersection
//    n for normal
//    q for parameter
// -----------------------------------------------------------------------------

// normalized_t
enum class normalized_t { yesnorm, nonorm };



// inq
template<class real, class base>
class inq : public point<real> {  // the point<real> is the intersection
public:
   using value_t = real;
   using base_t  = base;

   // data
   point<real> n;
   real q, fac;

   const kip::shape<real,base> *shape;
   const base *color;
   normalized_t normalized;

   // inq()
   // inq(real)
   inline explicit inq() { }
   inline explicit inq(const real _q) : q(_q) { }

   // assignment from real
   inline inq &operator=(const real from)
   {
      q = from;
      return *this;
   }

   // conversion to real
   inline operator       real&()       { return q; }
   inline operator const real&() const { return q; }

   // operator()(nx,ny,nz, shape,        normalized [, fac = default])
   // operator()(n,        shape,        normalized [, fac = default])
   // operator()(nx,ny,nz, shape, color, normalized [, fac = default])
   // operator()(n,        shape, color, normalized [, fac = default])

   inline inq &operator()(
      const real nx, const real ny, const real nz,
      const kip::shape<real,base> *const _shape,
      const normalized_t _normalized,
      const real _fac = real(-1)
   ) {
      n.x = nx;  shape = _shape;
      n.y = ny;  color = _shape;
      n.z = nz;  normalized = _normalized;
      fac = _fac;
      return *this;
   }

   inline inq &operator()(
      const point<real> &_n,
      const kip::shape<real,base> *const _shape,
      const normalized_t _normalized,
      const real _fac = real(-1)
   ) {
      shape = _shape;  n = _n;
      color = _shape;  normalized = _normalized;
      fac = _fac;
      return *this;
   }

   inline inq &operator()(
      const real nx, const real ny, const real nz,
      const kip::shape<real,base> *const _shape,
      const base *const _color,
      const normalized_t _normalized,
      const real _fac = real(-1)
   ) {
      n.x = nx;  shape = _shape;
      n.y = ny;  color = _color;
      n.z = nz;  normalized = _normalized;
      fac = _fac;
      return *this;
   }

   inline inq &operator()(
      const point<real> &_n,
      const kip::shape<real,base> *const _shape,
      const base *const _color,
      const normalized_t _normalized,
      const real _fac = real(-1)
   ) {
      shape = _shape;  n = _n;
      color = _color;  normalized = _normalized;
      fac = _fac;
      return *this;
   }

   // reverse
   inline inq &reverse(const bool doit)  // conditional
   {
      if (doit) {
         n.x = -n.x;
         n.y = -n.y;
         n.z = -n.z;
      }
      return *this;
   }
   inline inq &reverse() { return reverse(true); }  // unconditional
};



// -----------------------------------------------------------------------------
// afew
// -----------------------------------------------------------------------------

template<class INQ>
class afew {

   // Note: length...
   //    must be >= 2 for convex() to work
   //    must be >= 4 for four() to work
   static const ulong length = 12;
   INQ buffer[length], *ptr;
   ulong bufsize, num;

public:

   // constructor
   inline explicit afew() : ptr(buffer), bufsize(length), num(0) { }

   // destructor
   inline ~afew()
   {
      if (ptr != buffer)
         delete[] ptr;
   }

   // copy constructor
   inline afew(const afew &from) :
      num(from.num)
   {
      if (from.ptr == from.buffer) {
         bufsize = length;
         ptr = buffer;
      } else
         ptr = new INQ[bufsize = from.bufsize];

      for (ulong i = 0;  i < num;  ++i)
         ptr[i] = from.ptr[i];
   }

   // copy assignment
   inline afew &operator=(const afew &from)
   {
      return assign(from);
   }

   // assign
   // zzz depending on usage, we might be able to get away with a shallow
   // copy; for now, do a deep copy
   inline afew &assign(const afew &from)
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
            ptr = new INQ[bufsize];
         } else {
            if (bufsize == from.bufsize) {
               // nothing
            } else {
               delete[] ptr;
               bufsize = from.bufsize;
               ptr = new INQ[bufsize];
            }
         }
      }

      num = from.num;
      for (ulong i = 0;  i < num;  ++i)
         ptr[i] = from.ptr[i];
      return *this;
   }

   // size()
   inline ulong size() const { return num; }

   // setsize(n) - use only if you know what you're doing
   inline void setsize(const ulong n) { num = n; }

   // operator[]
   inline       INQ &operator[](const ulong i)
      { kip_assert_index(i < num);  return ptr[i]; }
   inline const INQ &operator[](const ulong i) const
      { kip_assert_index(i < num);  return ptr[i]; }

   // push
   inline INQ &push(const INQ &value)
   {
      kip_assert_index(num <= bufsize);
      if (num == bufsize) {
         bufsize += bufsize;
         INQ *const newptr = new INQ[bufsize];
         for (ulong i = 0;  i < num;  ++i)
            newptr[i] = ptr[i];
         if (ptr != buffer)
            delete[] ptr;
         ptr = newptr;
      }
      return ptr[num++] = value;
   }

   // one()
   inline INQ &one()
   {
      num = 1;
      return ptr[0];
   }

   // reset()
   inline void reset()
   {
      num = 0;
   }

   // convex()
   inline void convex()
   {
      num = 0;
   }

   inline bool convex(const INQ &inq)  // for INQ of inq<> type
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
   inline void four()
   {
      num = 0;
   }

   inline bool four(const INQ &inq)  // for INQ of inq<> type
   {
      if (num < 4)
         ptr[num++] = inq;
      return num == 4;  // done (true) if 4
   }

   inline bool sort()
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
            std::sort(
               ptr, ptr+num,
               internal::less<typename INQ::value_t, typename INQ::base_t>()
            );
      }

      return num > 0;
   }

   // reverse (reverses normals, not container of inqs)
   inline afew &reverse(const bool doit)
   {
      if (doit)
         for (ulong i = 0;  i < num;  ++i)
            ptr[i].reverse();
      return *this;
   }

   inline afew &reverse()
   {
      return reverse(true);
   }

   // conversion to bool (always returns true)
   inline operator bool() const { return true; }
};
