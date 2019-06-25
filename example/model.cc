
#include "kip.h"

int main()
{
   kip::model<> model;

   // Create some objects. Note: model.append = true isn't needed;
   // it relates to stream input, as shown in another example
   model.push(kip::sphere<>(0,0,0, 1.5, kip::rgb(255,0,0)));
   model.push(kip::sphere<>(0,0,0, 2.5, kip::rgb(0,255,0)));
   model.push(kip::cylinder<>(0,0,0, 1,0,0, 0.5, kip::rgb(0,0,255)));

   assert(model.sphere.size() == 2);  // Number of spheres
   assert(model.sphere[0].r == 1.5);
   assert(model.sphere[1].r == 2.5);
   assert(model.size() == 3);  // Total number of objects
   model.clear();
   assert(model.size() == 0);
}
