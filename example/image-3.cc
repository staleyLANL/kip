
#include "kip.h"

int main()
{
   kip::image<> image;
   const std::size_t nx = 1920;
   const std::size_t ny = 1080;
   image.resize(nx,ny);

   // Access particular pixels. (We'll ignore the actual values
   // here; they're indeterminate before a rendering is made.)
   (void)image(0,   0   );  // lower left
   (void)image(nx-1,0   );  // lower right
   (void)image(0,   ny-1);  // upper left
   (void)image(nx-1,ny-1);  // upper right

   // As we've already seen, operator() gives a kip::rgba<> *.
   assert(image() == &image(0,0));
}
