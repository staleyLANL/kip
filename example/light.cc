
#include "kip.h"

int main()
{
   kip::light<> light;
   // Little is available, at the present time, for lighting. We were
   // eventually going to allow for several light sources, and for
   // various light-source properties. For now, there's an "array" of
   // one simple light; hence the [0] below.

   // Light-source [0]'s (x,y,z) coordinate
   light[0](10.0, 15.0, 20.0);

   // If we have a view (or a scene, which is derived from a view),
   // this 4-argument construction actually places the light relative
   // to the view. In a UI, you could use something like this as the
   // view changes, to make the light's position change similarly.
   // light[0](view.target, view.d, view.theta+60, view.phi+10);
}
