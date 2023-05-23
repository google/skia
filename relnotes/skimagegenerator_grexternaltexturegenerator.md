`GrTextureGenerator` now has a subclass `GrExternalTextureGenerator` which can be subclassed by
clients and used with `SkImages::DeferredFromTextureGenerator` in order to create images from
textures that were created outside of skia. `GrTextureGenerator` has been removed from the public
API in favor of `GrExternalTextureGenerator`.
