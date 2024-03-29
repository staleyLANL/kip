
// -----------------------------------------------------------------------------
// indsn
// Meaning: "intersection, normal, distance, shape, normalized"
// -----------------------------------------------------------------------------

template<class real, class base>
class indsn {
public:
   // data
   point<real> intersection;            // ray/shape intersection
   point<real> normal;                  // shape's surface normal
   real distance;                       // eyeball/intersection distance
   const kip::shape<real,base> *shape;  // shape at this pixel
   bool isnormalized;                   // is |normal| == 1?

   // conversion to bool (meaning: ray hits shape)
   operator bool() const
   {
      return shape != nullptr;
   }

   // initialize
   void initialize()
   {
      shape = nullptr;
   }

   // indsn: set
   void set(const inq<real,base> &q)
   {
      // true ray/shape intersection
      intersection = q.shape->back(q.fac > 0 ? q.fac*q : point<real>(q));

      // true shape normal
      // zzz somehow tell back() to *rotate* only (not also translate)
      normal = q.shape->back(q.n) - q.shape->back(point<real>(0,0,0));

      // true eyeball/intersection distance
      distance = q.q;  // zzz q.fac scaling?

      // intersected shape; always a primitive, never an operator!
      shape = q.shape;

      // is |normal| == 1 ?
      isnormalized = q.isnormalized == normalized::yes;
   }
};
