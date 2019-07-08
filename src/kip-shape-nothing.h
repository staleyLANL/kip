
#pragma once

// -----------------------------------------------------------------------------
// nothing
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class nothing : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   kip_functions(nothing);
   inline point<real> back(const point<real> &from) const { return from; }

   // nothing([base])
   inline explicit nothing() : shape<real,tag>(this) { }
   inline explicit nothing(const tag &thebase) :
      shape<real,tag>(this,thebase)
   { }
};



// process
kip_process(nothing)
{
   interior = false;
   return std::numeric_limits<minimum_t>::max();
} kip_end


// aabb
kip_aabb(nothing)
{
   return bbox<real>(
      false,1, 0,false,
      false,1, 0,false,
      false,1, 0,false
   );
} kip_end


// dry, infirst, inall, check
kip_dry    (nothing) { return true;  } kip_end
kip_infirst(nothing) { return false; } kip_end
kip_inall  (nothing) { return false; } kip_end
kip_check  (nothing) { return diagnostic_t::diagnostic_good; } kip_end


// random
kip_random(nothing)
{
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
template<class ISTREAM, class real, class tag>
bool read_value(
   ISTREAM &s, nothing<real,tag> &obj,
   const std::string &description = "\"nothing\" object"
) {
   // (
   //    base
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_done(s, obj, '\0')
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(nothing) {
   bool okay;

   // stub
   if (kip::format == kip::format_t::format_stub)
      okay = k << "nothing()";

   // one
   // op
   else if (kip::format == kip::format_t::format_one ||
            kip::format == kip::format_t::format_op)
      okay = k << "nothing(" && kip::write_finish(k, obj, true);

   // full
   else
      okay = k << "nothing(" && kip::write_finish(k, obj, false);

   // done
   if (!okay)
      (void)kip::error("Could not write \"nothing\" object");
   return k;
}

#define   kip_class nothing
#include "kip-macro-io.h"
