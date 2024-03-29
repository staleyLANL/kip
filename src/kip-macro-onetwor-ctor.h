
// This file is #included in various places to help create
// primitives with {a,b,r} (or {c,n,r}).

#ifndef kip_aye
#define kip_aye a
#endif

#ifndef kip_bee
#define kip_bee b
#endif

   // kip_class([a[,b[,r[,base]]]])
   explicit kip_class(
      const point<real> &_a = point<real>(0,0,0),
      const point<real> &_b = point<real>(1,0,0),
      const real _r = 1
   ) :
      shape<real,tag>(this),
      kip_aye(_a), kip_bee(_b), r(_r)
   { }

   explicit kip_class(
      const point<real> &_a,
      const point<real> &_b, const real _r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      kip_aye(_a), kip_bee(_b), r(_r)
   { }



   // kip_class(a.x,a.y,a.z[,b.x,b.y,b.z[,r[,base]]])
   explicit kip_class(
      const real ax, const real ay, const real az
   ) :
      shape<real,tag>(this),
      kip_aye(ax,ay,az),
      kip_bee(1,0,0),
      r(1)
   { }

   explicit kip_class(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz
   ) :
      shape<real,tag>(this),
      kip_aye(ax,ay,az),
      kip_bee(bx,by,bz),
      r(1)
   { }

   explicit kip_class(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz, const real _r
   ) :
      shape<real,tag>(this),
      kip_aye(ax,ay,az),
      kip_bee(bx,by,bz),
      r(_r)
   { }

   explicit kip_class(
      const real ax, const real ay, const real az,
      const real bx, const real by, const real bz,
      const real _r, const tag &thebase
   ) :
      shape<real,tag>(this,thebase),
      kip_aye(ax,ay,az),
      kip_bee(bx,by,bz),
      r(_r)
   { }



   // kip_class(kip_class)
   kip_class(const kip_class &from) :
      shape<real,tag>(from),
      kip_aye(from.kip_aye),
      kip_bee(from.kip_bee),
      r(from.r)
   { }

   // kip_class = kip_class
   kip_class &operator=(const kip_class &from)
   {
      this->shape<real,tag>::operator=(from);
      kip_aye = from.kip_aye;
      kip_bee = from.kip_bee;
      r = from.r;
      return *this;
   }

#undef kip_aye
#undef kip_bee
#undef kip_class
