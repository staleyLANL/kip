
#pragma once

// -----------------------------------------------------------------------------
// Certain forward declarations
// I had these here, as they seemed to be necessary only with the Intel C++
// compiler, but then discovered that clang didn't work without them. So, I
// think they're probably actually required, but other compilers (g++ and
// pgcc) are lax about it. Anyway, they really should go into a different file.
// -----------------------------------------------------------------------------

namespace kip {
   class istream;
   class ostream;
}

#define kip_forward(type)\
   namespace kip {\
      template<class real, class tag>\
      class type;\
      \
      template<class real, class tag>\
      kip::istream &operator>>(kip::istream &, kip::type<real,tag> &);\
      \
      template<class real, class tag>\
      std::istream &operator>>(std::istream &, kip::type<real,tag> &);\
      \
      template<class real, class tag>\
      kip::ostream &operator<<(kip::ostream &, const kip::type<real,tag> &);\
      \
      template<class real, class tag>\
      std::ostream &operator<<(std::ostream &, const kip::type<real,tag> &);\
   }

   kip_expand_plain(kip_forward)
   kip_extra_plain(kip_forward)
#undef kip_forward
