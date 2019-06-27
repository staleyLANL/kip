
#include "kip.h"

int main()
{
   kip::engine<> engine;

   // Ray-tracing "engine's" internal parameters. We're showing
   // engine<>'s contents only because we've shown those of other
   // major data types. ***DON'T MESS WITH THESE*** unless you've
   // talked with me, and really know what you're doing.
   (void)engine.method;
   (void)engine.hzone;     (void)engine.hsub;
   (void)engine.vzone;     (void)engine.vsub;
   (void)engine.hdivision; (void)engine.sort_frac;
   (void)engine.vdivision; (void)engine.sort_min;
   (void)engine.xzone;     (void)engine.min_area;
   (void)engine.yzone;     (void)engine.fudge;
   (void)engine.zzone;     (void)engine.lean;
}
