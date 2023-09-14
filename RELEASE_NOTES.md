Skia Graphics Release Notes

This file includes a list of high level updates for each milestone release.

Milestone 118
-------------
  * `GrDirectContext::flush` variants now expect a SkSurface pointer only, not
    an sk_sp<SkSurface>.
  * `SkImage::makeWithFilter` has been deprecated. It has been replaced with three factory functions:

    Ganesh:   `SkImages::MakeWithFilter(GrRecordingContext*, ...);`         -- declared in SkImageGanesh.h

    Graphite: `SkImages::MakeWithFilter(skgpu::graphite::Recorder*, ...);`  -- declared in Image.h

    Raster:   `SkImages::MakeWithFilter(...);`                              -- declared in SkImage.h

    The new factories require the associated backend context object be valid. For example, the Graphite version will return nullptr if it isn't supplied with a `Recorder` object.
  * SkSL and Runtime Effects are no longer optional features of Skia; they are always available.
    The GN flag `skia_enable_sksl` has been removed.
  * SkSL will now properly reject sequence-expressions containing arrays, or sequence-expressions
    containing structures of arrays. Previously, the left-side expression of a sequence was checked,
    but the right-side was not. In GLSL ES 1.0, and therefore in SkSL, the only operator which is
    allowed to operate on arrays is the array subscript operator (`[]`).
  * The Dawn backend for Ganesh has been removed. Dawn will continue to be supported in the
    Graphite backend.
  * We plan to remove SkTime.h from the public API. As of now, SkAutoTime has been
    deleted as it was unused.
  * Vulkan-specific calls are being removed from GrBackendSurface.h. Clients should use the
    equivalents found in `include/gpu/ganesh/vk/GrVkBackendSurface.h"`

* * *

Milestone 117
-------------
  * `SkGraphics::AllowJIT()` has been removed. It was previously deprecated (and did nothing).
  * New methods are added to `SkImage`, `SkSurface`, and `skgpu::graphite::context` named
    `asyncRescaleAndReadPixeksYUVA420`. These function identically to the existing
    `asyncRescaleAndReadPixelsYUV420` methods but return a fourth plane containing alpha at full
    resolution.
  * `SkAutoGraphics` was removed. This was a helper struct that simply called `SkGraphics::Init`.
    Any instance of `SkAutoGraphics` can be replaced with a call to `SkGraphics::Init`.
  * `SkCanvas::flush()` has been removed. It can be replaced with:
    ```
        if (auto dContext = GrAsDirectContext(canvas->recordingContext())) {
            dContext->flushAndSubmit();
        }
    ```

    `SkCanvas::recordingContext()` and `SkCanvas::recorder()` are now const. They were implicitly const
    but are now declared to be such.
  * `SkCanvas::recordingContext()` and `SkCanvas::recorder()` are now const.
    They were implicitly const but are now declared to be such.
  * `SkMesh::MakeIndexBuffer`, `SkMesh::CopyIndexBuffer`, `SkMesh::MakeVertexBuffer`, and
    `SkMesh::CopyVertexBuffer` have been moved to the `SkMeshes` namespace. Ganesh-specific versions
    have been created in `include/gpu/ganesh/SkMeshGanesh.h`.
  * SkPath now enforces an upper limit of 715 million path verbs.
  * `SkRuntimeEffectBuilder::uniforms()`, `SkRuntimeEffectBuilder::children()`,
    `SkRuntimeShaderBuilder::makeShader()`, `SkRuntimeColorFilterBuilder::makeColorFilter()`, and
    `SkRuntimeBlendBuilder::makeBlender()` are now marked as const. No functional changes internally,
    just making explicit what had been implicit.
  * `SkRuntimeEffect::makeImage` and `SkRuntimeShaderBuilder::makeImage` have been removed.
  * GL-specific calls have been removed from GrBackendSurface.h. Clients should use the
    equivalents found in `include/gpu/ganesh/gl/GrGLBackendSurface.h`
  * A new `SkTiledImageUtils` namespace (in `SkTiledImageUtils.h`) provides `DrawImage` and `DrawImageRect` methods that directly mirror `SkCanvas'` `drawImage` and `drawImageRect` calls.

    The new entry points will breakup large `SkBitmap`-backed `SkImages` into tiles and draw them if they would be too large to upload to the gpu as one texture.

    They will fall through to their `SkCanvas` correlates if tiling isn't needed or possible.

* * *

Milestone 116
-------------
  * `SkPromiseImageTexture` has been removed from the public API, as well as
    `SkImages::PromiseTextureFrom` and `SkImages::PromiseTextureFromYUVA`, public consumers of that
    data type.
  * `SkDeferredDisplayList`, `SkDeferredDisplayListRecorder`, and `SkSurfaceCharacterization` have
    been removed from the public API.
  * The intermediate color computed by `SkBlenders::Arithmetic` is now always clamped to between 0 and 1 (inclusive), and then `enforcePremul` is applied when that parameter is true.
  * Added a new public type, `SkColorTable`, to own the lookup tables passed into `SkColorFilters::Table`, which allows clients and the returned `SkColorFilter` to share the table memory instead of having to duplicate it in any wrapper types that lazily create Skia representations.
  * The deprecated `SkImageFilters::Magnifier` factory that did *not* take a lens bounds parameter has been removed.
  * `SkImageFilters::RuntimeShader` has variations that take a maximum sample radius, which is used to provide padded input images to the runtime effect so that boundary conditions are avoided.
  * `SkImageFilters::AlphaThreshold` has been removed. Its only use was in ChromeOS and that usage has been replaced with a `Blend(kSrcIn, input, Picture(region))` filter graph to achieve the same effect.
  * The single-argument `SkImageFilters::Image(sk_sp<SkImage>)` factory is removed. The `SkSamplingOptions` to use when rendering the image during filtering must be provided. `SkFilterMode::kLinear` is recommended over the previous bicubic default.
  * `GrTextureGenerator` now has a subclass `GrExternalTextureGenerator` which can be subclassed by
    clients and used with `SkImages::DeferredFromTextureGenerator` in order to create images from
    textures that were created outside of skia. `GrTextureGenerator` has been removed from the public
    API in favor of `GrExternalTextureGenerator`.
  * SkPoint now uses float for its coordinates. This starts the process of removing SkScalar from Skia.
    SkScalar was a typedef for float, so this has no practical impact on code that uses Skia.
  * `SkSamplingOptions(SkFilterMode)` and `SkSamplingOptions(SkCubicResampler)` are no longer marked `explicit` so that samplings can be created inline more succinctly.
  * `SkShaders` is now a namespace (was previously a non-constructable class with only static
    functions). `SkPerlinNoiseShader::MakeFractalNoise` and `SkPerlinNoiseShader::MakeTurbulence` have
    been moved to the `SkShaders` namespace and `SkPerlinNoiseShader` (the public non-constructable
    class) has been slated for moving into private internals of Skia.
    There are no functional differences in the moved functions, however the change of some #includes
    in `include/core/SkShader.h`, `include/effects/SkGradientShader.h`, and
    `include/effects/SkPerlinNoiseShader.h` may cause clients who were depending on the transitive
    dependencies to now fail to compile.
  * The following methods have been removed from SkSurface and relocated to other methods/functions:
      - `SkSurface::asImage` -> `SkSurfaces::AsImage` (include/gpu/graphite/Surface.h)
      - `SkSurface::flushAndSubmit` -> `GrDirectContext::flushAndSubmit`
      - `SkSurface::flush` -> `GrDirectContext::flush`
      - `SkSurface::makeImageCopy` -> `SkSurfaces::AsImageCopy` (include/gpu/graphite/Surface.h)
      - `SkSurface::resolveMSAA` -> `SkSurfaces::ResolveMSAA()` (include/gpu/ganesh/SkSurfaceGanesh.h)

    Additionally, `SkSurface::BackendSurfaceAccess` is now in the `SkSurfaces` namespace.
  * The deprecated `SkTableColorFilter` class and its methods have been removed. Clients should use
    `SkColorFilters::Table` and `SkColorFilters::TableARGB` (defined in include/core/SkColorFilter.h).
  * The `SkYUVAPixmapInfo::SupportedDataTypes(const GrImageContext&)` constructor has been removed from
    the public API.

* * *

