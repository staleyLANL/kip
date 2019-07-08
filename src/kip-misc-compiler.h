
#pragma once

// -----------------------------------------------------------------------------
// Certain forward declarations
// I had these here, as they seemed to be necessary only with the Intel C++
// compiler, but then discovered that clang didn't work without them. So, I
// think they're probably actually required, but other compilers (g++ and
// pgcc) are lax about it. Anyway, they really should go into a different file.
// -----------------------------------------------------------------------------

class istream;
class ostream;

#define kip_forward(type)\
   template<class real, class tag>\
   class type;\
   \
   template<class real, class tag>\
   kip::istream &operator>>(kip::istream &, type<real,tag> &);\
   \
   template<class real, class tag>\
   std::istream &operator>>(std::istream &, type<real,tag> &);\
   \
   template<class real, class tag>\
   kip::ostream &operator<<(kip::ostream &, const type<real,tag> &);\
   \
   template<class real, class tag>\
   std::ostream &operator<<(std::ostream &, const type<real,tag> &);\

   kip_expand_plain(kip_forward)
   kip_extra_plain(kip_forward)
#undef kip_forward
