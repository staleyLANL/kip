
#include "kip.h"

int main(const int argc, const char *const *const argv)
{
   // want program + one argument
   assert(argc == 2);

   // open file
   std::ifstream infile(argv[1]);
   assert(infile);

   // read scene
   kip::scene<> scene;
   infile >> scene;

   // ray trace
   kip::trace(scene);

   // a pointer to scene.hpixel*scene.vpixel red-green-blue-alpha
   // values is now in scene()
}