Milestone 115
-------------
  * Clients now need to register codecs which Skia should use to decode raw bytes. For example:
    `SkCodecs::Register(SkJpegDecoder::Decoder());`. Skia still provides many supported formats
    (see `include/codec/*Decoder.h`). Clients are free to specify their own, either supplementing
    the existing set or using a custom version instead of the one previously provided by default
    by Skia. See `SkCodecs::Decoder` for the necessary data to provide when using a custom decoder
    (in `include/codec/SkCodec.h`).

    To ease the transition, Skia will continue (for a short while) to register codecs unless
    `SK_DISABLE_LEGACY_INIT_DECODERS` is defined.
  * `SkDrawable::newPictureSnapshot` is removed. Instead, call `SkDrawable::makePictureSnapshot`.
    The old method returned a bare (but ref-counted) pointer, which was easy for clients to get wrong.
    The new method returns an `sk_sp<SkPicture>`, which is easier to handle, and consistent with the
    rest of skia.
  * `SkGraphics::PurgePinnedFontCache()` has been added to allow clients to
    explicitly trigger `SkStrikeCache` purge checks for `SkStrikes` with
    pinners. Defining `SK_STRIKE_CACHE_DOESNT_AUTO_CHECK_PINNERS` in the
    user configuration now disables automatic purge checking of strikes with
    pinners.
  * The following SkImage factories have been moved to `include/gpu/graphite/Image.h`:
     - `SkImage::MakeGraphiteFromBackendTexture -> SkImages::AdoptTextureFrom`
     - `SkImage::MakeGraphiteFromYUVABackendTextures -> SkImages::TextureFromYUVATextures`
     - `SkImage::MakeGraphiteFromYUVAPixmaps -> SkImages::TextureFromYUVAPixmaps`
     - `SkImage::MakeGraphitePromiseTexture -> SkImages::PromiseTextureFrom`

    The SkImage method `makeTextureImage` has been moved to `SkImages::TextureFromImage`.

    `SkImage::RequiredImageProperties` has been renamed to `SkImage::RequiredProperties`,
    with fMipmapped turned into a boolean instead of the GPU enum.
  * `SkImage::makeColorSpace` and `SkImage::makeColorTypeAndColorSpace` now take a `GrDirectContext`
    as the first parameter. This should be supplied when dealing with texture-backed images and can
    be `nullptr` otherwise.
  * `SkImage::subset` now takes a `GrDirectContext*` as its first parameter (this can be `nullptr` for
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
  * `SkImageFilters::Image` now returns a non-null image filter if the input `sk_sp<SkImage>` is
    null or the src rectangle is empty or does not overlap the image. The returned filter evaluates to
    transparent black, which is equivalent to a null or empty image. Previously, returning a null image
    filter would mean that the dynamic source image could be surprisingly injected into the filter
    evaluation where it might not have been intended.
  * `SkImageFilters::Magnifier(srcRect, inset)` is deprecated. These parameters do not provide enough
    information for the implementation to correctly respond to canvas transform or participate accurately
    in layer bounds planning.

    A new `SkImageFilters::Magnifier` function is added that takes additional parameters: the outer
    lens bounds and the actual zoom amount (instead of inconsistently reconstructing the target zoom
    amount, which was the prior behavior). Additionally, the new factory accepts an SkSamplingOptions
    to control the sampling quality.
  * `SkImageFilters::Picture` now returns a non-null image filter if the input `sk_sp<SkPicture>` is
    null. The returned filter evaluates to transparent black, which is equivalent to a null or empty
    picture. Previously, returning a null image filter would mean that the dynamic source image could
    be surprisingly injected into the filter evaluation where it might not have been intended.
  * `SkImageFilters::Shader` now returns a non-null image filter if the input `sk_sp<SkShader>` is
    null. The returned filter evaluates to transparent black, which is equivalent to a null or empty
    shader. Previously, returning a null image filter would mean that the dynamic source image could
    be surprisingly injected into the filter evaluation where it might not have been intended.
  * `SkImageGenerator::MakeFromEncoded` has been removed from the public API.
    `SkImage::DeferredFromEncoded` or `SkCodec::MakeFromData` should be used instead.
  * `SkSurface::getBackendTexture` and `SkSurface::getBackendRenderTarget` have been deprecated and
    replaced with `SkSurfaces::GetBackendTexture` and `SkSurfaces::GetBackendRenderTarget` respectively.
    These are found in `include/gpu/ganesh/SkSurfaceGanesh.h`. The supporting enum `BackendHandleAccess`
    has also been moved to `SkSurfaces::BackendHandleAccess` as an enum class, with shorter member
    names.
  * SkSurface factory methods have been moved to the SkSurfaces namespace. Many have been renamed to
    be more succinct or self-consistent. Factory methods specific to the Ganesh GPU backend are
    defined publicly in include/gpu/ganesh/SkSurfaceGanesh.h. The Metal Ganesh backend has some
    specific factories in include/gpu/ganesh/mtl/SkSurfaceMetal.h.
      * SkSurface::MakeFromAHardwareBuffer -> SkSurfaces::WrapAndroidHardwareBuffer
      * SkSurface::MakeFromBackendRenderTarget -> SkSurfaces::WrapBackendRenderTarget
      * SkSurface::MakeFromBackendTexture -> SkSurfaces::WrapBackendTexture
      * SkSurface::MakeFromCAMetalLayer -> SkSurfaces::WrapCAMetalLayer
      * SkSurface::MakeFromMTKView -> SkSurfaces::WrapMTKView
      * SkSurface::MakeGraphite -> SkSurfaces::RenderTarget
      * SkSurface::MakeGraphiteFromBackendTexture -> SkSurfaces::WrapBackendTexture
      * SkSurface::MakeNull -> SkSurfaces::Null
      * SkSurface::MakeRaster -> SkSurfaces::Raster
      * SkSurface::MakeRasterDirect -> SkSurfaces::WrapPixels
      * SkSurface::MakeRasterDirectReleaseProc -> SkSurfaces::WrapPixels
      * SkSurface::MakeRasterN32Premul -> SkSurfaces::Raster (clients should make SkImageInfo)
      * SkSurface::MakeRenderTarget -> SkSurfaces::RenderTarget

* * *

Milestone 114
-------------
  * The CPU backend for Runtime Effects has been rewritten. This may cause slight differences in
    performance and image quality when runtime effects are painted onto a raster surface.
  * Gradient shaders support interpolation in several different color spaces, by passing a
    `SkGradientShader::Interpolation` struct to the shader factory functions. The color space and
    hue method options are based on the CSS Color Level 4 specfication:
    * https://www.w3.org/TR/css-color-4/#interpolation-space
    * https://www.w3.org/TR/css-color-4/#hue-interpolation
  * `SkImages::GetBackendTextureFromImage` has been renamed `SkImages::MakeBackendTextureFromImage`.
  * `SkImage::getBackendTexture()` has been moved to `SkImages::GetBackendTextureFromImage()` in
    `SkImageGanesh.h`.
  * `SkImage::makeTextureImage()` has been moved to `SkImages::TextureFromImage()` in
    `SkImageGanesh.h`.
  * `SkImage::flush()` and `SkImage::flushAndSubmit()` has been moved to
    `GrDirectContext::flush()` and `GrDirectContext::flushAndSubmit()` in `SkImageGanesh.h`.
  * `SkSurfaceProperties::kAlwaysDither_Flag` added to globally enable dithering for a specific
    `SkSurface` target.
  * `SkSerialImageProc` and `SkDeserialImageProc` are now also used to encode/decode the SkMipmap
    layers of certain SkImages.
  * The defines `SK_USE_WIC_ENCODER` and `SK_USE_CG_ENCODER` have been removed, as well as the code
    to use the Windows Image Codecs and Core Graphics as a way to have Skia encode files in PNG,
    JPEG, and WEBP format. Skia continues to support use of the NDK codecs on Android, as well
    as using external C++ libraries (e.g. libpng, libjpeg-turbo) to *encode* images. WIC and CG
    are still used to *decode* images on the appropriate platforms.
  * `SkImage::encodeToData` has been deprecated. Clients should use `refEncodedData` if the image
    was from an encoded bytestream or one of `SkPngEncoder::Encode`, `SkJpegEncoder::Encode`,
    `SkWebpEncoder::Encode` directly.
  * The following defines no longer do anything. GN clients should instead set the provided
    arguments (from gn/skia.gni) as necessary:
      - `SK_ENCODE_PNG` -> `skia_use_libjpeg_turbo_encode`
      - `SK_ENCODE_JPEG` -> `skia_use_libpng_encode`
      - `SK_ENCODE_WEBP` -> `skia_use_libwebp_encode`
    Other clients should make sure the appropriate `*EncoderImpl.cpp` files from `src/encode` are
    included in the build.
  * `SkImageEncoder` has been removed. Clients should use one of `SkPngEncoder::Encode`,
    `SkJpegEncoder::Encode` or `SkWebpEncoder::Encode` directly.
  * `SkImageGenerator` has a new subclass `GrTextureGenerator` which can be used if clients want to
    provide specialized ways of making Ganesh texture-backed Images.
  * `SkImageGenerator::MakeFromPicture` has been removed from the public API. Clients should be
    drawing the picture directly instead of turning it into an image first.


* * *

Milestone 113
-------------
  * The define SK_SUPPORT_GPU is now SK_GANESH. It is no longer detected as a 0 or 1, but
    as the absence or presence of that define. As a result, it defaults to off (not defined) if
    not defined (SK_SUPPORT_GPU would default to SK_SUPPORT_GPU=1 if not defined).
  * SkStrSplit is no longer part of the public API.
  * SkImage::encodeToData now takes a GrDirectContext. The versions which do not have that are
    deprecated and will be removed at some point.
  * SkMatrix::Scale, preScale, setScale, etc. with any scale factor of 0 correctly no longer
    return true from rectStaysRect(), consistent with rectStaysRect() implying a non-zero scale.
  * `SkImage::CompressionType` has been renamed to `SkTextureCompressionType` and moved to
    `include/core/SkTextureCompressionType.h`
  * `SkEncodedImageFormat.h` and `SkPngChunkReader.h` are now in include/codec
  * `SkICC.h` is now in include/encode
  * SkImage factory methods have been moved to the SkImages namespace. Many have been renamed to
    be more succinct or self-consistent. Factory methods specific to the Ganesh GPU backend are
    defined publicly in include/gpu/ganesh/SkImageGanesh.h.
      * SkImage::MakeBackendTextureFromSkImage -> SkImages::GetBackendTextureFromImage
      * SkImage::MakeCrossContextFromPixmap -> SkImages::CrossContextTextureFromPixmap
      * SkImage::MakeFromAdoptedTexture -> SkImages::AdoptTextureFrom
      * SkImage::MakeFromBitmap -> SkImages::RasterFromBitmap
      * SkImage::MakeFromCompressedTexture -> SkImages::TextureFromCompressedTexture
      * SkImage::MakeFromEncoded -> SkImages::DeferredFromEncodedData
      * SkImage::MakeFromGenerator -> SkImages::DeferredFromGenerator
      * SkImage::MakeFromPicture -> SkImages::DeferredFromPicture
      * SkImage::MakeFromRaster -> SkImages::RasterFromPixmap
      * SkImage::MakeFromTexture -> SkImages::BorrowTextureFrom
      * SkImage::MakeFromYUVAPixmaps -> SkImages::TextureFromYUVAPixmaps
      * SkImage::MakeFromYUVATextures -> SkImages::TextureFromYUVATextures
      * SkImage::MakePromiseTexture -> SkImages::PromiseTextureFrom
      * SkImage::MakePromiseYUVATexture -> SkImages::PromiseTextureFromYUVA
      * SkImage::MakeRasterCopy -> SkImages::RasterFromPixmapCopy
      * SkImage::MakeRasterData -> SkImages::RasterFromData
      * SkImage::MakeRasterFromCompressed -> SkImages::RasterFromCompressedTextureData
      * SkImage::MakeTextureFromCompressed -> SkImages::TextureFromCompressedTextureData
    To help in the transition, there is some temporary bridge code (e.g. aliases) which will
    eventually be removed.

* * *

Milestone 112
-------------
  * SkImage::CubicResampler has been removed. Clients should use SkCubicResampler from
    include/core/SkSamplingOptions.h instead (the former was an alias for the latter).
  * SkRuntimeColorFilterBuilder has been added. This is a helper class for setting up color filters,
    analogous to SkRuntimeShaderBuilder.
  * SkShaders::CoordClamp has been added. It clamps the coords passed used with another
    shader to a rectangle.
  * SkRandom is no longer part of the public API.
  * SK_ARRAY_COUNT is no longer part of the public API. Clients should use std::size.
  * SK_SCALAR_IS_FLOAT is not set anymore. SkScalar is always a float (and has been since 2017).
  * sk_realloc_throw (an internal API) now frees up memory when 0 is passed in as the size.
    This should have no user-facing impacts for clients which use the default allocator, but
    requires custom allocators to also implement this change.
  * The particles module has been deleted.
  * SkJpegEncoder::Options includes a parameter for XMP metadata.
  * SkJpegEncoder includes support for encoding SkYUVAPixmaps directly.

* * *

Milestone 111
-------------
  * SkToBool is no longer part of the public API.
  * A float version of SkCanvas::saveLayerAlpha now exists as SkCanvas::saveLayerAlphaf.
  * SkAbs32 and SkTAbs are no longer part of the public API.
  * SkAlign2, SkAlign4, SkAlign8, SkIsAlign2, SkIsAlign4, SkIsAlign8, SkAlignPtr, SkIsAlignPtr,
    and SkAlignTo are no longer part of the public API.
  * GrContextOptions::fSkipGLErrorChecks no longer stops checking shader compilation and program
    linking success.
  * SkBackingFit is no longer part of the public API.
  * SkBudgeted was moved from include/core/SkTypes.h to include/gpu/GpuTypes.h and moved into the
    skgpu namespace.
  * include/gpu/GrConfig.h has been removed; its contents were folded into other files.
  * SkLeftShift is no longer part of the public API.
  * SK_MaxS32 and related constants are no longer part of the public API.
  * include/core/SkMath.h is no longer part of the public API.

* * *

Milestone 110
-------------
  * SkParsePath::ToSVGString now returns the string, rather than modifying a passed-in string.
  * Removed previously deprecated SkImageFilters::Paint factory. Use SkImageFilters::Shader instead.
  * SkMesh::Make and SkMesh::MakeIndexed now return a SkMesh and error message string.
  * SkPaint::getFillPath has been replaced with skpathutils::FillPathWithPaint from
    include/core/SkPathUtils.h. The functionality should be the same.

* * *

Milestone 109
-------------
  * SkMesh vertex and fragment main() signatures have changed. See docs on SkMeshSpecification.
  * Added SkImage::RescaleMode::kLinear so that the async rescale/readback APIs can scale in a
    single step no matter the total scale factor (faster but lower quality than kRepeatedLinear).
  * SkMesh buffer factories added that make copies of CPU-backed buffers.
  * A utility for minifying Runtime Effect code has been added to Skia. Add the gn argument
    "skia_compile_modules = true" to your gn args, and a new utility called "sksl-minify" will be
    compiled as part of your Skia build. Run the command:
      `skia-minify output-file.sksl input-file.sksl`
    to write a minified version of the runtime shader "input-file.sksl" into a file named
    "output-file.sksl". By default, sksl-minify expects a shader, but you can also pass command
    line options `--colorfilter` or `--blender` if your program is a color-filter or a blender.
    A compile error will be printed to stdout if an error is found in the program.
  * The order of SkShader local matrix concatenation has been reversed. See skbug.com/13749
  * PromiseImages have been added to Graphite. This supports both volatile and non-volatile Promise Images.
    See the comment for SkImage::MakeGraphitePromiseTexture for more details.
  * Graphite has loosened the immutability requirements of SkImages - through a new SkSurface API and careful
    synchronization, clients can now mutate the backend object backing an SkImage. The new API consists of
    SkSurface::asImage and SkSurface::makeImageCopy. We have a document that covers the expected use cases and
    the synchronization required for each one.

* * *

Milestone 108
-------------
  * SkShader::asAGradient() has been removed.
  * SkMesh and SkMeshSpecification has separate sk_sp and bare ptr getters for ref counted types.
  * Add support for specifying a custom ICC profile to SkJpegEncoder, SkPngEncoder, and
    SkWebpEncoder.

* * *

Milestone 107
-------------
  * Exported SkColor4f::toBytes_RGBA() and SkColor4f::FromBytes_RGBA.
  * SkWebpEncoder: Added support for animated WebP image encoding.
  * SkRuntimeEffect shader effects were inadvertently allowing functions with the signature
    `half4 main(float2 coords, half4 color)`. This was disallowed at Milestone 87, but the
    restriction was inadvertently relaxed in later milestones. Going forward, we will only
    accept a shader signature of `half4 main(float2 coords)`.

* * *

Milestone 106
-------------
  * sk_sp is marked with the [[clang::trivial_abi]] attribute where supported.
  * SkMesh API: Allows a user to draw a vertex mesh with custom attributes and
    varyings using SkSL. Mesh data (vertex and index) can be created on a
    GrDirectContext to avoid re-uploading data per draw. Currently does not
    work with SkPicture or any backend but GPU.
  * Added SkColorFilters::Blend(const SkColor4f&, sk_sp<SkColorSpace>, SkBlendMode) to
    complement the existing SkColorFilters::Blend(SkColor, SkBlendMode) factory.
  * The experimental C API was removed.
  * Added support for AVIF decoding using libavif.

* * *

Milestone 104
-------------
  * New functions SkBitmap::getColor4f and SkPixmap::getColor4f return float colors.
  * SkRuntimeEffect takes and returns a const SkData.
  * SkRasterHandleAllocator::MakeCanvas now takes optional SkSurfaceProps.
  * SkImage::MakeFromPicture and SkImageGenerator::MakeFromPicture now take an optional
    SkSurfaceProps to use when rasterizing the picture.
  * SkRuntimeEffect::Uniform now stores the uniform name as a string_view, rather than a
    SkString. Related methods SkRuntimeEffect::findUniform and SkRuntimeEffectBuilder::uniform
    also take std::string_view instead of const char*.
  * SkRuntimeEffect::Child now stores the child name as a string_view, rather than a SkString.
    Related methods SkRuntimeEffect::findChild and SkRuntimeEffectBuilder::child also take
    std::string_view instead of const char*. Also, SkImageFilters::RuntimeShader now takes the
    child name(s) as std::string_view instead of const char*.
  * skcms.h has been relocated to //modules/skcms/skcms.h (was //include/third_party/skcms/skcms.h)
  * New functions SkCanvas::getBaseProps and SkCanvas::getTopProps; SkCanvas::getBaseProps is a
    direct replacement for the (now deprecated) SkCanvas::getProps function, while getTopProps is
    a variant that returns the SkSurfaceProps that are active in the current layer.
  * New function SkEventTracer::newTracingSection(const char* name) enables splitting traces up
    into different sections for a selection of backend tracing frameworks (Perfetto, SkDebugf).

* * *

Milestone 103
-------------
  * SkSamplingOptions now includes anisotropic filtering. Implemented on GPU only.
  * SkBitmap::clear and SkBitmap::clearColor take in SkColor4fs

* * *

Milestone 102
-------------
  * Add glGetFloatv and glSamplerParameterf to GrGLInterface.
  * GrGLCreateNativeInterface is removed. Use GrGLMakeNativeInterface.
  * GrContextOptions::fSharpenMipmappedTextures is removed. MIP LOD is now always
    biased on the GPU backend. The CPU backend implementation is modified to match
    this behavior.
  * Passing SkCanvas::kStrict_SrcRectConstraint disables mipmapping. The old behavior differed
    between GPU and CPU. CPU always computed a new set of mipmap based on the subset. GPU restricted
    the sampling coordinates to the subset in the base level but upper level pixels that map to
    pixels outside the subset in the base level were still used. To get the previous CPU behavior
    use SkImage::makeSubset() to make a subset image and draw that. The previous GPU behavior is
    similar, though not exactly, equivalent to making a mipmapped image shader from the original
    image and applying that to a rectangle.
  * Fully disable experimental support for HW tessellation shaders.
    GrContextOptions::fEnableExperimentalHardwareTessellation is ignored and behaves as if it is
    false. The optimized path renderer no longer requires hardware tessellation at all, and
    is automatically enabled when drawing to an SkSurface created with MSAA, or when
    GrContextOptions::fInternalMultisampleCount is set to a non-zero value.

* * *

Milestone 101
-------------
  * Add maxSurfaceSampleCountForColorType(SkColorType ct) in GrContextThreadSafeProxy
  * Enums SkAlphaType and SkColorType are broken out into their own header files in include/core/

* * *

Milestone 100
-------------
  * Skia now requires C++17 and the corresponding standard library (or newer).
  * Skia on iOS now requires iOS 11 to build; earlier versions of iOS do not support C++17.
  * The skstd::string_view and skstd::optional Skia classes have been replaced with the C++17 native
    std::string_view and std::optional.
  * Added SkSurface::resolveMSAA api to force Skia to resolve MSAA draws. Useful for when
    Skia wraps a client's texture as the resolve target.
  * All of the `makeShader` functions associated with `SkRuntimeEffect` no longer take an
    `isOpaque` parameter. These functions will now make a best effort to determine if your
    shader always produces opaque output, and optimize accordingly. If you definitely want your
    shader to produce opaque output, do so in the shader's SkSL code. This can be done by adjusting
    any `return` statement in your shader with a swizzle: `return color.rgb1;`.
    https://review.skia.org/506462
  * SkRSXform is now exported to DLL/.so files.
* * *

Milestone 99
------------
  * Added two new intrinsic functions to SkSL for use in runtime effects:
      vec3 toLinearSrgb(vec3 color)
      vec3 fromLinearSrgb(vec3 color)
    These convert RGB color values between the working color space (the color space of the
    destination surface) and a known, fixed color space. `toLinearSrgb` converts a color to the
    sRGB color gamut, with a linear transfer function. `fromLinearSrgb` converts a color from that
    same color space. These are helpful for effects that need to work in a specific color space, or
    want to apply effects (like lighting) that work best in a linear color space.
    Note that if the destination surface has no color space (color space is `nullptr`), these
    intrinsics will do no conversion, and return the input color unchanged.
    https://review.skia.org/481416
  * Added a new variant of SkImageFilters::RuntimeShader that supports multiple child nodes.
    https://review.skia.org/489536
  * Add the ability to specify palette overrides in SkFontArguments. Implemented
    for the FreeType-backed SkFontMgrs.

* * *

Milestone 98
------------
  * The following functions and methods are not defined in SkSurface when SK_SUPPORT_GPU is 0:
    MakeFromBackendTexture, MakeFromBackendRenderTarget, MakeRenderTarget,
    getBackendTexture, getBackendRenderTarget, replaceBackendTexture. flush() with parameters
    was removed as well. These were all no-ops anyway when just the CPU backend was compiled in
    (noting that flush() and flushAndSubmit() are still no-ops on the CPU backend).
  * GrBackendSemaphore only includes methods that match the GPU backend that Skia was compiled for.
    For example, initVulkan and vkSemaphore are not defined unless the Vulkan backend is compiled
    into Skia.
  * Surfaces and images are now limited to just under 2GB of total size. Previously, larger images
    could be created, but the CPU backend would fail to index them correctly.
  * SkCanvas::drawVertices and SkCanvas::drawPatch variants that did not take SkBlendMode are
    removed.
  * SkImageFilters::RuntimeShader is a new public API that enables adding RuntimeShaderEffects into
    image filter graph.
  * SkImage::makeRawShader is a new public API that creates "raw" image shaders. makeRawShader
    functions like SkImage::makeShader, but for images that contain non-color data. This includes
    images encoding things like normals, material properties (eg roughness), heightmaps, or any
    other purely mathematical data that happens to be stored in an image. These types of images are
    useful with some programmable shaders (ie SkRuntimeEffect).
    Raw image shaders work like regular image shaders (including filtering and tiling), with a few
    major differences:
      - No color space transformation is ever applied (the color space of the image is ignored).
      - Images with an alpha type of kUnpremul are not automatically premultiplied.
      - Bicubic filtering is not supported. If SkSamplingOptions::useCubic is true, these factories
        will return nullptr.
  * Removed SkCanvas::markCTM and SkCanvas::findMarkedCTM. These were created to be used with other
    features that have since been deleted, so they served no purpose.
  * Added limited JPEGXL support.

* * *

Milestone 97
------------
  * Added basic support for vulkan DRM modifiers. All of these are treated as read only textures
    internally (versus querying specific modifier support). Clients can either pass a flag to Vulkan
    GrBackendFormat to say it uses modifiers or pass the VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT
    to a GrBackendTexture via the GrVkImageInfo struct.
  * The following functions and methods are not defined in SkImage when SK_SUPPORT_GPU is 0:
    MakeTextureFromCompressed, MakeFromTexture, MakeFromCompressedTexture,
    MakeCrossContextFromPixmap, MakeFromAdoptedTexture, MakeFromYUVATextures,
    MakeFromYUVAPixmaps, MakePromiseTexture, MakePromiseYUVATexture, MakeBackendTextureFromSkImage,
    flush, flushAndSubmit, getBackendTexture, makeTextureImage.
    These were all no-ops anyway when just the CPU backend was compiled in.

* * *

Milestone 96
------------
  * SkRuntimeEffect no longer clamps the RGB values of an effect's output to the range 0..A.
    This makes it easier to use a hierarchy of SkSL shaders where intermediate values do not
    represent colors but are, for example, non-color inputs to a lighting model.
    http://review.skia.org/452558

* * *

Milestone 95
------------
  * Minimum supported iOS raised from 8 to 11. Skia may build back to iOS 9 but versions older
    than 11 are not tested. Community contributions to support versions 9 and 10 of iOS may be
    considered, but they may not be complex as they cannot be tested.

* * *

Milestone 94
------------
  * Metal backend has been changed to track command buffer resources manually
    rather than using retained resources.
    https://review.skia.org/432878

  * Added virtual onResetClip() to SkCanvas for Android Framework, to emulate the soon-to-be-removed
    expanding clip ops guarded by SK_SUPPORT_DEPRECATED_CLIPOPS.
    https://review.skia.org/430897

  * Removed SK_SUPPORT_DEPRECATED_CLIPOPS build flag. Clips can only be intersect and difference.
    https://review.skia.org/436565

  * There is a new syntax for invoking (sampling) child effects in SkSL. Previously, children
    (shaders, colorFilters, blenders) were invoked using different overloads of `sample`. That
    syntax is deprecated (but still supported). Now, the child behaves like an object, with a method
    name `eval`. The arguments to these `eval` methods are the same as the arguments in the old
    `sample` intrinsics. For example:
      // Old syntax:
        sample(shader, xy)
        sample(colorFilter, color)
        sample(blender, srcColor, dstColor)
      // New syntax:
        shader.eval(xy)
        colorFilter.eval(color)
        blender.eval(srcColor, dstColor)
    https://review.skia.org/444735

* * *

Milestone 93
------------
  * Removed SkPaint::getHash
    https://review.skia.org/419336

  * Removed SkShaders::Lerp. It was unused (and easy to replicate with SkRuntimeEffect).
    https://review.skia.org/419796

  * The default value of GrContextOptions::fReduceOpsTaskSplitting is now enabled.
    https://review.skia.org/419836

  * Removed SkMatrix44

* * *

Milestone 92
------------
  * Hides SkPathEffect::computeFastBounds() from public API; external subclasses of SkPathEffect
    must implement onComputeFastBounds() but can return false to signal it's not computable.
    https://review.skia.org/406140

  * Add SkM44::RectToRect constructor (SkM44's equivalent to SkMatrix::RectToRect)
    https://review.skia.org/402957

  * Metal support has been removed for versions of iOS older than 10.0 and MacOS older than 10.14.
    https://review.skia.org/401816

  * Removed custom attributes from SkVertices and the corresponding `varying` feature from
    SkRuntimeEffect.
    https://review.skia.org/398222

  * Dropped support for mixed samples. Mixed samples is no longer relevant for Ganesh. DMSAA and the
    new Ganesh architecture both rely on full MSAA, and any platform where mixed samples is
    supported will ultimately not use the old architecture.

  * SkRuntimeEffect::Make has been removed. It is replaced by MakeForShader and MakeForColorFilter.
    These functions do stricter error checking on the SkSL, to ensure it is valid for a particular
    stage of the Skia pipeline.
    https://review.skia.org/402156

* * *

Milestone 91
------------
  * The SkSL DSL API has been moved into public headers, although it is still under active
    development and isn't quite ready for prime time yet.
    https://review.skia.org/378496

  * Skia's GPU backend no longer supports NVPR. Our more recent path renderers are more
    performant and are not limited to nVidia hardware.

  * SkRuntimeEffect now supports uniforms of type int, int2, int3, and int4. Per the OpenGL ES
    Shading Language Version 1.00 specification, there are few guarantees about the representation
    or range of integral types, and operations that assume integral representation (eg, bitwise),
    are not supported.
    https://review.skia.org/391856

  * SkRuntimeEffect requires that 'shader' variables be declared as 'uniform'. The deprecated
    syntax of 'in shader' is no longer supported.
    https://review.skia.org/393081

* * *

Milestone 90
------------
  * Renamed use of sk_cf_obj in external Metal types to sk_cfp.
    https://review.skia.org/372556

  * GrDirectContext::ComputeImageSize() is removed. Use SkImage::textureSize() instead.
    https://review.skia.org/368621
    https://review.skia.org/369317
    https://review.skia.org/371958

  * Remove SkImageFilter::MakeMatrixFilter as it was unused and replaced with
    SkImageFilters::MatrixTransform.
    https://review.skia.org/366318

  * Refactored particle system to use a single code string containing both Effect and Particle code.
    Uniform APIs are now shared for all program entry points, and no longer prefixed with 'Effect'
    or 'Particle'. For example, instead of `SkParticleEffect::effectUniformInfo` and
    `SkParticleEffect::particleUniformInfo`, there is just `SkParticleEffect::uniformInfo`.

  * Remove SkImageFilter::CropRect from the public API as it's no longer usable. All factories
    work with 'SkRect', 'SkIRect', or nullable pointers to 'Sk[I]Rect'.
    https://review.skia.org/361496

  * Remove deprecated SkImageFilter factory functions and supporting types. All default-provided
    SkImageFilters are now only constructed via 'include/effects/SkImageFilters.h'
    https://review.skia.org/357285

  * Added SkRuntimeEffect::makeImage() to capture the output of an SkRuntimeEffect in an SkImage.
    https://review.skia.org/357284

  * Updated SkRuntimeEffect::Make() to take an Options struct. It also now returns a Results struct
    instead of a tuple.
    https://review.skia.org/363785
    https://review.skia.org/367060

  * Changed SkRuntimeEffect::Varying to have lower-case member names, with no 'f' prefix.
    https://review.skia.org/365656

  * Changed SkRuntimeEffect::Uniform to have lower-case member names, with no 'f' prefix.
    https://review.skia.org/365696

  * Deprecate (and ignore) SkAndroidCodec::ExifOrientation
    https://review.skia.org/344763

  * Fix several minor issues in lighting image filters:
    - The spotlight falloff exponent is no longer clamped to [1, 128]. SVG 1.1 requires the specular
      lighting effect's exponent (shininess) to be clamped; not the spotlight's falloff. Any such
      parameter clamping is the client's responsibility, which makes Skia's lighting effect easily
      adaptable to SVG 1.1 (clamp exponent) or SVG 2 (no clamp).
    - Fix spotlight incorrectly scaling light within the cone angle.
    - Move saturation of RGBA to after multiplying lighting intensity with the lighting color, which
      improves rendering when diffuse and specular constants are greater than 1.
    https://review.skia.org/355496

  * SkDeferredDisplayListRecorder::makePromiseTexture has moved to SkImage::MakePromiseTexture.
    New code should use the new entry point – migration CLs will be coming soon.
    https://review.skia.org/373716

* * *

Milestone 89
------------

  * Removed SkYUVAIndex and SkYUVASizeInfo. These were no longer used in any
    public APIs.
    https://review.skia.org/352497

  * Numerous changes to SkRuntimeEffect, aligning the capabilities and restrictions with
    The OpenGL ES Shading Language 1.00 (aka, the shading language of OpenGL ES2 and WebGL 1.0).
    All built-in functions from sections 8.1 through 8.6 implemented & tested on all backends.
    Removed types and features that require newer versions of GLSL:
      https://review.skia.org/346657  [Non-square matrices]
      https://review.skia.org/347046  [uint, short, ushort, byte, ubyte]
      https://review.skia.org/349056  [while and do-while loops]
      https://review.skia.org/350030  [Bitwise operators and integer remainder]

  * Add SkShadowUtils::GetLocalBounds. Generates bounding box for shadows
    relative to path.
    https://review.skia.org/351922

  * Removed SkPerlinNoiseShader::MakeImprovedNoise.
    https://review.skia.org/352057

  * Removed deprecated version of MakeFromYUVATextures. Use the version
    that takes GrYUVABackendTextures instead.
    https://review.skia.org/345174

  * SkAnimatedImage: Always respect exif orientation
    Replace SkPixmapPriv::ShouldSwapWidthHeight with
    SkEncodedOriginSwapsWidthHeight.
    https://review.skia.org/344762

  * Add kDirectionalLight_ShadowFlag support. If enabled, light position represents
    a vector pointing towards the light, and light radius is blur radius at elevation 1.
    https://review.skia.org/321792

  * Support GL_LUMINANCE8_ALPHA8 textures. These can be used with GrBackendTexture APIs
    on GrDirectContext and as planes of YUVA images via GrYUVABackendTextures.
    https://review.skia.org/344761

  * Removed previously deprecated SkImage::MakeFromYUVATexturesCopyToExternal.
    https://review.skia.org/342077

  * Add versions of GrDirectContext::createBackendTexture and updateBackendTexture
    that take a GrSurfaceOrigin. The previous versions are deprecated.
    https://review.skia.org/341005

  * Remove support for deprecated kDontClipToLayer_SaveLayerFlag in SkCanvas::SaveLayerRec
    https://review.skia.org/339988

  * Expose more info in SkCodec::FrameInfo
    https://review.skia.org/339857

  * Added dither control to the SkImageFilters::Shader factory.
    https://review.skia.org/338156

  * Add MTLBinaryArchive parameter to GrMtlBackendContext. This allows
    Skia to cache PipelineStates in the given archive for faster
    shader compiles on future runs. The client must handle loading and
    saving of the archive.
    https://review.skia.org/333758

  * Deprecated enum SkYUVAInfo::PlanarConfig has been removed.
    https://review.skia.org/334161

  * Deprecated SkImage factories have been removed from
    SkDeferredDisplayListRecorder.

  * The following YUV image factories have been removed:
    SkImage::MakeFromYUVTexturesCopyWithExternalBackend
    SkImage::MakeFromNV12TexturesCopyWithExternalBackend
    Replacement pattern outlined below.
        1) Make image using MakeFromYUVATextures
        2) Make a SkSurface around result texture using SkSurface::MakeFromBackendTexture
        3) surface->getCanvas()->drawImage(image, 0, 0);
        4) surface->flushAndSubmit()
        5) Optional: SkImage::MakeFromBackendTexture() to use as SkImage.
    https://review.skia.org/334596

  * Added a new interface for GrDirectContext creation in Metal, using
    a new struct called GrMtlBackendContext. The previous interface taking
    a MTLDevice and MTLCommandQueue is deprecated.
    https://review.skia.org/334426

  * SkCanvas::flush has been deprecated.

