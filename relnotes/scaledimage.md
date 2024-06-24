
SkImage now has a method makeScaled(...) which returns a scaled version of
the image, retaining its original "domain"
- raster stays raster
- ganesh stays ganesh
- graphite stays graphite
- lazy images become raster (just like with makeSubset)
