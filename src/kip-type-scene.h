
// forward: scene, trace(scene), trace(scene,array)
template<class real, class base, class color>
class scene;

template<class real, class base, class color>
bool trace(scene<real,base,color> &);

template<class real, class base, class color, class pix>
bool trace(scene<real,base,color> &, array<2,pix> &);



// -----------------------------------------------------------------------------
// scene
// Derives from the other major classes: model, view, light, engine, image
// -----------------------------------------------------------------------------

template<
   class real  = defaults::real,
   class base  = defaults::base,
   class color = defaults::color
>
class scene :
   public model <real,base>,
   public view  <real>,
   public light <real>,
   public engine<real>,
   public image <real,color>
{
   // For now, disallow copy construction/assignment by making them private
   scene(const scene &) { }
   scene &operator=(const scene &) { return *this; }

public:
   explicit scene() { }

   // direct access to constituents
   kip::model <real,base>  &model () { return *this; }
   kip::view  <real>       &view  () { return *this; }
   kip::light <real>       &light () { return *this; }
   kip::engine<real>       &engine() { return *this; }
   kip::image <real,color> &image () { return *this; }

   // trace()
   bool trace() { return kip::trace(*this); }

   // trace(array<2,pix>)
   template<class pix>
   bool trace(array<2,pix> &pixel) { return kip::trace(*this,pixel); }
};
