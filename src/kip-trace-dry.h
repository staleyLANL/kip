
#pragma once

#define kip_dry_function(name)\
   template<class real, class base>\
   inline rotate<-3,real> name(\
      const internal::vars<real,base> &vars, const real h, const real v\
   )



// -----------------------------------------------------------------------------
// n: North
// s: South
// e: East
// w: West
// -----------------------------------------------------------------------------

// n
template<class real, class base>
inline rotate<-3,real> dry_n
  (const internal::vars<real,base> &vars, const real v)
{
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_01n(v-vars.veps),   // (1,v)
      vars.t2e.back_00n(v-vars.veps));  // (0,v)
}

// s
template<class real, class base>
inline rotate<-3,real> dry_s
  (const internal::vars<real,base> &vars, const real v)
{
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_00n(v+vars.veps),   // (0,v)
      vars.t2e.back_01n(v+vars.veps));  // (1,v)
}

// e
template<class real, class base>
inline rotate<-3,real> dry_e
  (const internal::vars<real,base> &vars, const real h)
{
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0n0(h-vars.heps),   // (h,0)
      vars.t2e.back_0n1(h-vars.heps));  // (h,1)
}

// w
template<class real, class base>
inline rotate<-3,real> dry_w
  (const internal::vars<real,base> &vars, const real h)
{
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0n1(h+vars.heps),   // (h,1)
      vars.t2e.back_0n0(h+vars.heps));  // (h,0)
}



// -----------------------------------------------------------------------------
// ne: NorthEast
// nw: NorthWest
// se: SouthEast
// sw: SouthWest
// -----------------------------------------------------------------------------

// ne
kip_dry_function(dry_ne) {
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h-vars.heps-1,v-vars.veps+1));
}

// nw
kip_dry_function(dry_nw) {
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h+vars.heps-1,v-vars.veps-1));
}

// se
kip_dry_function(dry_se) {
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h-vars.heps+1,v+vars.veps+1));
}

// sw
kip_dry_function(dry_sw) {
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h+vars.heps+1,v+vars.veps-1));
}



// -----------------------------------------------------------------------------
// nne: NorthNorthEast
// ene: EastNorthEast
// nnw: NorthNorthWest
// wnw: WestNorthWest
// sse: SouthSouthEast
// ese: EastSouthEast
// ssw: SouthSouthWest
// wsw: WestSouthWest
// -----------------------------------------------------------------------------

// a = tan(pi/6) = tan(30)
// b = tan(pi/3) = tan(60)

// nne
kip_dry_function(dry_nne) {
   /*static*/ const real a = op::tan(pi<real>/6);  // about 0.577
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h-vars.heps-1,v-vars.veps+a));
}

// ene
kip_dry_function(dry_ene) {
   /*static*/ const real b = op::tan(pi<real>/3);  // about 1.732
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h-vars.heps-1,v-vars.veps+b));
}

// nnw
kip_dry_function(dry_nnw) {
   /*static*/ const real a = op::tan(pi<real>/6);  // about 0.577
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h+vars.heps-1,v-vars.veps-a));
}

// wnw
kip_dry_function(dry_wnw) {
   /*static*/ const real b = op::tan(pi<real>/3);  // about 1.732
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h+vars.heps-1,v-vars.veps-b));
}

// sse
kip_dry_function(dry_sse) {
   /*static*/ const real a = op::tan(pi<real>/6);  // about 0.577
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h-vars.heps+1,v+vars.veps+a));
}

// ese
kip_dry_function(dry_ese) {
   /*static*/ const real b = op::tan(pi<real>/3);  // about 1.732
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h-vars.heps+1,v+vars.veps+b));
}

// ssw
kip_dry_function(dry_ssw) {
   /*static*/ const real a = op::tan(pi<real>/6);  // about 0.577
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h+vars.heps+1,v+vars.veps-a));
}

// wsw
kip_dry_function(dry_wsw) {
   /*static*/ const real b = op::tan(pi<real>/3);  // about 1.732
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h+vars.heps+1,v+vars.veps-b));
}



