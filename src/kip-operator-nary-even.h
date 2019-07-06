
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// even
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class even : public shape<real,tag> {
   using shape<real,tag>::interior;

#define   kip_class even
#include "kip-macro-nary.h"
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(even)
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

   // The logical-even operator is mutually reflexive, so we can arbitrarily
   // rearrange its operands. Putting operands with smaller minima first tends
   // to speed up the operator.
   std::sort(min_and_op.begin(), min_and_op.end(), internal::less<real,tag>());

   // Bookkeeping
   nary.total_in = 0;
   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      vec[i].min = min_and_op[i].minimum;
      if ((vec[i].in=(vec[i].op=min_and_op[i].shape)->interior))  // =, not ==
         nary.total_in++;
   }
   interior = kip_data.nop > 1 ? nary.total_in && nary.total_in % 2 == 0 : true;

   // minimum
   if (kip_data.nop < 2) return 0;
   // else...at least 2 operands...

   // in 0
   if (nary.total_in == 0)
      return min_and_op[1].minimum;  // get to at least two (minima [0] and [1])

   // in 1
   if (nary.total_in == 1) {
      // get to at least one other
      // qqq in fact I think we just need to check [0] or [1] here...
      for (size_t i = 0;  i < kip_data.nop;  ++i)
         if (!vec[i].in)
            return min_and_op[i].minimum;
      assert(false);
   }

   // in > 1: get to any border
   return min_and_op[0].minimum;
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// twosort - helper
namespace internal {
   template<class CONTAINER, class COMPARE>
   inline void twosort(CONTAINER &c, const COMPARE &comp)
   {
      if (c.size() > 1)
         std::partial_sort(c.begin(), c.begin()+2, c.end(), comp);
   }
}



// aabb
kip_aabb(even)
{
   bbox<real> b(false,1,0,false, false,1,0,false, false,1,0,false);
   std::vector<real> xmin,ymin,zmin, xmax,ymax,zmax;

   const vec_t &vec = kip_data.vec();
   kip_data.nop = vec.size();

   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      b = vec[i].op->aabb();

      if (b.x.valid()) {
         if (b.x.min.finite()) xmin.push_back(b.x.min);
         if (b.x.max.finite()) xmax.push_back(b.x.max); }
      if (b.y.valid()) {
         if (b.y.min.finite()) ymin.push_back(b.y.min);
         if (b.y.max.finite()) ymax.push_back(b.y.max); }
      if (b.z.valid()) {
         if (b.z.min.finite()) zmin.push_back(b.z.min);
         if (b.z.max.finite()) zmax.push_back(b.z.max); }
   }

   twosort(xmin,internal::less<real,tag>()), twosort(xmax,internal::more<real,tag>());
   twosort(ymin,internal::less<real,tag>()), twosort(ymax,internal::more<real,tag>());
   twosort(zmin,internal::less<real,tag>()), twosort(zmax,internal::more<real,tag>());

   return kip::bbox<real>(
      false, xmin.size() > 1 ? xmin[1] : +std::numeric_limits<real>::max(),
             xmax.size() > 1 ? xmax[1] : -std::numeric_limits<real>::max(), false,
      false, ymin.size() > 1 ? ymin[1] : +std::numeric_limits<real>::max(),
             ymax.size() > 1 ? ymax[1] : -std::numeric_limits<real>::max(), false,
      false, zmin.size() > 1 ? zmin[1] : +std::numeric_limits<real>::max(),
             zmax.size() > 1 ? zmax[1] : -std::numeric_limits<real>::max(), false
   );
} kip_end



// dry
kip_dry(even)
{
   unsigned char wet = 0;

   const vec_t &vec = kip_data.vec();
   for (size_t i = 0;  i < kip_data.nop;  ++i)
      if (!vec[i].op->dry(seg) && ++wet == 2) return false;
   return kip_data.nop > 0;  // false if no operands, true otherwise
} kip_end



// random
kip_random(even)
{
   const size_t nop = 4;

   const point<real> loc(
      op::twice(random<real>() - real(0.5)),
      op::twice(random<real>() - real(0.5)),
      op::twice(random<real>() - real(0.5))
   );
   obj.clear();
   obj.nary.vec().reserve(nop);

   for (size_t i = 0;  i < nop;  ++i) {
      sphere<real,tag> *const ptr = new sphere<real,tag>;
      random(*ptr);
      obj.nary.push().op = ptr;

      ptr->c = loc + point<real>(
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

kip_infirst(even)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop < 2) return false;  // 0 or 1 operands --> no intersection

   // Inside 2 or more operands...
   if (nary.total_in > 1) {
      inq<real,tag> qtmp;  q = qmin;
      for (size_t i = 0;  i < kip_data.nop && vec[i].min < q;  ++i)
         if (internal::op_first(vec[i].op, kip_etd, real(q),qtmp, insub))
            q = qtmp;
      return q < qmin;
   }

   // Inside 0 or 1 operands...
   const size_t num_operand = kip_data.nop;
   using per_operand = afew_book< inq<real,tag> >;
#include "kip-macro-workspace.h"

   // compute operand information
   bool found = false;
   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      operand[i].size =
         vec[i].min < qmin &&
         internal::op_all(vec[i].op, kip_etd,
                      qmin,operand[i].points, insub)
         ? (found = true, operand[i].points.size())
         :  0;
      operand[i].next = 0, operand[i].in_now = vec[i].in;
   }
   if (!found) return kip_less, false;

   // search for the first relevant intersection.
   size_t num_in = nary.total_in;

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (size_t i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size &&
             (current == -1 || operand[i].q() < operand[current].q()))
            current = int(i);
      if (current == -1) return kip_less, false;

      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (num_in++)
            return
               q = operand[current].q().reverse(num_in % 2 != 0),
               kip_less, true;
      } else
         // leaving current operand
         num_in--;
   }

   return kip_less, false;  // unreachable; presence silences g++ warning
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(even)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop < 2) return false;  // 0 or 1 operands --> no intersection

   const size_t num_operand = kip_data.nop;
   using per_operand = afew_book<inq<real,tag>>;
#include "kip-macro-workspace.h"

   // compute operand information
   bool found = false;
   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      operand[i].size =
         vec[i].min < qmin &&
         internal::op_all(vec[i].op, kip_etd,
                      qmin,operand[i].points, insub)
         ? (found = true, operand[i].points.size())
         :  0;
      operand[i].next = 0, operand[i].in_now = vec[i].in;
   }
   if (!found) return kip_less, false;

   size_t num_in = nary.total_in;
   ints.reset();

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (size_t i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size &&
             (current == -1 || operand[i].q() < operand[current].q()))
             current = int(i);
      if (current == -1) return kip_less, ints.size() > 0;

      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (num_in++)
            ints.push(operand[current].q().reverse(  num_in % 2 != 0));
      } else
         // leaving current operand
         if (--num_in)
            ints.push(operand[current].q().reverse(!(num_in % 2 != 0)));
   }

   return kip_less, false;  // unreachable; presence silences g++ warning
} kip_end

}



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#include "kip-macro-io-nary.h"
