
// -----------------------------------------------------------------------------
// marble
// -----------------------------------------------------------------------------

template<class BASE, class real = default_real>
class marble {
public:
   // for i/o
   static const std::string description;

   real amp, ampfac;
   real per, perfac;
   unsigned nfun;
   unsigned seed;
   BASE base;
   bool swirl;

   // marble([base[,amp[,ampfac[,per[,perfac[,nfun[,seed[,swirl]]]]]]]])
   explicit marble(
      const BASE &_base = BASE{},
      const real _amp = 1, const real _ampfac = 0.5,
      const real _per = 1, const real _perfac = 0.5,
      const unsigned _nfun = 1,
      const unsigned _seed = 0,
      const bool _swirl = false
   ) :
      amp(_amp), ampfac(_ampfac),
      per(_per), perfac(_perfac),
      nfun(_nfun),
      seed(_seed),
      base(_base),
      swirl(_swirl)
   { }
};

// description
template<class basis, class real>
const std::string marble<basis,real>::description =
   std::string("marble<") + basis::description + ">";



// -----------------------------------------------------------------------------
// randomize
// -----------------------------------------------------------------------------

template<class BASE, class real>
inline marble<BASE,real> &randomize(marble<BASE,real> &obj)
{
   randomize(obj.base);

   obj.amp    = 1 + random_unit<real>();
   obj.ampfac = 1;
   obj.per    = 0.05*(1 + random_unit<real>());
   obj.perfac = 1;
   obj.nfun   = 1;
   obj.seed   = rand() % 1000000;
   obj.swirl  = false;

   return obj;
}



// -----------------------------------------------------------------------------
// read_value
// -----------------------------------------------------------------------------

template<class ISTREAM, class BASE, class real>
bool read_value(ISTREAM &s, marble<BASE,real> &obj)
{
   s.bail = false;
   int swirl = 0; // so we read as int, not bool

   if (!(
      read_value(s,obj.base  ) && read_comma(s) &&
      read_value(s,obj.amp   ) && read_comma(s) &&
      read_value(s,obj.ampfac) && read_comma(s) &&
      read_value(s,obj.per   ) && read_comma(s) &&
      read_value(s,obj.perfac) && read_comma(s) &&
      read_value(s,obj.nfun  ) && read_comma(s) &&
      read_value(s,obj.seed  ) && read_comma(s) &&
      read_value(s,swirl)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + obj.description, diagnostic::error);
   }

   obj.swirl = swirl;
   return !s.fail();
}



// -----------------------------------------------------------------------------
// convert
// -----------------------------------------------------------------------------

// marble<BASE,real> ==> RGB<comp>
template<class BASE, class real, class comp>
inline void convert(const marble<BASE,real> &in, kip::RGB<comp> &out)
{
   convert(in.base,out);
}

// marble<BASE,real> ==> RGBA<comp>
template<class BASE, class real, class comp>
inline void convert(const marble<BASE,real> &in, kip::RGBA<comp> &out)
{
   convert(in.base,out);
}