* * *

Milestone 88
------------

  * SkYUVAInfo now has separate enums for division of channels among planes and
    the subsampling. The previous combined enum, PlanarConfig, is deprecated.
    https://review.skia.org/334102

  * Simplified SkDeferredDisplayListRecorder promise image API. Removed "release"
    callback and renamed "done" callback to "release". The new "release" proc can
    be null. Added a new SkYUVAInfo-based factory for YUVA promise texture images
    and deprecated the old SkYUVAIndex-based one.
    https://review.skia.org/331836
    https://review.skia.org/333519

  * Limit the types and intrinsics supported in SkRuntimeEffect to GLSL ES 1.00
    https://review.skia.org/332597

  * Add AVIF support to SkHeifCodec.

  * Add support for creating SkSurfaceCharacterizations directly for use by a
    GrVkSecondaryCBDrawContext.
    https://review.skia.org/331877

  * Removed SkSurfaceProps::kLegacyFontHost_InitType, SkFontLCDConfig, and related code.
    The default pixel geometry for SkSurfaceProps is now kUnknown instead of kRGB_H.
    The removal was guarded by the SK_LEGACY_SURFACE_PROPS build flag which was later removed.
    https://review.skia.org/322490
    https://review.skia.org/329364

  * Legacy 8-bit YUV interface removed from SkImageGenerator. Use more flexible SkYUVAPixmaps-
    based interface instead.
    https://review.skia.org/327917

  * New variant of SkImage::MakeFromYUVATextures. Takes a new type GrYUVATextures
    which wraps an SkYUVAInfo and compatible set of GrBackendTextures. The provides
    a more complete and structured specification of the planar configuration. Previous
    version is deprecated.
    Already deprecated MakeFromYUVATexturesCopyToExternal added to replace other deprecated
    APIs. It's not recommended that clients use this and instead use the pattern described
    in the API comment.
    https://review.skia.org/317762
    https://review.skia.org/329956

  * Add field to GrContextOptions to disable mipmap support even if the backend
    supports it.

  * SkTPin() removed from public API.

  * Add new SkImageFilters::Blend factory function, in place of the now deprecated
    SkImageFilters::Xfermode factory function. Behavior is identical, but name better matches
    conventions in SkShader and SkColorFilter.
    https://review.skia.org/324623

  * SkImageFilters::Foo() factory functions now accept SkIRect, SkRect, and optional SkIRect* or
    SkRect*, instead of previously just the optional SkIRect*. Internally, the crop rects are stored
    as floats to allow for fractional crops to be defined in the local coordinate system (before
    transformation by the canvas matrix).
    https://review.skia.org/324622

  * Add new SkImageFilters::Shader factory and deprecate SkImageFilters::Paint factory. All
    supported/valid Paint() filters can be represented more cleanly as a Shader image filter.
    https://review.skia.org/323680

  * GrContext has been replaced by two separate classes: GrDirectContext which is
    the traditional notion of GrContext, and GrRecordingContext which is a context
    that is recording an SkDeferredDisplayList and therefore has reduced functionality.
    Unless you are using SkDeferredDisplayList, migrate directly to GrDirectContext in
    all cases.

  * CPU sync bool added to SkSurface::flushAndSubmit() and GrContext::flushAndSubmit()

  * Removed legacy variant of SkImage::MakeFromYUVAPixmaps. Use the version that
    takes SkYUVAPixmaps instead. It has a more structured description of the
    planar configuration.
    https://review.skia.org/322480

  * Some SkImage YUV image factories have been removed. Replacement patterns
    outlined below.
    SkImage::MakeFromYUVATexturesCopy
        1) Make SkImage from YUVA planes using SkImage::MakeFromYUVATextures
        2) Use Skia to allocate a surface using SkSurface::MakeRenderTarget
        3) surface->getCanvas()->drawImage(image, 0, 0);
        4) surface->makeImageSnapShot() produces RGBA image.
    SkImage::MakeFromYUVATexturesCopyWithExternalBackend
        1) Make image using MakeFromYUVATextures
        2) Make a SkSurface around result texture using SkSurface::MakeFromBackendTexture
        3) surface->getCanvas()->drawImage(image, 0, 0);
        4) surface->flushAndSubmit()
        5) Optional: SkImage::MakeFromBackendTexture() to use as SkImage.
    SkImage::MakeFromNV12TexturesCopy
        Same as SkImage::MakeFromYUVATexturesCopy
    https://review.skia.org/321537

  * GrBackendRenderTargets which are created with a stencilBits param, now require
    the stencilBits to be 0, 8, or 16.
    https://review.skia.org/321545

