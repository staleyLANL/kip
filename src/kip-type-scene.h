
#pragma once

// This file provides the "scene" class, which derives from the other
// major classes: model, view, light, engine, image.

namespace kip {
   // forward declarations: scene, trace(scene), trace(scene,array)
   template<class real, class tag, class color> class scene;

   template<class real, class tag, class color>
   bool trace(scene<real,tag,color> &);

   template<class real, class tag, class color, class pix>
   bool trace(scene<real,tag,color> &, array<2,pix> &);
}



// -----------------------------------------------------------------------------
// scene
// -----------------------------------------------------------------------------

namespace kip {

template<
   class real  = default_real_t,
   class tag   = default_tag_t,
   class color = default_color_t
>
class scene :
   public model <real,tag>,
   public view  <real>,
   public light <real>,
   public engine<real>,
   public image <real,color>
{
   // For now, disallow copy construction/assignment by making them private
   inline scene(const scene &) { }
   inline scene &operator=(const scene &) { return *this; }

public:
   // scene()
   inline explicit scene(void) { }

   // direct access to constituents
   inline kip::model <real,tag>   &model (void) { return *this; }
   inline kip::view  <real>       &view  (void) { return *this; }
   inline kip::light <real>       &light (void) { return *this; }
   inline kip::engine<real>       &engine(void) { return *this; }
   inline kip::image <real,color> &image (void) { return *this; }

   // trace()
   inline bool trace(void) { return kip::trace(*this); }

   // trace(array<2,pix>)
   template<class pix>
   inline bool trace(array<2,pix> &pixel) { return kip::trace(*this,pixel); }
};

}
