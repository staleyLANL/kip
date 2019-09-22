
#pragma once

// This file provides the "scene" class, which derives from the other
// major classes: model, view, light, engine, image.

// forward: scene, trace(scene), trace(scene,array)
template<class real, class base, class color>
class scene;

template<class real, class base, class color>
bool trace(scene<real,base,color> &);

template<class real, class base, class color, class pix>
bool trace(scene<real,base,color> &, array<2,pix> &);



// -----------------------------------------------------------------------------
// scene
// -----------------------------------------------------------------------------

template<
   class real  = default_real,
   class base  = default_base,
   class color = default_color
>
class scene :
   public model <real,base>,
   public view  <real>,
   public light <real>,
   public engine<real>,
   public image <real,color>
{
   // For now, disallow copy construction/assignment by making them private
   inline scene(const scene &) { }
   inline scene &operator=(const scene &) { return *this; }

public:
   inline explicit scene() { }

   // direct access to constituents
   inline kip::model <real,base>  &model () { return *this; }
   inline kip::view  <real>       &view  () { return *this; }
   inline kip::light <real>       &light () { return *this; }
   inline kip::engine<real>       &engine() { return *this; }
   inline kip::image <real,color> &image () { return *this; }

   // trace()
   inline bool trace() { return kip::trace(*this); }

   // trace(array<2,pix>)
   template<class pix>
   inline bool trace(array<2,pix> &pixel) { return kip::trace(*this,pixel); }
};