* * *

Milestone 87
------------

  * GrVkImageInfo now has a field for sample count. GrBackendRenderTarget constructor
    that took both a GrVkImageInfo and separate sample count is deprecated. Use the
    version without sample count instead. Similarly, GrD3DTextureResourceInfo now
    has a sample count field and GrBackendRenderTarget no longer takes a separate
    sample count for Direct3D. The sample count for GrBackendRenderTarget is now
    directly queried from MtlTexture rather than passed separately. The version that
    takes a separate sample count is deprecated and the parameter is ignored.
    https://review.skia.org/320262
    https://review.skia.org/320757
    https://review.skia.org/320956

  * Added deprecation warning for Metal support on MacOS 10.13, iOS 8.3, and older.
    https://review.skia.org/320260

  * GrVkImageInfo now has a field for sample count. GrBackendRenderTarget constructor
    that took both a GrVkImageInfo and separate sample count is deprecated. Use the
    version without sample count instead.

  * Update SkClipOp::kMax_EnumValue to include only intersect and difference when
    SK_SUPPORT_DEPRECATED_CLIPOPS is not defined.
    https://review.skia.org/320064

  * Add support for external allocator for Direct3D 12 backend.
    Defines base classes for an allocation associated with a backend texture and a
    a memory allocator to create such allocations.
    Adds memory allocator to backend context.
    https://review.skia.org/317243

  * Add new optional parameter to GrContext::setBackend[Texture/RenderTarget]State which can
    be used to return the previous GrBackendSurfaceMutableState before the requested change.
    https://review.skia.org/318698

  * New optimized clip stack for GPU backends. Enabled by default but old behavior based on
    SkClipStack can be restored by defining SK_DISABLE_NEW_GR_CLIP_STACK when building. It is not
    compatible with SK_SUPPORT_DEPRECATED_CLIPOPS and we are targeting the removal of support for
    the deprecated, expanding clip ops.
    https://review.skia.org/317209

  * GPU backends now properly honor the SkFilterQuality when calling drawAtlas.
    https://review.skia.org/313081

  * The signature of 'main' used with SkRuntimeEffect SkSL has changed. There is no longer an
    'inout half4 color' parameter, effects must return their color instead.
    Valid signatures are now 'half4 main()' or 'half4 main(float2 coord)'.
    https://review.skia.org/310756

  * New YUVA planar specifications for SkCodec, SkImageGenerator, SkImage::MakeFromYUVAPixmaps.
    Chroma subsampling is specified in more structured way. SkCodec and SkImageGenerator
    don't assume 3 planes with 8bit planar values. Old APIs are deprecated.
    https://review.skia.org/309658
    https://review.skia.org/312886
    https://review.skia.org/314276
    https://review.skia.org/316837
    https://review.skia.org/317097

  * Added VkImageUsageFlags to GrVkImageInfo struct.

