Milestone Release Notes
=======================

This page includes a list of high level updates for each milestone release.

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
    https://review.skia.org/246296

  * Added support for PQ and HLG transfer functions to SkColorSpace.
    https://review.skia.org/249000

  * Added new api on GrContext ComputeImageSize. This replaces the hold static helper
    ComputeTextureSize.
    https://review.skia.org/247337

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
  * Added RELEASE_NOTES.txt file
    https://review.skia.org/229760

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

