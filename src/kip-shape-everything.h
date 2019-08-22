
#pragma once

// -----------------------------------------------------------------------------
// everything
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class everything : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   kip_functions(everything);
   inline point<real> back(const point<real> &from) const { return from; }

   // everything([base])
   inline explicit everything() : shape<real,tag>(this) { }
   inline explicit everything(const tag &thebase) :
      shape<real,tag>(this,thebase)
   { }
};



// process
kip_process(everything)
{
   interior = true;
   return std::numeric_limits<minimum_t>::max();
} kip_end


// aabb
kip_aabb(everything)
{
   const real infinity = std::numeric_limits<real>::max();
   return bbox<real>(
      false,-infinity, infinity,false,
      false,-infinity, infinity,false,
      false,-infinity, infinity,false
   );
} kip_end


// dry, infirst, inall, check
kip_dry    (everything) { return false; } kip_end
kip_infirst(everything) { return false; } kip_end
kip_inall  (everything) { return false; } kip_end
kip_check  (everything) { return diagnostic_t::diagnostic_good; } kip_end


// randomize
kip_randomize(everything)
{
   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
template<class ISTREAM, class real, class tag>
bool read_value(
   ISTREAM &s, everything<real,tag> &obj,
   const std::string &description = "\"everything\" object"
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
kip_ostream(everything) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << "everything()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << "everything(" && write_finish(k, obj, true);

   // full
   else
      okay = k << "everything(" && write_finish(k, obj, false);

   // done
   if (!okay)
      (void)error("Could not write \"everything\" object");
   return k;
}

#define   kip_class everything
#include "kip-macro-io.h"