* * *

Milestone 86
------------

  * Remove support for 'in' variables from SkRuntimeEffect. API now exclusively refers to inputs
    as 'uniforms'.
    https://review.skia.org/309050

  * Add SkImageGeneratorNDK and SkEncodeImageWithNDK for using Android's NDK APIs to decode and
    encode.
    https://review.skia.org/308185
    https://review.skia.org/308800

  * SkImage:remove DecodeToRaster, DecodeToTexture
    https://review.skia.org/306331

  * Add GrContext api to update compressed backend textures.
    https://review.skia.org/302265

  * Rename GrMipMapped to GrMipmapped for consistency with new APIs.
    Also rename GrBackendTexture::hasMipMaps() to GrBackendTexture::hasMipmaps()
    https://review.skia.org/304576
    https://review.skia.org/304598

  * Add option for clients to own semaphores after wait calls.
    https://review.skia.org/301216

  * Remove obsolete GrFlushFlags.
    https://review.skia.org/298818

  * Adds default flush() calls to SkSurface, SkImage, and GrContext. These calls do
    a basic flush without a submit. If you haven't updated Skia in a couple releases
    and still have flush() calls in your code that you expect to do a flush and
    submit, you should update all those to the previously added flushAndSubmit() calls
    instead.
    https://review.skia.org/299141

  * Enable BackendSemaphores for the Direct3D backend.
    https://review.skia.org/298752

  * Added SkImage:asyncRescaleAndReadPixels and SkImage::asyncRescaleAndReadPixelsYUV420
    https://review.skia.org/299281

  * Ganesh is moving towards replacing GrContext with the GrDirectContext/GrRecordingContext
    pair. GrDirectContexts have _direct_ access to the GPU and are very similar to the old
    GrContext. GrRecordingContexts are less powerful contexts that lack GPU access but provided
    context-like utilities during DDL recording. SkSurfaces and SkCanvas will now only return
    GrRecordingContexts. Clients requiring context features that need GPU access can then
    check (via GrRecordingContext::asDirectContext) if the available recording context is actually
    a direct context.

  * Replace #defined values in SkString with equivalent constexprs.
    http://review.skia.org/306160

