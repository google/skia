`SkImage::subset` now takes a `GrDirectContext*` as its first parameter (this can be `nullptr` for
non-gpu backed images. Images which are backed by a codec or picture will not be turned into a GPU
texture before being read. This should only impact picture-backed images, which may not be read
correctly if the picture contain nested texture-backed images itself. To force a conversion to
a texture, clients should call `SkImages::TextureFromImage`, passing in the image, and then call
subset on the result. Documentation has been clarified that `SkImage::subset` will return a raster-
backed image if the source is not backed by a texture, and texture-otherwise.

`SkImages::SubsetTextureFrom` has been added to subset an image and explicitly return a texture-
backed image. This allows some optimizations, especially for large images that exceed a maximum
texture size of a GPU.

`SkImage::makeRasterImage` and `SkImage::makeNonTextureImage` now take a `GrDirectContext*` which
clients should supply for reading-back pixels from texture-backed images.