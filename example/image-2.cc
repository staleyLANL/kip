
#include "kip.h"

int main()
{
   kip::image<> image;

   // Set the image size: horizontal*vertical pixels. This
   // reallocates, and thus invalidates any existing data.
   // Call trace() again, afterwards, to get a new image!
   image.resize(1920,1080);

   // Query size. [hv]pixel are const size_t & (read-only!)
   assert(image.hpixel == 1920);
   assert(image.vpixel == 1080);

   // Like resize(), but reallocates image memory only if the new
   // dimensions require more total space than is already allocated.
   image.upsize(1200,1024);  // <== resizes, but no reallocation
}