* * *

Milestone 85
------------

  * Added GrContext::oomed() which reports whether Skia has seen a GL_OUT_OF_MEMORY
    error from Open GL [ES] or VK_ERROR_OUT_OF_*_MEMORY from Vulkan.
    https://review.skia.org/298216

  * Add option on SkSurface::flush to pass in a GrBackendSurfaceMutableState which
    we will set the gpu backend surface to be at the end of the flush.
    https://review.skia.org/295567

  * Add GrContext function to set mutable state on a backend surface. Currently this
    is only used for setting vulkan VkImage layout and queue family.
    https://review.skia.org/293844

  * SkSurface factores that take GrBackendTexture or GrBackendRenderTarget now always
    call the release proc (if provided) on failure. SkSurface::replaceBackendTexture
    also calls the release proc on failure.
    https://review.skia.org/293762

  * SkSurface::asyncRescaleAndReadPixels and SkSurfaceasyncRescaleAndReadPixelsYUV420
    now require explicit GrContext submit to guarantee finite time before callback
    is invoked.
    https://review.skia.org/292840

  * Add VkSharingMode field to GrVkImageInfo.
    https://review.skia.org/293559

  * Move SkBitmapRegionDecoder into client_utils/android.

  * SkCanvas.clear and SkCanvas.drawColor now accept SkColor4f in addition to SkColor.

  * Remove SkSurface::MakeFromBackendTextureAsRenderTarget.
    This factory existed to work around issues with GL_TEXTURE_RECTANGLE that existed
    in Chrome's command buffer. Those issues have since been resolved. Use
    SkSurface::MakeFromBackendTexutre or SkSurface::MakeFromBackendRenderTarget instead.
    https://review.skia.org/292719

  * Adds submittedProc callback to GrFlushInfo which will be called when the work
    from the flush call is submitted to the GPU. This is specifically useful for knowing
    when semahpores sent with the flush have been submitted and can be waiting on.
    https://review.skia.org/291078

  * GrContext submit is now required to be called in order to send GPU work to the
    actual GPU. The flush calls simply produces 3D API specific objects that are ready
    to be submitted (e.g. command buffers). For the GL backend, the flush will still
    send commands to the driver. However, clients should still assume the must call
    submit which is where any glFlush that is need for sync objects will be called. There,
    are flushAndSubmit() functions of GrContext, SkSurface, and SkImage that will act
    like the previous flush() functions. This will flush the work and immediately call
    submit.
    https://review.skia.org/289033

  * Remove deprecated version of flush calls on GrContext and SkSurface.
    https://review.skia.org/2290540

  * SkCanvas::drawVertices and drawPatch now support mapping an SkShader without explicit
    texture coordinates. If they're not supplied, the local positions (vertex position or
    patch cubic positions) will be directly used to sample the SkShader.
    https://review.skia.org/290130

