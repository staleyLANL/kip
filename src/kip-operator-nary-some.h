
#pragma once

// -----------------------------------------------------------------------------
// some
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class some : public shape<real,tag> {
   using shape<real,tag>::interior;

#define   kip_class some
#include "kip-macro-nary.h"
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(some)
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

   // The logical-some operator is mutually reflexive, so we can arbitrarily
   // rearrange its operands. Putting operands with smaller minima first tends
   // to speed up the operator.
   std::sort(min_and_op.begin(), min_and_op.end(), detail::less<real,tag>());

   // Bookkeeping
   nary.total_in = 0;
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      vec[i].min = min_and_op[i].minimum;
      if ((vec[i].in=(vec[i].op=min_and_op[i].shape)->interior))  // =, not ==
         nary.total_in++;
   }
   interior = kip_data.nop < 2 ||
      (nary.total_in && nary.total_in != kip_data.nop);

   // minimum...

   // In contrast with other nary operators, not only does 0 operands mean
   // there's effectively nothing, but 1 operand means this too. After all,
   // some means neither none nor *all* of the operands.
   if (kip_data.nop < 2) return 0;
   // else...at least 2 operands...

   // If in none, or in all; hit anything to get "some"
   if (nary.total_in == 0 || nary.total_in == kip_data.nop) {
      real rv = min_and_op[0].minimum;
      for (ulong i = 1;  i < kip_data.nop;  ++i)
         rv = op::min(rv, real(min_and_op[i].minimum));
      return rv;
   }

   // If in some, either exit all ins, or enter all outs
   real ins = 0, outs = 0;
   for (ulong i = 0;  i < kip_data.nop;  ++i)
      if (vec[i].in) ins  = op::max(ins,  real(min_and_op[i].minimum));
      else            outs = op::max(outs, real(min_and_op[i].minimum));
   return op::min(ins,outs);
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(some)
{
   // zzz I think we could do slightly better than this; if nop >= 3
   // zzz and the innermost is strictly inside, then perhaps tight?
   bbox<real> rv(false,1,0,false, false,1,0,false, false,1,0,false);

   const vec_t &vec = kip_data.vec();
   kip_data.nop = vec.size();

   for (ulong i = 0;  i < kip_data.nop;  ++i)
      rv = detail::bound_combine(rv, vec[i].op->aabb(), detail::op_less());
   return rv;
} kip_end



// dry
kip_dry(some)
{
   const vec_t &vec = kip_data.vec();
   for (ulong i = 0;  i < kip_data.nop;  ++i)
      if (!vec[i].op->dry(seg)) return false;  // makes the whole thing non-dry
   return kip_data.nop > 1;  // false if 0 or 1 operands, true otherwise
} kip_end



// randomize
kip_randomize(some)
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

kip_infirst(some)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop < 2) return false;  // 0 or 1 operands --> no intersection

   // Inside none or all operands (equivalent to !in some)...
   if (!interior) {
      // Find the first hit with anything. It represents one of the following,
      // each of which amounts to a border of the some() object:
      //    in none --> in some
      //    in all  --> in some
      inq<real,tag> qtmp;  q = qmin;
      for (ulong i = 0;  i < kip_data.nop && vec[i].min < q;  ++i)
         if (detail::op_first(vec[i].op, kip_etd, real(q),qtmp, insub))
            q = qtmp;
      return q < qmin;
   }

   // Inside some...
   const ulong num_operand = kip_data.nop;
   using per_operand = afew_book<inq<real,tag>>;
#include "kip-macro-workspace.h"

   // compute operand information
   bool found = false;
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      operand[i].size =
         vec[i].min < qmin &&
         detail::op_all(vec[i].op, kip_etd,
                      qmin,operand[i].points, insub)
         ? (found = true, operand[i].points.size())
         :  0;
      operand[i].next = 0, operand[i].in_now = vec[i].in;
   }
   if (!found) return kip_less, false;

   // search for the first relevant intersection.
   ulong num_in = nary.total_in;

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (ulong i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size &&
             (current == -1 || operand[i].q() < operand[current].q()))
            current = int(i);
      if (current == -1) return kip_less, false;

      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (++num_in == kip_data.nop)  // some --> all...
            return q = operand[current].q().reverse(true ), kip_less, true;
      } else
         // leaving current operand
         if (--num_in == 0)  // some --> none...
            return q = operand[current].q().reverse(false), kip_less, true;
   }

   return kip_less, false;  // unreachable; presence silences g++ warning
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(some)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop < 2) return false;  // 0 or 1 operands --> no intersection

   const ulong num_operand = kip_data.nop;
   using per_operand = afew_book<inq<real,tag>>;
#include "kip-macro-workspace.h"

   // compute operand information
   bool found = false;
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      operand[i].size =
         vec[i].min < qmin &&
         detail::op_all(vec[i].op, kip_etd,
                      qmin,operand[i].points, insub)
         ? (found = true, operand[i].points.size())
         :  0;
      operand[i].next = 0, operand[i].in_now = vec[i].in;
   }
   if (!found) return kip_less, false;

   ulong num_in = nary.total_in;
   ints.reset();

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (ulong i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size &&
             (current == -1 || operand[i].q() < operand[current].q()))
             current = int(i);
      if (current == -1) return kip_less, ints.size() > 0;

      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (++num_in == 1)
            ints.push(operand[current].q().reverse(false));  // none --> some
         else if (num_in == kip_data.nop)
            ints.push(operand[current].q().reverse(true ));  // some --> all
      } else
         // leaving current operand
         if (num_in-- == kip_data.nop)
            ints.push(operand[current].q().reverse(true ));  // all  --> some
         else if (num_in == 0)
            ints.push(operand[current].q().reverse(false));  // some --> none
   }

   return kip_less, false;  // unreachable; presence silences g++ warning
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#include "kip-macro-io-nary.h"
