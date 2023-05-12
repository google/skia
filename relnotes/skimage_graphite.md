The following SkImage factories have been moved to `include/gpu/graphite/Image.h`:
 - `SkImage::MakeGraphiteFromBackendTexture -> SkImages::AdoptTextureFrom`
 - `SkImage::MakeGraphiteFromYUVABackendTextures -> SkImages::TextureFromYUVATextures`
 - `SkImage::MakeGraphiteFromYUVAPixmaps -> SkImages::TextureFromYUVAPixmaps`
 - `SkImage::MakeGraphitePromiseTexture -> SkImages::PromiseTextureFrom`

The SkImage method `makeTextureImage` has been moved to `SkImages::TextureFromImage`.

`SkImage::RequiredImageProperties` has been renamed to `SkImage::RequiredProperties`,
with fMipmapped turned into a boolean instead of the GPU enum.