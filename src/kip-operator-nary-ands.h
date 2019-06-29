
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// ands
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class ands : public shape<real,tag> {
   using shape<real,tag>::interior;

#define   kip_class ands
#define   kip_type  misc_type::ands_type
#define   kip_data  misc.ands
#include "kip-macro-nary.h"
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(ands)
{
   // process operands
   vec_t &vec = kip_data.vec();
   kip_data.nop = vec.size();
   std::vector< minimum_and_shape<real,tag> > min_and_op(kip_data.nop);

   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      vec[i].op->is_operand = true;
      min_and_op[i].minimum = minimum_t((min_and_op[i].shape = vec[i].op)->
         process(eyeball,light,engine,vars));
      if_kip_assert(min_and_op[i].minimum >= 0);
   }

   // The logical-ands operator is mutually reflexive, so we can arbitrarily
   // rearrange its operands. Putting operands with larger minima first tends
   // to speed up the operator.
   std::sort(min_and_op.begin(), min_and_op.end(), internal::more<real,tag>());

   // Bookkeeping.
   // Note: if we do input followed by output of an "ands" object, then because
   // of the upcoming change of the vec[i].op's, operands may have been
   // rearranged. The same is true for other nary operators.

   bool in_all = true;   // remains true  if no objects
   bool in_ge1 = false;  // remains false if no objects
   kip_data.total_in = 0;

   for (size_t i = 0;  i < kip_data.nop;  ++i)
      if ((vec[i].in = (vec[i].op=min_and_op[i].shape)->interior))  // =, not ==
         in_ge1 = true, kip_data.total_in++;
      else
         in_all = false;
   interior = kip_data.total_in == kip_data.nop;

   // minimum
   real rv = kip_data.nop ? min_and_op[0].minimum : real(0);
   if (in_all)
      // in ALL; use minimum of minima (need to exit any)
      for (size_t i = 1;  i < kip_data.nop;  ++i)
         rv = op::min(rv, real(min_and_op[i].minimum));
   else if (in_ge1) {
      // in >=1; use maximum of not-in minima (need to enter all others)
      rv = real(0);
      for (size_t i = 0;  i < kip_data.nop;  ++i)
         if (!vec[i].in)
            rv = op::max(rv, real(min_and_op[i].minimum));
   } else
      // in NONE; use maximum of minima (need to enter all)
      for (size_t i = 1;  i < kip_data.nop;  ++i)
         rv = op::max(rv, real(min_and_op[i].minimum));
   return rv;
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(ands)
{
   const vec_t &vec = kip_data.vec();
   if ((kip_data.nop = vec.size()) == 0)
      return bbox<real>(false,1,0,false, false,1,0,false, false,1,0,false);

   const bbox<real> a = vec[0].op->aabb();
   real xmin = a.x.min, xmax = a.x.max;
   real ymin = a.y.min, ymax = a.y.max;
   real zmin = a.z.min, zmax = a.z.max;

   // By construction, if !vec[i].op->aabb().valid() for any i,
   // then the return value is !valid()
   for (size_t i = 1;  i < kip_data.nop;  ++i) {
      const bbox<real> b = vec[i].op->aabb();
      xmin = op::max(xmin, real(b.x.min)), xmax = op::min(xmax, real(b.x.max));
      ymin = op::max(ymin, real(b.y.min)), ymax = op::min(ymax, real(b.y.max));
      zmin = op::max(zmin, real(b.z.min)), zmax = op::min(zmax, real(b.z.max));
   }

   return bbox<real>(
      false,xmin, xmax,false,
      false,ymin, ymax,false,
      false,zmin, zmax,false
   );
} kip_end



// dry
kip_dry(ands)
{
   // Could be stronger. The intersection may still be dry, even
   // if none of the operands is dry. But that's difficult to determine.
   const vec_t &vec = kip_data.vec();

   for (size_t i = 0;  i < kip_data.nop;  ++i)
      if (vec[i].op->dry(seg)) return true;
   return false;
   // note: false if no operands
} kip_end



// random
kip_random(ands)
{
   const size_t nop = 4;

   const point<real> loc(
      op::twice(random<real>() - real(0.5)),
      op::twice(random<real>() - real(0.5)),
      op::twice(random<real>() - real(0.5))
   );
   obj.clear();
   obj.misc.ands.vec().reserve(nop);

   for (size_t i = 0;  i < nop;  ++i) {
      sphere<real,tag> *const ptr = new sphere<real,tag>;
      random(*ptr);
      obj.misc.ands.push().op = ptr;

      ptr->r += ptr->r;
      ptr->c  = loc + point<real>(
         real(0.2)*random<real>(),
         real(0.2)*random<real>(),
         real(0.2)*random<real>()
      );
   }

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(ands)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop == 0) return false;  // no operands --> no intersection

   // INSIDE: we're inside all operands; the overall status changes (from
   // inside to outside) at the first intersection with any object's boundary
   if (interior) {
      inq<real,tag> qtmp;  q = qmin;

      for (size_t i = 0;  i < kip_data.nop;  ++i)
         if (internal::op_first(vec[i].op, kip_etd, real(q),qtmp, insub))
            q = qtmp;
      return q < qmin;
   }

   // OUTSIDE: we're outside one or more operands; the overall status changes
   // (from outside to inside) at the point after which we're inside all objects

   const size_t num_operand = kip_data.nop;
   typedef afew_book< inq<real,tag> > per_operand;
#include "kip-macro-workspace.h"

   // compute operand information
   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      const bool found =
         internal::op_all(vec[i].op, kip_etd, qmin,operand[i].points,
                      insub);

      if (found || vec[i].in) {
         operand[i].size   = found ? operand[i].points.size() : 0;
         operand[i].next   = 0;
         operand[i].in_now = vec[i].in;
      } else
         // there exists an operand with no ray intersection, AND we're not
         // inside it, so the ray can't intersect with the logical conjunction
         return kip_less, false;
   }

   // search for the first relevant intersection. "current" will be the operand
   // that provides this (entry) point, and it's the relevant one if there's
   // no previous such operand, or if this one is closer
   size_t num_in  = kip_data.total_in;

   for (int current = -1 ;; ++operand[current].next) {
      for (size_t i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size) {
            // there are still more intersections with i
            if (current == -1 || operand[i].q() < operand[current].q())
               current = int(i);
         } else if (!operand[i].in_now)
            // there are no more intersections with i, and
            // we're outside of it
            return kip_less, false;

      // if here: never (over all operands) at the end without being in
      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (++num_in < kip_data.nop) continue;
      } else {
         // leaving current operand
         --num_in;  continue;
      }

      return q = operand[current].q(), kip_less, true;
   }
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(ands)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop == 0) return false;  // no operands --> no intersection

   const size_t num_operand = kip_data.nop;
   typedef afew_book< inq<real,tag> > per_operand;
#include "kip-macro-workspace.h"

   // compute operand information
   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      const bool found =
         internal::op_all(vec[i].op, kip_etd, qmin,operand[i].points,
                      insub);

      if (found || vec[i].in) {
         operand[i].size   = found ? operand[i].points.size() : 0;
         operand[i].next   = 0;
         operand[i].in_now = vec[i].in;
      } else
         return kip_less, false;
   }

   size_t num_in = kip_data.total_in;
   ints.reset();

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (size_t i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size) {
            // there are still more intersections with i
            if (current == -1 || operand[i].q() < operand[current].q())
               current = int(i);
         } else if (!operand[i].in_now)
            // there are no more intersections with i, and we're outside of it
            return kip_less, ints.size() > 0;
      if (current == -1) return kip_less, ints.size() > 0;

      // if here: never (over all operands) at the end without being in
      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (++num_in < kip_data.nop) continue;
      } else
         // leaving current operand
         if (num_in-- < kip_data.nop) continue;

      ints.push(operand[current].q());
   }

   return kip_less, false;  // unreachable; presence silences g++ warning
} kip_end

}



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#include "kip-macro-io-nary.h"