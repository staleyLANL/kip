
// -----------------------------------------------------------------------------
// propagate_*
// -----------------------------------------------------------------------------

namespace detail {

// propagate_unary
template<class SHAPE>
void propagate_unary(const SHAPE &obj, const bool force)
{
   bool baseset;
   if (!(baseset = obj.unary.a->baseset) || force) {
      obj.unary.a->base() = obj.base(); // side effect: marks as set
      obj.unary.a->baseset = baseset;   // re-flag as necessary
   }
   obj.unary.a->propagate_base(force);
}


// propagate_binary
template<class real, class tag>
void propagate_binary(const shape<real,tag> &obj, const bool force)
{
   bool baseset;

   if (!(baseset = obj.binary.a->baseset) || force) {
      obj.binary.a->base() = obj.base();
      obj.binary.a->baseset = baseset;
   }
   obj.binary.a->propagate_base(force);

   if (!(baseset = obj.binary.b->baseset) || force) {
      obj.binary.b->base() = obj.base();
      obj.binary.b->baseset = baseset;
   }
   obj.binary.b->propagate_base(force);
}


// propagate_nary
template<class VEC, class tag>
void propagate_nary(const VEC &vec, const tag &base, const bool force)
{
   const ulong size = vec.size();
   for (ulong n = 0;  n < size;  ++n) {
      bool baseset;
      if (!(baseset = vec[n].op->baseset) || force) {
         vec[n].op->base() = base;
         vec[n].op->baseset = baseset;
      }
      vec[n].op->propagate_base(force);
   }
}

} // namespace detail



// -----------------------------------------------------------------------------
// Miscellaneous
// -----------------------------------------------------------------------------

// forward: read_shapes
template<class real, class tag, class NARY>
bool read_shapes(kip::istream &, NARY &);



// afew_book: helper class
// An "afew", plus bookkeeping for it; for brevity with nary operator code
template<class real, class base>
class afew_book {
public:
   afew<real,base> points;  // for this operand
   ulong size;  // size (computed before loop) of "points"
   ulong next;  // running next-position
   bool in_now;  // status as we move along the ray

   /* */ inq<real,base> &q()       { return points[next]; }
   const inq<real,base> &q() const { return points[next]; }
};



// grower
template<class T>
class grower {
   std::vector<T> vec;
   ulong position;

public:
   // grower()
   grower() : position(0) { }

   // more(count)
   T *more(const ulong count)
   {
      const ulong available = vec.size() - position;
      if (available < count)
         vec.insert(vec.end(), count-available, T());
         /*
         for (ulong i = 0;  i < count-available;  ++i)
            vec.push_back(T());
         */

      T *const operand = &vec[position];
      position += count;
      return operand;
   }

   // less(count)
   void less(const ulong count)
   {
      position -= count;
   }
};
