
// This file is #included in various places, to create member functions
// for nary operator classes.

// for ands: misc_type::ands_type
#ifndef kip_type
#define kip_type nary_type
#endif

// for ands: misc.ands
#ifndef kip_data
#define kip_data nary
#endif

public:

   using shape<real,tag>::base;  // for base()
   using shape<real,tag>::misc;  // for ands
   using shape<real,tag>::nary;  // for nary != ands

   using misc_type = typename shape<real,tag>::misc_type;
   using nary_type = typename shape<real,tag>::nary_type;
   using vec_t = typename kip_type::vec_t;



   // ------------------------
   // Helpers
   // ------------------------

private:

   // propagate_base_last
   inline shape<real,tag> &propagate_base_last() const
   {
      shape<real,tag> &ref = *kip_data.vec().back().op;
      if (!ref.baseset) {
         ref.base() = base();  // side effect: marks as set
         ref.baseset = false;  // re-flag
      }
      return ref;
   }

   // copy
   inline void copy(const kip_class &from)
   {
      kip_data.vec().reserve(kip_data.nop = from.size());

      for (ulong n = 0;  n < kip_data.nop;  ++n)
         kip_data.push().op =
              from.kip_data.vec()[n].op
            ? from.kip_data.vec()[n].op->duplicate()
            : nullptr;
      // don't need to bring over .in and (when applicable) .min; those
      // are used only internally, by kip, and are initialized as needed.
   }



   // ------------------------
   // Miscellaneous
   // ------------------------

public:

   // size
   inline ulong size() const
      { return kip_data.vec().size(); }

   // back
   inline point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   // propagate_base
   inline void propagate_base(const bool force = false) const
      { detail::propagate_nary(kip_data.vec(), base(), force); }

   // clear
   inline void clear()
   {
      for (ulong n = size();  n ; )
         delete kip_data.vec()[--n].op;
      kip_data.vec().clear();
   }



   // ------------------------
   // Constructors
   // Copy assignment
   // Destructor
   // ------------------------

   // kip_class()
   inline explicit kip_class() : shape<real,tag>(this)
   {
      new (&kip_data.vec()) vec_t;
   }

   // kip_class(base)
   inline explicit kip_class(const tag &thebase) : shape<real,tag>(this,thebase)
   {
      new (&kip_data.vec()) vec_t;
   }

   // kip_class(kip_class)
   inline kip_class(const kip_class &from) : shape<real,tag>(from)
   {
      new (&kip_data.vec()) vec_t;
      copy(from);
   }


   // kip_class = kip_class
   inline kip_class &operator=(const kip_class &from)
   {
      this->shape<real,tag>::operator=(from);
      clear();
      copy(from);
      return *this;
   }


   // destructor
   inline ~kip_class()
   {
      clear();
      kip_data.vec().~vec_t();
   }



   // ------------------------
   // push
   // ------------------------

   // from pointer
   inline shape<real,tag> &push(const shape<real,tag> *const ptr)
   {
      kip_data.push().op = ptr ? ptr->duplicate() : nullptr;
      return propagate_base_last();
   }

   // from reference
   inline shape<real,tag> &push(const shape<real,tag> &ref)
   {
      //kip_data.push().op = &ref ? ref.duplicate() : nullptr;
      kip_data.push().op = ref.duplicate();
      return propagate_base_last();
   }



   // ------------------------
   // check
   // ------------------------

   kip_functions(kip_class)  // ends with check() header
   {
      diagnostic d = diagnostic::good;
      kip_data.nop = size();

      for (ulong n = 0;  n < kip_data.nop;  ++n) {
         std::ostringstream oss;
         oss << '[' << n << ']';

         d = op::min(
            d,
            detail::check_operand(
               kip_quote(kip_class), kip_data.vec()[n].op, oss.str().c_str())
         );
      }

      return d;
      // note: good if no operands
   }
