
// -----------------------------------------------------------------------------
// light
// -----------------------------------------------------------------------------

template<class real = defaults::real>
class light {

   // light source
   point<real> primary = point<real>(0,-10,5);

public:

   // operator[]
   point<real> &operator[](const ulong) { return primary; }
   const point<real> &operator[](const ulong) const { return primary; }

   // fix
   const light &fix() const
   {
      // currently, nothing needs fixing
      return *this;
   }
};
