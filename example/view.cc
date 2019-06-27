
#include "kip.h"

int main()
{
   kip::view<> view;

   // target (the viewer is looking towards this coordinate);
   // and viewer location and orientation
   view.target(0,0,0);
   view.theta = 60;  // angle in xy plane, +x toward -y
   view.phi   = 30;  // angle up(+) or down(-) from the xy plane
   view.roll  =  0;  // right(+) or left(-) roll
   view.d     =  8;  // viewer is this far from the target

   // Camera "field-of-view" in degrees; use 0 < fov < 180
   view.fov = 16;  // default
   view.fov = 10;  // example: somewhat telephoto
   view.fov = 80;  // example: somewhat wide-angle
}
