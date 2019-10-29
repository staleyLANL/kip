
// -----------------------------------------------------------------------------
// ors
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class ors : public shape<real,tag> {
#define   kip_class ors
#include "kip-macro-nary.h"
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(ors)
{
   // process operands
   vec_t &vec = kip_data.vec();
   kip_data.nop = vec.size();
   std::vector<minimum_and_shape<real,tag>> min_and_op(kip_data.nop);

   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      vec[i].op->is_operand = true;
      min_and_op[i].minimum = minimum_t((min_and_op[i].shape = vec[i].op)->
         process(eyeball,light,engine,vars));
      kip_assert(min_and_op[i].minimum >= 0);
   }

   // The logical-ors operator is mutually reflexive, so we can arbitrarily
   // rearrange its operands. Putting operands with smaller minima first tends
   // to speed up the operator.
   std::sort(min_and_op.begin(), min_and_op.end(), detail::less<real,tag>());

   // Bookkeeping
   bool in_all = true;   // remains true  if no objects
   bool in_ge1 = false;  // remains false if no objects
   nary.total_in = 0;
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      vec[i].min = min_and_op[i].minimum;
      if ((vec[i].in=(vec[i].op=min_and_op[i].shape)->interior))  // =, not ==
         in_ge1 = true, nary.total_in++;
      else
         in_all = false;
   }
   this->interior = kip_data.nop ? nary.total_in > 0 : true;

   // minimum
   real rv = kip_data.nop ? min_and_op[0].minimum : real(0);
   if (in_all)
      // in ALL; use maximum of minima (furthest exit)
      for (ulong i = 1;  i < kip_data.nop;  ++i)
         rv = op::max(rv, real(min_and_op[i].minimum));
   else if (in_ge1) {
      // in >=1; use maximum of "in" minima (furthest exit of "in"s)
      rv = real(0);
      for (ulong i = 0;  i < kip_data.nop;  ++i)
         if (vec[i].in)
            rv = op::max(rv, real(min_and_op[i].minimum));
   } else
      // in NONE; use minimum of minima (closest entry)
      for (ulong i = 1;  i < kip_data.nop;  ++i)
         rv = op::min(rv, real(min_and_op[i].minimum));
   return rv;
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(ors)
{
   bbox<real> rv(false,1,0,false, false,1,0,false, false,1,0,false);

   const vec_t &vec = kip_data.vec();
   kip_data.nop = vec.size();

   for (ulong i = 0;  i < kip_data.nop;  ++i)
      rv = detail::bound_combine(rv, vec[i].op->aabb(), detail::op_leq());
   return rv;
} kip_end



// dry
kip_dry(ors)
{
   const vec_t &vec = kip_data.vec();
   for (ulong i = 0;  i < kip_data.nop;  ++i)
      if (!vec[i].op->dry(seg)) return false;  // makes the whole thing non-dry
   return kip_data.nop > 0;  // false if no operands, true otherwise
} kip_end



// randomize
kip_randomize(ors)
{
   const ulong nop = 4;

   point<real> loc;
   random_full(loc);
   obj.clear();
   obj.nary.vec().reserve(nop);

   for (ulong i = 0;  i < nop;  ++i) {
      sphere<real,tag> *const ptr = new sphere<real,tag>;
      randomize(*ptr);
      obj.nary.push().op = ptr;

      ptr->c = loc + point<real>(
         real(0.2)*random_unit<real>(),
         real(0.2)*random_unit<real>(),
         real(0.2)*random_unit<real>()
      );
   }

   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(ors)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop == 0) return false;  // no operands --> no intersection

   // OUTSIDE: we're outside all operands; the overall status changes (from
   // outside to inside) at the first intersection with any object's boundary
   if (!this->interior) {
      inq<real,tag> qtmp;  q = qmin;
      for (ulong i = 0;  i < kip_data.nop && vec[i].min < q;  ++i)
         if (detail::op_first(vec[i].op, kip_etd, real(q),qtmp, insub))
            q = qtmp;
      return q < qmin;
   }

   // INSIDE: we're inside one or more operands; the overall status changes
   // (from inside to outside) at the point after which we're outside all
   // objects
   const ulong num_operand = kip_data.nop;
   using per_operand = afew_book<real,tag>;
#include "kip-macro-workspace.h"

   // compute operand information
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      const bool found =
         vec[i].min < qmin &&
         detail::op_all(vec[i].op, kip_etd,
                      qmin,operand[i].points, insub);

      if (found || !vec[i].in) {
         operand[i].size   = found ? operand[i].points.size() : 0;
         operand[i].next   = 0;
         operand[i].in_now = vec[i].in;
      } else
         // there exists an operand with no ray intersection, AND we're
         // inside it, so the ray can't intersect with the logical disjunction
         return kip_less, false;
   }

   // search for the first relevant intersection. "current" will be the operand
   // that provides this (exit) point, and it's the relevant one if there's no
   // previous such operand, or if this one is closer
   ulong num_in  = nary.total_in;

   for (int current = -1 ;; ++operand[current].next) {
      for (ulong i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size) {
            // there are still more intersections with i
            if (current == -1 || operand[i].q() < operand[current].q())
               current = int(i);
         } else if (operand[i].in_now)
            // there are no more intersections with i, and
            // we're inside of it
            return kip_less, false;

      // if here: never (over all operands) at the end without being outside
      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         ++num_in;  continue;
      } else
         // leaving current operand
         if (--num_in > 0) continue;

      return q = operand[current].q(), kip_less, true;
   }
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(ors)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop == 0) return false;  // no operands --> no intersection

   const ulong num_operand = kip_data.nop;
   using per_operand = afew_book<real,tag>;
#include "kip-macro-workspace.h"

   // compute operand information
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      const bool found =
         vec[i].min < qmin &&
         detail::op_all(vec[i].op, kip_etd,
                      qmin,operand[i].points, insub);

      if (found || !vec[i].in) {
         operand[i].size   = found ? operand[i].points.size() : 0;
         operand[i].next   = 0;
         operand[i].in_now = vec[i].in;
      } else
         return kip_less, false;
   }

   ulong num_in = nary.total_in;
   ints.reset();

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (ulong i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size) {
            // there are still more intersections with i
            if (current == -1 || operand[i].q() < operand[current].q())
               current = int(i);
         } else if (operand[i].in_now)
            // there are no more intersections with i, and we're inside of it
            return kip_less, ints.size() > 0;
      if (current == -1) return kip_less, ints.size() > 0;

      // if here: never (over all operands) at the end without being outside
      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (num_in++ > 0) continue;
      } else
         // leaving current operand
         if (num_in-- > 1) continue;

      ints.push(operand[current].q());
   }

   return kip_less, false;  // unreachable; presence silences g++ warning
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#include "kip-macro-io-nary.h"