* * *

Milestone 84
------------

  * Add api on GrContext, updateBackendTexture that will upload new data to a
    GrBackendTexture.
    https://review.skia.org/288909

  * Add GrContext getter to SkSurface.
    https://review.skia.org/289479

  * Deprecate GrContext and SkSurface flush() call and replace ith with flushAndSubmit().
    This only effects the default flush call that takes no parameters.
    https://review.skia.org/289478

  * GrContext::createBackendTexture functions that initialize the texture no longer
    guarantee that all the data has been uploaded and the gpu is done with the texture.
    Instead the client can assume the upload work has been submitted to the gpu and they
    must wait for that work to finish before deleting the texture. This can be done via
    their own synchronization or by passing in a finish proc into the create calls which
    will be called when it is safe to delete the texture (at least in terms of work
    done during the create).
    https://review.skia.org/286517

  * Remove unused SkMaskFilter helpers: compbine, compose
    Note: shadermaskfilter will likely be removed next (clipShader should serve)

  * Add back SkCanvas::kPreserveLCDText_SaveLayerFlag to indicate that saveLayer()
    will preserve LCD-text. All text in the layer must be drawn on opaque background
    to ensure correct rendering.

  * Add the new directory client_utils/ for code that is specific to a single client and
    should be considered separate from Skia proper. Move SkFrontBufferedStream into the
    subdir android/.

  * SkBitmap and SkPixmap's erase() methods now treat their color parameters
    consistently with the rest of Skia, with all SkColors and any untagged
    SkColor4fs interpreted as sRGB, not as a color in the bitmap's color space.
    SkPixmap::erase(SkColor4f) now takes an SkColorSpace, so you can pass
    pixmap.colorSpace() if you want the old behavior.

  * SkCamera.h and SkMatrix44.h are DEPRECATED.
    Use SkM44 if you want to have 3d transformations.

  * Changed Dilate and Erode image filters to take SkScalar for radius instead of int. While
    the image filters themselves are defined in terms of discrete pixels, the radii provided by
    the user are mapped through the CTM so taking ints forced over discretization. After mapping
    through the CTM the radii are now rounded to pixels.
    https://review.skia.org/281731
    https://review.skia.org/282636

  * Updated the contract of GrContext and SkSurface flush calls in regards to semaphores. Made it
    clear that the caller is responsible for deleting any initialized semaphores after the flush
    call regardless if we were able to submit them or not. Also, allows skia to only submit a
    subset of the requested semaphores if we failed to create some.
    https://review.skia.org/282265


  * SkCanvas::drawVertices will now always fill the triangles specified by the vertices. Previously,
    vertices with no colors and no (texture coordinates or shader) would be drawn in wireframe.
    https://review.skia.org/282043

* * *

Milestone 83
------------

  * Remove localmatrix option from SkShaders::[Blend, Lerp]

  * Fill out Direct3D parameters for backend textures and backend rendertargets.

  * SkImage::makeTextureImage() takes an optional SkBudgeted param

  * Made non-GL builds of GPU backend more robust.
    https://review.skia.org/277456

  * MoltenVK support removed. Use Metal backend instead.
    https://review.skia.org/277612

* * *

Milestone 82
------------

  * Removed drawBitmap and related functions from SkDevice; all public drawBitmap functions on
    SkCanvas automatically wrap the bitmap in an SkImage and call the equivalent drawImage function.
    Drawing mutable SkBitmaps will now incur a mandatory copy. Switch to using SkImage directly or
    mark the bitmap as immutable before drawing.

  * Removed "volatile" flag from SkVertices. All SkVertices objects are assumed to be
    volatile (the previous default behavior).

  * Removed exotic legacy bitmap functions from SkCanvas (drawBitmapLattic, drawBitmapNine); the
    exotic SkImage functions still exist.

  * Make it possible to selectively turn on/off individual encoders/decoders,
    using skia_use_(libpng/libjpeg_turbo/libwebp)(decode/encode).

  * Removed GrGpuResource, GrSurface, and GrTexture from public api. These were not
    meant to be public, and we now can move them into src. Also removed getTexture
    function from SkImage.h

  * Removed Bones from SkVertices

  * Added a field to GrContextOptions that controls whether GL errors are checked after
    GL calls that allocate textures, etc. It also controls checking for shader compile
    success, and program linking success.

  * Made SkDeferredDisplayList.h officially part of the public API (i.e., moved it to
    include/core). Also added a ProgramIterator to SkDeferredDisplayList which allows
    clients to pre-compile some of the shaders the DDL requires.

  * Added two new helper methods to SkSurfaceCharacterization: createBackendFormat and
    createFBO0. These make it easier for clients to create new surface characterizations that
    differ only a little from an existing surface characterization.

  * Removed SkTMax and SkTMin.
  * Removed SkTClamp and SkClampMax.
  * Removed SkScalarClampMax and SkScalarPin.
  * Removed SkMax32 and SkMin32.
  * Removed SkMaxScalar and SkMinScalar.

  * SkColorSetA now warns if the result is unused.

  * An SkImageInfo with a null SkColorSpace passed to SkCodec::getPixels() and
    related calls is treated as a request to do no color correction at decode
    time.

  * Add new APIs to add attributes to document structure node when
    creating a tagged PDF.

  * Remove CGFontRef parameter from SkCreateTypefaceFromCTFont.
    Use CTFontManagerCreateFontDescriptorFromData instead of
    CGFontCreateWithDataProvider to create CTFonts to avoid memory use issues.

  * Added SkCodec:: and SkAndroidCodec::getICCProfile for reporting the native
    ICC profile of an encoded image, even if it doesn't map to an SkColorSpace.

  * SkSurface::ReplaceBackendTexture takes ContentChangeMode as a parameter,
    which allow callers to specify whether retain a copy of the current content.

  * Enforce the existing documentation in SkCanvas::saveLayer that it ignores
    any mask filter on the restore SkPaint. The 'coverage' of a layer is
    ill-defined, and masking should be handled by pre-clipping or using the
    auxiliary clip mask image of the SaveLayerRec.