// -----------------------------------------------------------------------------
// nnne: NorthNorthNorthEast
// eene: EastEastNorthEast
// nnnw: NorthNorthNorthWest
// wwnw: WestWestNorthWest
// ssse: SouthSouthSouthEast
// eese: EastEastSouthEast
// sssw: SouthSouthSouthWest
// wwsw: WestWestSouthWest
// -----------------------------------------------------------------------------

// a = tan(  pi/8) = tan(22.5)
// b = tan(3*pi/8) = tan(67.5)

// nnne
kip_dry_function(dry_nnne) {
   /*static*/ const real a = op::tan(pi<real>/8);  // about 0.414
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h-vars.heps-1,v-vars.veps+a));
}

// eene
kip_dry_function(dry_eene) {
   /*static*/ const real b = op::tan(3*pi<real>/8);  // about 2.414
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h-vars.heps-1,v-vars.veps+b));
}

// nnnw
kip_dry_function(dry_nnnw) {
   /*static*/ const real a = op::tan(pi<real>/8);  // about 0.414
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h+vars.heps-1,v-vars.veps-a));
}

// wwnw
kip_dry_function(dry_wwnw) {
   /*static*/ const real b = op::tan(3*pi<real>/8);  // about 2.414
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h+vars.heps-1,v-vars.veps-b));
}

// ssse
kip_dry_function(dry_ssse) {
   /*static*/ const real a = op::tan(pi<real>/8);  // about 0.414
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h-vars.heps+1,v+vars.veps+a));
}

// eese
kip_dry_function(dry_eese) {
   /*static*/ const real b = op::tan(3*pi<real>/8);  // about 2.414
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h-vars.heps+1,v+vars.veps+b));
}

// sssw
kip_dry_function(dry_sssw) {
   /*static*/ const real a = op::tan(pi<real>/8);  // about 0.414
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h+vars.heps+1,v+vars.veps-a));
}

// wwsw
kip_dry_function(dry_wwsw) {
   /*static*/ const real b = op::tan(3*pi<real>/8);  // about 2.414
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h+vars.heps+1,v+vars.veps-b));
}



// -----------------------------------------------------------------------------
// ne75
// ne15
// nw75
// nw15
// se75
// se15
// sw75
// sw15
// -----------------------------------------------------------------------------

// a = tan(  pi/12) = tan(15)
// b = tan(5*pi/12) = tan(75)

// ne75
kip_dry_function(dry_ne75) {
   /*static*/ const real a = op::tan(pi<real>/12);
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h-vars.heps-1,v-vars.veps+a));
}

// ne15
kip_dry_function(dry_ne15) {
   /*static*/ const real b = op::tan(5*pi<real>/12);
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h-vars.heps-1,v-vars.veps+b));
}

// nw75
kip_dry_function(dry_nw75) {
   /*static*/ const real a = op::tan(pi<real>/12);
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h+vars.heps-1,v-vars.veps-a));
}

// nw15
kip_dry_function(dry_nw15) {
   /*static*/ const real b = op::tan(5*pi<real>/12);
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v-vars.veps),
      vars.t2e.back_0nn(h+vars.heps-1,v-vars.veps-b));
}

// se75
kip_dry_function(dry_se75) {
   /*static*/ const real a = op::tan(pi<real>/12);
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h-vars.heps+1,v+vars.veps+a));
}

// se15
kip_dry_function(dry_se15) {
   /*static*/ const real b = op::tan(5*pi<real>/12);
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h-vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h-vars.heps+1,v+vars.veps+b));
}

// sw75
kip_dry_function(dry_sw75) {
   /*static*/ const real a = op::tan(pi<real>/12);
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h+vars.heps+1,v+vars.veps-a));
}

// sw15
kip_dry_function(dry_sw15) {
   /*static*/ const real b = op::tan(5*pi<real>/12);
   return rotate<-3,real>(vars.eyeball,
      vars.t2e.back_0nn(h+vars.heps,  v+vars.veps),
      vars.t2e.back_0nn(h+vars.heps+1,v+vars.veps-b));
}

#undef kip_dry_function
