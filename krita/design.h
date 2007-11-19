/**
  @mainpage Krita Image manipulation and paint application

  Krita is an advanced and modular paint and image manipulation
  application.

  Krita is built around two core libraries: pigment and kritaimage.

  The pigment library abstracts colorspaces and color
  transformations. ColorSpaces provide functions to manipulate pixels. The
  kritcolor library loads colorspace plugins to extend the range of
  available colorspaces. Pigment is part of the KOffice libraries.

  The kritaimage library abstracts the storage, creation, inspection
  and manipulation of pixels stored in a rectangular area. It provides
  layers, filters, iterators and painters. Filters and paint operations
  are provided as service plugins loaded through the appropriate trader
  queries.

  Both libraries are used by the user interface, which is a KOffice
  part. the user interface loads tools and other plugins.

  There are the following types of plugins

<ul>
  <li> filters
  <li> tools
  <li> paintops
  <li> colorspaces
  <li> file filters
  <li> view plugins
</u>
 
  Of these, file filters exist outside the Krita tree and colorspaces
  depend only on pigment, which is also outside krita itself. Please
  consult the plugin writers' manual for more information.

 */
#ifndef DESIGN
#define DESIGN
// Let's keep icefox.net/kde/tests.headerincluded_koffice.html happy
#endif