* * *

Milestone 81
------------

  * Added support for GL_NV_fence extension.

  * Make SkImageInfo::validRowBytes require rowBytes to be pixel aligned. This
    makes SkBitmap match the behavior of raster SkSurfaces in rejecting
    non-aligned rowBytes.

  * Added an SkImage::MakeRasterFromCompressed entry point. Also updated
    SkImage::MakeFromCompressed to decompress the compressed image data if
    the GPU doesn't support the specified compression type (i.e., macOS Metal
    doesn't support BC1_RGB8_UNORM so such compressed images will always be
    decompressed on that platform).

  * Added support for BC1 RGBA compressed textures

  * Added CachingHint to SkImage::makeRasterImage

  * Added SkAnimatedImage::getCurrentFrame()

  * Add support to create an SkSurface from an MTKView, with delayed acquisition of
    the MTLDrawable.
    Entry point: SkSurface::MakeFromMTKView

  * Removed SkIRect::EmptyIRect(). Use SkIRect::MakeEmpty() instead.
    https://review.skia.org/262382/

  * Moved SkRuntimeEffect to public API. This is the new (experimental) interface to custom SkSL
    shaders and color filters.

  * Added BC1 compressed format support. Metal and Vulkan seem to only support the BC
    formats on desktop machines.

  * Added compressed format support for backend texture creation API.
    This adds the following new entry points:
    GrContext::compressedBackendFormat
    GrContext::createCompressedBackendTexture
    The latter method comes in variants that allow color-initialized and
    compressed texture data initialized.

  * Added SkMatrix::MakeTrans(SkIVector)
    https://review.skia.org/259804

* * *

Milestone 80
------------

  * For Vulkan backend, we now require that the VkDevice, Queue, and Instance outlive
    either the destruction or abandoning of the GrContext. Additionally, all
    GrBackendTextures created via GrContext::createBackendTexture calls must be deleted
    before destroying or abandoning the GrContext.
    https://review.skia.org/257921

  * Removed SkSize& SkSize::operator=(const SkISize&)
    https://review.skia.org/257880

  * SkISize width() and height() now constexpr
    https://review.skia.org/257680

  * Added SkMatrix::MakeTrans(SkVector) and SkRect::makeOffset(SkVector).
    https://review.skia.org/255782

  * Added SkImageInfo::MakeA8(SkISize) and added optional color space parameter to
    SkImageInfo::MakeN32Premul(SkISize).

  * Added dimensions() and getFrameCount() to SkAnimatedImage
    https://review.skia.org/253542

  * Removed SkMatrix44 version of toXYZD50 from SkColorSpace. Switched to skcms types in
    transferFn, invTrasnferFn, and gamutTransformTo functions.
    https://review.skia.org/252596

  * Removed rotation and YUV support from SkColorMatrix
    https://review.skia.org/252188

  * Added kBT2020_SkYUVColorSpace. This is BT.2020's YCbCr conversion (non-constant-luminance).
    https://review.skia.org/252160

  * Remove old async read pixels APIs
    https://review.skia.org/251198

  * Expose SkBlendModeCoeff and SkBlendMode_AsCoeff for Porter-Duff blend modes.
    https://review.skia.org/252600

* * *

Milestone 79
------------

  * SkTextBlob::Iter to discover the glyph indices and typefaces in each run
    https://skia-review.googlesource.com/246296

  * Added support for PQ and HLG transfer functions to SkColorSpace.
    https://skia-review.googlesource.com/c/skia/+/249000

  * Added new api on GrContext ComputeImageSize. This replaces the hold static helper
    ComputeTextureSize.
    https://skia-review.googlesource.com/c/skia/+/247337

  * New versions of SkSurface async-rescale-and read APIs that allow client to extend
    the lifetime of the result data. Old versions are deprecated.
    https://review.skia.org/245457

  * Add SkColorInfo. It's dimensionless SkImageInfo.
    https://review.skia.org/245261

  * Added SkPixmap-based createBackendTexture method to GrContext. This allows clients to create
    backend resources (initialized with texture data) that Skia/Ganesh doesn't know about/track.
    https://review.skia.org/244676

  * Add explicit src and dst colorspace parameters to SkColorFilter::filterColor4f()
    https://review.skia.org/244882

  * Remove Vulkan/Metal float32 RGBA texture support
    https://review.skia.org/244881

  * Add SkSurface::MakeFromCAMetalLayer
    https://review.skia.org/242563

  * Added kAlpha_F16_SkColorType, kRG_F16_SkColorType and kRGBA_16161616_SkColorType.
    This is intended to help support HDR YUV uses case (e.g., P010 and P016). As such,
    the addition is focused on allowing creation of SkPixmaps and SkImages and not
    SkSurfaces (i.e., who wants to render to render to these?)
    https://review.skia.org/241357

  * Start to move nested SkPath types (e.g. Direction, Verb) up to root level in SkPathTypes.h
    https://review.skia.org/241079

  * Remove isRectContour and ksNestedFillRects from public
    https://review.skia.org/241078

  * Added kRG_88_SkColorType. This is intended to help support YUV uses case (e.g., NV12).
    As such, the addition is focused on allowing creation of SkPixmaps and SkImages and not
    SkSurfaces (i.e., who wants to render to RG?)
    https://review.skia.org/239930
    https://review.skia.org/235797

  * Make the size of program/pipeline caches configurable via
    GrContextOptions::fRuntimeProgramCacheSize
    https://review.skia.org/239756

  * Added kAlpha_16_SkColorType and kRG_1616_SkColorType. This is intended to help support HDR YUV
    uses case (e.g., P010 and P016). As such, the addition is focused on allowing creation of
    SkPixmaps and SkImages and not SkSurfaces (i.e., who wants to render to render to these?)
    https://review.skia.org/239930

  * Add GrContext::precompileShader to allow up-front compilation of previously-cached shaders.
    https://review.skia.org/239438

* * *

Milestone 78
------------

  * SkDrawLooper is no longer supported in SkPaint or SkCanvas.
    https://review.skia.org/230579
    https://review.skia.org/231736

  * SkPath::Iter::next() now ignores its consumDegenerates bools. Those will so
    go away entirely
    https://review.skia.org/235104

  * SkImage: new factories: DecodeToRaster, DecodeToTexture
    https://review.skia.org/234476

  * SkImageFilter API refactor started:
    - Provide new factory API in include/effects/SkImageFilters
    - Consolidated enum types to use SkTileMode and SkColorChannel
    - Hide filter implementation classes
    - Hide previously public functions on SkImageFilter that were intended for
      internal use only
    https://review.skia.org/230198
    https://review.skia.org/230876
    https://review.skia.org/231256

  * SkColorFilters::HSLAMatrix - new matrix color filter operating in HSLA
    space.
    https://review.skia.org/231736

  * Modify GrBackendFormat getters to not return internal pointers. Use an enum
    class for GL formats.
    https://review.skia.org/233160

  * Expose GrContext::dump() when SK_ENABLE_DUMP_GPU is defined.
    https://review.skia.org/233557

  * Vulkan backend now supports YCbCr sampler for I420 Vulkan images that are
    not backed by external images.
    https://review.skia.org/233776

  * Add SkCodec::SelectionPolicy for distinguishing between decoding a still
    image or an image sequence for a container format that has both (e.g. HEIF).
    https://review.skia.org/232839

  * SkImage::makeTextureImage and SkImage::MakeCrossContextFromPixmap no longer
    take an SkColorSpace parameter. It was unused.
    https://review.skia.org/234579
    https://review.skia.org/234912

  * SkImage::reinterpretColorSpace - to reinterpret image contents in a new
    color space.
    https://review.skia.org/234328

  * Removed SkImage::MakeCrossContextFromEncoded.
    https://review.skia.org/234912

  * Add Metal support for GrFence, GrSemaphore, and GrBackendSemaphore
    https://review.skia.org/233416

  * SkMallocPixelRef: remove MakeDirect and MakeWithProc from API.
    https://review.skia.org/234660

  * Remove 4-parameter variant of SkRect::join() and intersect(), and
    noemptycheck variants of intersect().
    https://review.skia.org/235832
    https://review.skia.org/237142

  * Remove unused sk_sp comparison operators.
    https://review.skia.org/236942

  * Add SkColor4f variant to experimental_DrawEdgeAAQuad for SkiaRenderer.
    https://review.skia.org/237492

  * Deprecated maxCount resource cache limit for Ganesh.
    This hasn't been relevant for a long time.

  * Changed GrContextOptions' fDisallowGLSLBinaryCaching to fShaderCacheStrategy,
    and allow caching SkSL.
    https://review.skia.org/238856

  * Use GL_QCOM_TILED_RENDERING to explicitly discard stencil

  * Added RELEASE_NOTES.txt file
    https://review.skia.org/229760

  * Implemented internal support for OpenGL tessellation.
