# CanvasKit Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
 - Constants for the shadow flags. Of note, some of these values can be used on previous releases.
 - `getShadowLocalBounds()` to estimate the bounds of the shadows drawn by `Canvas.drawShadow`.

### Breaking
- `MakeImprovedNoise` is removed.

### Fixed
 - Improper error returned when a WebGL context could not be used.

## [0.22.0] - 2020-12-17

### Added
 - `Canvas.drawImageCubic`, `Canvas.drawImageOptions`, `Canvas.drawImageRectCubic`,
   `Canvas.drawImageRectOptions` to replace functionality that previously required FilterQuality.
 - A copy of this changelog is published in NPM releases for easier discovery.

### Breaking
 - `Canvas.drawImageNine` now takes a required FilterMode (the Paint still is optional).

## [0.21.0] - 2020-12-16

### Added
 - `getImageInfo()` and `getColorSpace()` to the `Image` type.
 - `CanvasKit.deleteContext()` for deleting WebGL contexts when done with them, resizing, etc.
 - `Image.makeCopyWithDefaultMipmaps()` for use with `Image.makeShaderOptions`; necessary if
   choosing a `MipmapMode` that is not `None`.

### Breaking
 - `Path.addPoly()` no longer accepts a 2d array of points, but a flattened 1d array.
 - `MakeVertices()` no longer accepts 2d arrays of points or texture coordinates, but
   flattened 1d arrays in both places.
 - `Paint.setFilterQuality`, `Paint.getFilterQuality`, `Image.makeShader` have been removed.
   The new way to specify interpolation settings is with the newly added `Image.makeShader*`
   methods. `Image.makeShaderCubic` is a replacement for high quality; `Image.makeShaderOptions`
   is for medium/low.

### Changed
 - `MakeImage` is now documented in the Typescript types (index.d.ts). The parameters have been
   streamlined to align with other, similar APIs.
 - `MakeAnimatedImageFromEncoded` respects Exif metadata. `MakeImageFromEncoded` already did so
   (and continues to do so).
 - The Canvas2D emulation layer always uses high quality image smoothing (this drastically
   simplifies the underlying code).
 - We now compile CanvasKit with emsdk 2.0.10 when testing and deploying to npm.
 - Instead of shipping a "core" build to npm, we ship a "profiling" build, which is the same as
   the main build, just with unmangled function calls and other debugging info useful for
   determining where runtime is spent.

### Fixed
 - `Canvas.drawPoints` correctly takes a flattened Array or TypedArray of points (as the
   documentation says), not a 2D array.

### Type Changes (index.d.ts)
 - Documented additional type for InputFlexibleColorArray.

## [0.20.0] - 2020-11-12

### Added
 - `MakeFractalNoise`, `MakeImprovedNoise`, and `MakeTurbulence` have been added to
   `CanvasKit.Shader`.
 - `MakeRasterDirectSurface` for giving the user direct access to drawn pixels.
 - `getLineMetrics` to Paragraph.
 - `Canvas.saveLayerPaint` as an experimental, undocumented "fast path" if one only needs to pass 
   the paint.
 - Support for .woff and .woff2 fonts. Disable .woff2 for reduced code size by supplying
   no_woff2 to compile.sh. (This removes the code to do brotli decompression).

### Breaking
 - `CanvasKit.MakePathFromSVGString` was renamed to `CanvasKit.Path.MakeFromSVGString`
 - `CanvasKit.MakePathFromOp` was renamed to `CanvasKit.Path.MakeFromOp`
 - The API for `Canvas.readPixels` and `Image.readPixels` has been reworked to more accurately
   reflect the C++ backend and each other. bytesPerRow is now a required parameter. They take an
   ImageInfo object to specify the output format. Additionally they take an optional malloc'd
   object as the last parameter. If provided, the data will be copied into there instead of
   allocating a new buffer.

### Changed
 - We now compile CanvasKit with emsdk 2.0.6 when testing and deploying to npm.
 - We no longer compile with rtti on, saving about 1% in code size.
 - `CanvasKit.Shader.Blend`, `...Color`, and `...Lerp` have been renamed to
   `CanvasKit.Shader.MakeBlend`, `...MakeColor` and `...MakeLerp` to align with naming conventions.
   The old names will be removed in an upcoming release.

### Removed
 - `CanvasKit.MakePathFromCmds`; Was deprecated in favor of `CanvasKit.Path.MakeFromCmds`.
 - `new CanvasKit.Path(path)` in favor of existing `path.copy()`.
 - Unused internal APIs (_getRasterN32PremulSurface, Drawable)
 - `measureText` from the CanvasContext2D emulation layer du to deprecation of measureText.

### Deprecated
 - `Font.getWidths` in favor of `Font.getGlyphIDs` and `Font.getGlyphWidths`.
 - `Font.measureText` in favor of the Paragraph APIs (which actually do shaping).

### Type Changes (index.d.ts)
 - Return value for MakeFromCmds correctly reflects the possibility of null.
 - `CanvasKit.GrContext` was renamed to `CanvasKit.GrDirectContext`.
 - Add docs/types for Shader Gradients (e.g. `CanvasKit.Shader.MakeLinearGradient`).

## [0.19.0] - 2020-10-08

### Breaking
 - "Sk" has been removed from all names. e.g. `new CanvasKit.SkPaint()` becomes
   `new CanvasKit.Paint()`. See `./types/index.d.ts` for all the new names.

### Removed
 - `Surface.captureFrameAsSkPicture`; it was deprecated previously.
 - `CanvasKit.MakeSkCornerPathEffect`, `CanvasKit.MakeSkDiscretePathEffect`,
   `CanvasKit.MakeBlurMaskFilter`, `CanvasKit.MakeSkDashPathEffect`,
   `CanvasKit.MakeLinearGradientShader`, `CanvasKit.MakeRadialGradientShader`,
   `CanvasKit.MakeTwoPointConicalGradientShader`;  these were deprecated previously and have
   replacements like `CanvasKit.PathEffect.MakeDash`.
 - `Canvas.concat44`; it was deprecated previously, just use `Canvas.concat`

## [0.18.1] - 2020-10-06

### Added
 - Typescript types (and documentation) are now in the types subfolder. We will keep these updated
   as we make changes to the CanvasKit library.

## [0.18.0] - 2020-10-05

### Breaking
 - SkRect are no longer returned from `CanvasKit.LTRBRect`, `CanvasKit.XYWHRect` nor
   are accepted as JS objects. Instead, the format is 4 floats in either an array, a
   Float32Array or a piece of memory returned by CanvasKit.Malloc. These floats are the
   left, top, right, bottom numbers of the rectangle.
 - SkIRect (Rectangles with Integer values) are no longer accepted as JS objects.
   Instead, the format is 4 ints in either an array, an Int32Array or a piece of memory
   returned by CanvasKit.Malloc. These ints are the left, top, right, bottom numbers of
   the rectangle.
 - SkRRect (Rectangles with rounded corners) are no longer returned from `CanvasKit.RRectXY`
   nor are accepted as JS objects. Instead, the format is 12 floats in either an array, a
   Float32Array or a piece of memory returned by CanvasKit.Malloc. The first 4 floats
   are the left, top, right, bottom numbers of the rectangle and then 4 sets of points
   starting in the upper left corner and going clockwise. This change allows for faster
   transfer between JS and WASM code.
 - `SkPath.addRoundRect` has been replaced with `SkPath.addRRect`. The same functionality
   can be had with the `CanvasKit.RRectXY` helper.
 - `SkPath.addRect` no longer accepts 4 floats as separate arguments. It only accepts
   an SkRect (an array/Float32Array of 4 floats) and an optional boolean for
   determining clockwise or counter-clockwise directionality.
 - The order of `SkCanvas.saveLayer` arguments is slightly different (more consistent).
   It is now `paint, bounds, backdrop, flags`

### Changed
 - We now compile CanvasKit with emsdk 2.0.0 when testing and deploying to npm.
 - WebGL interface creation is a little leaner in terms of code size and speed.
 - The signature of `main` used with SkSL passed to `CanvasKit.SkRuntimeEffect.Make` has changed.
   There is no longer an `inout half4 color` parameter, effects must return their color instead.
   Valid signatures are now `half4 main()` or `half4 main(float2 coord)`.
 - `SkPath.getBounds`, `SkShapedText.getBounds`, and `SkVertices.bounds` now
   take an optional argument. If a Float32Array with length 4 or greater is
   provided, the bounds will be copied into this array instead of allocating
   a new one.
 - `SkCanvas.drawAnimatedImage` has been removed in favor of calling
   `SkCanvas.drawImageAtCurrentFrame` or `SkAnimatedImage.makeImageAtCurrentFrame` and then
   `SkCanvas.drawImage`.
 - `SkTextBlob.MakeFromRSXform` also accepts a (possibly Malloc'd) Float32Array of RSXforms (
   see SkRSXform for more.)

### Removed
 - `SkCanvas.drawRoundRect` has been removed in favor of `SkCanvas.drawRRect`
   The same functionality can be had with the `CanvasKit.RRectXY` helper.
 - `SkPath.arcTo` which had been deprecated in favor of `SkPath.arcToOval`,
   `SkPath.arcToRotated`, `SkPath.arcToTangent`.
 - Extraneous ColorTypes from `ColorType` enum.

### Added
 - `CanvasKit.LTRBiRect` and `CanvasKit.XYWHiRect` as helpers to create SkIRects.
 - `SkCanvas.drawRect4f` as a somewhat experimental way to have array-free APIs for clients that
   already have their own representation of Rect. This is experimental because we don't know
   if it's faster/better under real-world use and because we don't want to commit to having these
   for all Rect APIs (and for similar types) until it has baked in a bit.
 - Added the following to `TextStyle`:
   - `decorationStyle`
   - `textBaseline`
   - `letterSpacing`
   - `wordSpacing`
   - `heightMultiplier`
   - `locale`
   - `shadows`
   - `fontFeatures`
 - Added `strutStyle` to `ParagraphStyle`.
 - Added `addPlaceholder` to `ParagraphBuilder`.
 - Added `getRectsForPlaceholders` to `Paragraph`.
 - `SkFont.getGlyphIDs`, `SkFont.getGlyphBounds`, `SkFont.getGlyphWidths` for turning code points
   into GlyphIDs and getting the associated metrics with those glyphs. Note: glyph ids are only
   valid for the font of which they were requested.
 - `SkTextBlob.MakeFromRSXformGlyphs` and `SkTextBlob.MakeFromGlyphs` as a way to build TextBlobs
   using GlyphIDs instead of code points.
 - `CanvasKit.MallocGlyphIDs` as a helper for pre-allocating space on the WASM heap for Glyph IDs.

### Deprecated
 - `SkAnimatedImage.getCurrentFrame`; prefer `SkAnimatedImage.makeImageAtCurrentFrame` (which
   follows the establishing naming convention).
 - `SkSurface.captureFrameAsSkPicture` will be removed in a future release. Callers can simply
   use `SkPictureRecorder` directly.
 - `CanvasKit.FourFloatArrayHelper` and related helpers (mostly helping with drawAtlas).
   `CanvasKit.Malloc` is the better tool and will replace these soon.
 - `SkPathMeasure`; SkContourMeasureIter has all the same functionality and a cleaner pattern.

### Fixed
 - Addressed Memory leak in `SkCanvas.drawText`.
 - Made SkTextBlob hang on to less memory during its lifetime.
 - `SkPath.computeTightBounds()` works again. Like getBounds() it takes an optional argument
   to put the bounds into.

## [0.17.3] - 2020-08-05

### Added
 - Added `CanvasKit.TypefaceFontProvider`, which can be used to register fonts
   with a font family alias. For example, "Roboto Light" may be registered with
   the alias "Roboto", and it will be used when "Roboto" is used with a light
   font weight.
 - Added `CanvasKit.ParagraphBuilder.MakeFromFontProvider` to make a
   `ParagraphBuilder` from a `TypefaceFontProvider`.
 - Added `CanvasKit.ParagraphBuilder.pushPaintStyle` which can be used to stroke or fill
   text with paints instead of simple colors.

## [0.17.2] - 2020-07-22

### Fixed
 - Shader programs are no longer generated with `do-while` loops in WebGL 1.0.

## [0.17.1] - 2020-07-21

### Added
 - Compile option to deserialize effects in skps `include_effects_deserialization`.

### Changed
 - Pathops and SKP deserialization/serialization enabled on the npm build.

## [0.17.0] - 2020-07-20

### Added
 - Added `CanvasKit.MakeImageFromCanvasImageSource` which takes either an HTMLImageElement,
   SVGImageElement, HTMLVideoElement, HTMLCanvasElement, ImageBitmap, or OffscreenCanvas and returns
   an SkImage. This function is an alternative to `CanvasKit.MakeImageFromEncoded` for creating
   SkImages when loading and decoding images. In the future, codesize of CanvasKit may be able to be
   reduced by removing image codecs in wasm, if browser APIs for decoding images are used along with
   `CanvasKit.MakeImageFromCanvasImageSource` instead of `CanvasKit.MakeImageFromEncoded`.
 - Three usage examples of `CanvasKit.MakeImageFromCanvasImageSource` in core.spec.ts.
 - Added support for asynchronous callbacks in perfs and tests.
 - `CanvasKit.SkPath.MakeFromVerbsPointsWeights` and `CanvasKit.SkPath.addVerbsPointsWeights` for
  supplying many path operations (e.g. moveTo, cubicTo) at once.
 - The object returned by `CanvasKit.malloc` now has a `subarray` method which works exactly like
  the normal TypedArray version. The TypedArray which it returns is also backed by WASM memory
  and when passed into CanvasKit will be used w/o copying the data (just like
  `Malloc.toTypedArray`).
 - `SkM44.setupCamera` to return a 4x4 matrix which sets up a perspective view from a camera.
 - `SkPath.arcToOval`, `SkPath.arcToTangent`, and `SkPath.arcToRotated` to replace the three
   overloads of `SkPath.arcTo`. https://github.com/flutter/flutter/issues/61305

### Changed
 - In all places where color arrays are accepted (gradient makers, drawAtlas, and MakeSkVertices),
   You can now provide either flat Float32Arrays of float colors, Uint32Arrays of int colors, or
   2d Arrays of Float32Array(4) colors. The one thing you should not pass is an Array of numbers,
   since canvaskit wouldn't be able to tell whether they're ints or floats without checking them all.
   The fastest choice for gradients is the flat Float32Array, the fastest choice for drawAtlas and
   MakeSkVertices is the flat Uint32Array.
 - Color arrays may also be objects created with CanvasKit.Malloc
 - renamed `reportBackendType` to `reportBackendTypeIsGPU` and made it return a boolean
 - `MakeWebGLCanvasSurface` can now accept an optional dictionary of WebGL context attributes that
   can be used to override default attributes.

### Fixed
 - `TextStyle.color` can correctly be a Malloc'd Float32Array.
 - Support wombat-dressing-room. go/npm-publish

### Deprecated
 - `CanvasKit.MakePathFromCmds` has been renamed to `CanvasKit.SkPath.MakeFromCmds`. The alias
   will be removed in an upcoming release.
 - `SkPath.arcTo` Separated into three functions.

## [0.16.2] - 2020-06-05

### Fixed
 - A bug where loading fonts (and other memory intensive calls) would cause CanvasKit
   to infrequently crash with
   `TypeError: Cannot perform %TypedArray%.prototype.set on a neutered ArrayBuffer`.
 - Incorrectly freeing Malloced colors passed into computeTonalColors.

## [0.16.1] - 2020-06-04

### Fixed
 - Colors are unsigned to be compatible with Flutter Web and previous behavior, not
   signed ints.

## [0.16.0] - 2020-06-03

### Added
 - Support for wide-gamut color spaces DisplayP3 and AdobeRGB. However, correct representation on a
   WCG monitor requires that the browser is rendering everything to the DisplayP3 or AdobeRGB
   profile, since there is not yet any way to indicate to the browser that a canvas element has a
   non-sRGB color space. See color support example in extra.html. Only supported for WebGL2 backed
   surfaces.
 - Added `SkSurface.reportBackendType` which returns either 'CPU' or 'GPU'.
 - Added `SkSurface.imageInfo` which returns an ImageInfo object describing the size and color
   properties of the surface. colorSpace is added to ImageInfo everywhere it is used.
 - `CanvasKit.Free` to explicitly clean up memory after `CanvasKit.Malloc`. All memory allocated
   with `CanvasKit.Malloc` must be released with `CanvasKit.Free` or it will be leaked. This can
   improve performance by reducing the copying of data between the JS and WASM side.
 - `CanvasKit.ColorAsInt`, `SkPaint.setColorComponents`, `SkPaint.setColorInt`,
   `SkCanvas.drawColorComponents`, `SkCanvas.drawColorInt` for when clients want
   to avoid the overhead of allocating an array for color components and only need 8888 color.

### Changed
 - We now compile/ship with Emscripten v1.39.16.
 - `CanvasKit.MakeCanvasSurface` accepts a new enum specifying one of the three color space and
   pixel format combinations supported by CanvasKit.
 - all `_Make*Shader` functions now accept a color space argument at the end. leaving it off or
   passing null makes it behave as it did before, defaulting to sRGB
 - `SkPaint.setColor` accepts a new color space argument, defaulting to sRGB.
 - Fewer allocations required to send Color and Matrices between JS and WASM layer.
 - All APIs that take a 1 dimensional array should also accept the object returned by Malloc. It is
   recommended to pass the Malloc object, as the TypedArray could be invalidated any time
   CanvasKit needs to allocate memory and needs to resize to accommodate.

### Breaking
 - `CanvasKitInit(...)` now directly returns a Promise. As such, `CanvasKitInit(...).ready()`
   has been removed.
 - `CanvasKit.MakeCanvasSurface` no longer accepts width/height arguments to override those on
   the canvas element. Use the canvas element's width/height attributes to dictate the size of
   the drawing area, and use CSS width/height to set the size it will appear on the page
   (it is rescaled after drawing when css sizing applies).
 - Memory returned by `CanvasKit.Malloc` will no longer be automatically cleaned up. Clients
   must use `CanvasKit.Free` to release the memory.
 - `CanvasKit.Malloc` no longer directly returns a TypedArray, but an object that can produce
   them with toTypedArray(). This is to avoid "detached ArrayBuffer" errors:
   <https://github.com/emscripten-core/emscripten/issues/6747>

### Fixed
 - WebGL context is no longer created with "antialias" flag. Using "antialias" caused poor AA
   quality in Ganesh when trying to do coverage-based AA with MSAA unknowingly enabled. It also
   reduced performance.

## [0.15.0] - 2020-05-14

### Added
 - Support for DOMMatrix on all APIs that take SkMatrix (i.e. arrays or Float32Arrays of length 6/9/16).
 - setEdging and setEmbeddedBitmaps to SkFont. You can disable the ability to draw aliased fonts (and save some code
   size) with the compile.sh argument `no_alias_font`.

### Removed
 - Previously deprecated functions `MakeSkDashPathEffect`, `MakeLinearGradientShader`,
   `MakeRadialGradientShader`, `MakeTwoPointConicalGradientShader`, `MakeSkCornerPathEffect`,
   `MakeSkDiscretePathEffect`

### Changed
 - CanvasKit colors are now represented with a TypedArray of four floats.
 - Calls to `getError` should be disabled. This may cause a performance improvement in some scenarios.

### Removed
 - SkPaint.setColorf is obsolete and removed. setColor accepts a CanvasKit color which is
   always composed of floats.
 - localmatrix option for `SkShader.Lerp` and `SkShader.Blend`.

### Deprecated
 - `SkCanvas.concat44` has been folded into concat (which now takes 3x2, 3x3, or 4x4 matrices). It will
   be removed soon.

### Fixed
 - Memory leak in paragraph binding code (https://github.com/flutter/flutter/issues/56938)
 - Safari now properly uses WebGL1 instead of WebGL2 when WebGL2 is not available (skbug.com/10171).

## [0.14.0] - 2020-03-18

### Added
 - `SkShader.MakeSweepGradient`
 - `SkCanvas.saveLayer` can now be called with 1 argument (the paint). In this case the current
   effective clip will be used, as the current rect is assumed to be null.
 - `SkPaint.setAlphaf`
 - Clients can supply `no_codecs` to compile.sh to remove all codec encoding and decoded code.
   This can save over 100 kb compressed if codecs are not needed.

### Deprecated
 - `MakeSkDashPathEffect` will be removed soon. Calls can be replaced with
   `SkPathEffect.MakeDash`.
 - `MakeLinearGradientShader` will be removed soon. Calls can be replaced with
   `SkShader.MakeLinearGradient`.
 - `MakeRadialGradientShader` will be removed soon. Calls can be replaced with
   `SkShader.MakeRadialGradient`.
 - `MakeTwoPointConicalGradientShader` will be removed soon. Calls can be replaced with
   `SkShader.MakeTwoPointConicalGradient`.

### Fixed
 - Shadows are properly draw on fillRect and strokeRect in the canvas2d emulation layer.
 - Shadow offsets properly ignore the CTM in the canvas2d emulation layer.

### Changed
 - Stop compiling jpeg and webp encoders by default. This results in a 100kb binary size reduction.
   Clients that need these encoders can supply `force_encode_webp` or `force_encode_jpeg` to
   compile.sh.

### Removed
 - Removed inverse filltypes.
 - Removed StrokeAndFill paint style.
 - Removed TextEncoding enum (it was only used internally). All functions assume UTF-8.

## [0.13.0] - 2020-02-28

### Deprecated
 - `MakeSkCornerPathEffect` will be removed soon. Calls can be replaced with
   `SkPathEffect.MakeCorner`.
 - `MakeSkDiscretePathEffect` will be removed soon. Calls can be replaced with
   `SkPathEffect.MakeDiscrete`.

### Added
 - `SkSurface.drawOnce` for drawing a single frame (in addition to already existing
   `SkSurface.requestAnimationFrame` for animation logic).
 - `CanvasKit.parseColorString` which processes color strings like "#2288FF"
 - Particles module now exposes effect uniforms, which can be modified for live-updating.
 - Experimental 4x4 matrices added in `SkM44`.
 - Vector math functions added in `SkVector`.
 - `SkRuntimeEffect.makeShaderWithChildren`, which can take in other shaders as fragmentProcessors.
 - `GrContext.releaseResourcesAndAbandonContext` to free up WebGL contexts.
 - A few methods on `SkFont`: `setHinting`, `setLinearMetrics`, `setSubpixel`.

### Changed
 - We now compile/ship with Emscripten v1.39.6.
 - `SkMatrix.multiply` can now accept any number of matrix arguments, multiplying them
    left-to-right.
 - SkMatrix.invert now returns null when the matrix is not invertible. Previously it would return an
   identity matrix. Callers must determine what behavior would be appropriate in this situation.
 - In Canvas2D compatibility layer, the underlying SkFont will have setSubpixel(true).
 - Bones are removed from Vertices builder

### Fixed
 - Support for .otf fonts (.woff and .woff2 still not supported).

## [0.12.0] - 2020-01-22

### Added
 - `SkFontMgr.countFamilies` and `SkFontMgr.getFamilyName` to expose the parsed font names.

### Changed
 - SKP serialization/deserialization now available (can be disabled with the 'no_skp').
   `SkPicture.DEBUGONLY_saveAsFile` renamed to `SkPicture.saveAsFile` and
   `CanvasKit.MakeSkPicture` is now exposed. SKP support is not shipped to npm builds.
   `force_serialize_skp` has been removed since it opt-out, not opt-in.

### Fixed
 - Bug that sometimes resulted in 'Cannot perform Construct on a neutered ArrayBuffer'
 - Bug with SkImage.readPixels (skbug.com/9788)
 - Bug with transparent colors in Canvas2d mode (skbug.com/9800)

## [0.11.0] - 2020-01-10

### Added
 - A "Core" build that removes Fonts, the Skottie animation player, the Particles demo,
   and PathOps is available in `bin/core/`. It is about half the size of the "CoreWithFonts"
   build.
 - Experimental Runtime shader available for custom builds.
 - WebP support.
 - `SkAnimatedImage.getCurrentFrame` which returns an SkImage.

### Fixed
 - `CanvasKit.SaveLayerInitWithPrevious` and `CanvasKit.SaveLayerF16ColorType` constants.
 - Some compilation configurations, for example, those with no fonts or just one of particles/skottie.

### Changed
 - Small tweaks to compilation settings to reduce code size and linkage time.
 - JS functions are no longer provided when the underlying c++ calls have been compiled out.

### Removed
 - `SkShader.Empty`
 - Support for Type 1 Fonts. These are ancient and removing them saves about 135k
   of code size.

### Breaking
 - In an effort to reduce code size for most clients, npm now contains two CanvasKit builds.
   In `bin/` there is the "CoreWithFonts" build that contains most functionality from 0.10.0.
   However, we no longer ship the Skottie animation player, nor the Particles demo. Further,
   PathOps are removed from this build `MakePathFromOp`, `SkPath.op` and `SkPath.simplify`.
   Clients who need any of those features are encouraged to create a custom build using
   `compile.sh`.
 - `SkPicture.DEBUGONLY_saveAsFile` was accidentally included in release builds. It has been
   removed. Clients who need this in a release build (e.g. to file a bug report that only
   reproduces in release) should do a custom build with the `force_serialize_skp` flag given.

### Deprecated
 - `SkCanvas.drawAnimatedImage` will be renamed soon. Calls can be replaced with `SkCanvas.drawImage`
   and `SkAnimatedImage.getCurrentFrame`.

## [0.10.0] - 2019-12-09

### Added
 - `SkContourMeasureIter` and `SkContourMeasure` as an alternative to `SkPathMeasure`.
 - CanvasKit image decode cache helpers: getDecodeCacheLimitBytes(), setDecodeCacheLimitBytes(),
   and getDecodeCacheUsedBytes().
 - `SkShader.Blend`, `SkShader.Color`, `SkShader.Empty`, `SkShader.Lerp`.

### Changed
 - The returned values from `SkParagraph.getRectsForRange` now have direction with value
   `CanvasKit.TextDirection`.

### Fixed
 - `MakeImage` properly in the externs file and can work with `CanvasKit.Malloc`.

## [0.9.0] - 2019-11-18
### Added
 - Experimental `CanvasKit.Malloc`, which can be used to create a
   TypedArray backed by the C++ WASM memory. This can save a copy in some cases
   (e.g. SkColorFilter.MakeMatrix). This is an advanced feature, so use it with care.
 - `SkCanvas.clipRRect`, `SkCanvas.drawColor`
 - Blur, ColorFilter, Compose, MatrixTransform SkImageFilters. Can be used with `SkPaint.setImageFilter`.
 - `SkCanvas.saveLayer` now takes 3 or 4 params to include up to bounds, paint, SkImageFilter, flags.
 - `SkPath.rArcTo`, `SkPath.rConicTo`, `SkPath.rCubicTo`, `SkPath.rLineTo`, `SkPath.rMoveTo`,
   `SkPath.rQuadTo`. Like their non-relative siblings, these are chainable.
 - Add `width()`, `height()`, `reset()`, `getFrameCount()` to SkAnimatedImage.
 - `SkCanvas.drawImageNine`, `SkCanvas.drawPoints` and related `PointMode` enum.
 - `SkPath.addPoly`
 - `SkPathMeasure.getSegment`
 - More information on SkParagraph API, eg. `getLongestLine()`, `getWordBoundary`, and others.

### Deprecated
 - `CanvasKit.MakeBlurMaskFilter` will be renamed/moved soon to `CanvasKit.SkMaskFilter.MakeBlur`.

### Changed
 - Use newer version of Freetype2 (Tracking Skia's DEPS now).
 - Use newer versions of libpng and zlib (Tracking Skia's DEPS now).

### Fixed
 - null dereference when sometimes falling back to CPU.
 - Actually ask WebGL for a stencil buffer.
 - Can opt out of Paragraph API with no_paragraph passed into compile.sh or when using primitive_shaper.

## [0.8.0] - 2019-10-21

### Added
 - `CanvasKit.MakeAnimatedImageFromEncoded`, `SkCanvas.drawAnimatedImage`.
 - `CanvasKit.SkFontMgr.FromData` which takes several ArrayBuffers of font data, parses
   them, reading the metadata (e.g. family names) and stores them into a SkFontMgr.
 - SkParagraph as an optional set of APIs for dealing with text layout.

### Changed
 - The `no_font` compile option should strip out more dead code related to fonts.
 - and `no_embedded_font` option now allows creating a `SkFontMgr.FromData` instead of
   always having an empty one.
 - Updated to emscripten 1.38.47
 - Switch to WebGL 2.0, but fall back to 1.0 when unavailable - bug.skia.org/9052

### Fixed
 - Null terminator bug in draw text - skbug.com/9314

## [0.7.0] - 2019-09-18

### Added
 - `SkCanvas.drawCircle()`, `SkCanvas.getSaveCount()`
 - `SkPath.offset()`, `SkPath.drawOval`
 - `SkRRect` support (`SkCanvas.drawRRect`, `SkCanvas.drawDRRect`, `CanvasKit.RRectXY`).
   Advanced users can specify the 8 individual radii, if needed.
 - `CanvasKit.computeTonalColors()`, which  returns TonalColors, which has an
   ambient SkColor and a spot SkColor.
 - `CanvasKit.SkColorFilter` and a variety of factories. `SkPaint.setColorFilter` is the only
   consumer of these at the moment.
 - `CanvasKit.SkColorMatrix` with functions `.identity()`, `.scaled()`, `.concat()` and
   others. Primarily for use with `CanvasKit.SkColorFilter.MakeMatrix`.

### Changed
 - `MakeSkVertices` uses a builder to save a copy.

### Breaking
 - When `SkPath.arcTo` is given seven arguments, it no longer turns the first four into
   a `SkRect` automatically, and instead uses them as
   `arcTo(rx, ry, xAxisRotate, useSmallArc, isCCW, x, y)` (see SkPath.h for more).

## [0.6.0] - 2019-05-06

### Added
 - `SkSurface.grContext` now exposed. `GrContext` has new methods for monitoring/setting
   the cache limits; tweaking these may lead to better performance in some cases.
   `getResourceCacheLimitBytes`, `setResourceCacheLimitBytes`, `getResourceCacheUsageBytes`
 - `SkCanvas.drawAtlas` for efficiently drawing multiple sprites from a sprite sheet with
   a set of transforms, color blends, etc.
 - `SkColorBuilder`, `RSXFormBuilder`, `SkRectBuilder` which increase performance by
   reducing the amount of malloc/free calls per frame, given that the array size is fixed.
 - Basic `SkPicture` support. `SkSurface.captureFrameAsSkPicture` is a helper function to
   capture an `SkPicture`, which can be dumped to disk (for debugging) with
   `SkPicture.DEBUGONLY_saveAsFile`.
 - `SkImage.readPixels`, which returns a TypedArray of pixel values (safe to use
   anywhere, doesn't need a delete()).

### Changed
 - Better `GrGLCaps` support for WebGL - this shouldn't have any impacts on APIs or
   correctness, except by perhaps fixing a few bugs in various surface types.
 - Use unsigned ints for SkColor on the JS side - this shouldn't have any impacts
   unless clients have pre-computed colors, in which case, they will need to re-compute them.
 - [breaking] Moved `CanvasKit.MakeImageShader` to `SkImage.makeShader` - removed clampUnpremul
   as argument.

## [0.5.1] - 2019-03-21

### Added
 - `SkPathMeasure`, `RSXFormBuilder`, `SkFont.getWidths`, `SkTextBlob.MakeFromRSXform`
   which were needed to add the helper function `SkTextBlob.MakeOnPath`.
 - `SkSurface.requestAnimationFrame` - wrapper around window.requestAnimationFrame that
   takes care of the setup/tear down required to use CanvasKit optimally. The callback
   has an `SkCanvas` as the first parameter - callers should draw on that.

### Changed
 - Location in Skia Git repo now `modules/canvaskit` (was `experimental/canvaskit`)

### Fixed
 - Extern bug in `CanvasKit.SkMatrix.invert`
 - Fallback to CPU now properly refreshes the canvas to get access to the
   CanvasRenderingContext2D.
 - Compile flags for better WebGL1 support for some graphics cards.
 - Antialias bug on large oval paths <https://crbug.com/skia/8873>

### Deprecated
 - `SkCanvas.flush` will be removed soon - client should only call `SkSurface.flush`


## [0.5.0] - 2019-03-08

### Added
 - isVolitile option to `CanvasKit.MakeSkVertices`. The previous (and current default) behavior
   was for this to be true; some applications may go faster if set to false.
 - `SkCanvas.saveLayer(rect, paint)`
 - `SkCanvas.restoreToCount(int)` which can be used with the output of .save() and .saveLayer().
 - Optional particles library from modules/particles. `See CanvasKit.MakeParticles(json)`;
 - More public APIs for working with Surfaces/Contexts `GetWebGLContext`,
   `MakeGrContext`, `MakeOnScreenGLSurface`, `MakeRenderTarget`.
 - `SkSurface.getSurface()` and `SkCanvas.getSurface()` for making compatible surfaces (typically
   used as a workspace and then "saved" with `surface.makeImageSnapshot()`)

### Breaking
 -  `CanvasKit.MakeWebGLCanvasSurface` no longer takes a webgl context as a first arg, only a
    canvas or an id of a canvas. If users want to manage their own GL contexts, they should build
    the `SkSurface` themselves with `GetWebGLContext` -> `MakeGrContext` ->
    `MakeOnScreenGLSurface`.

## [0.4.1] - 2019-03-01

### Added
 - Optional arguments to `MakeManagedAnimation` for supplying external assets (like images, fonts).

## [0.4.0] - 2019-02-25

### Added
 - `SkPath.addRoundRect`, `SkPath.reset`, `SkPath.rewind` exposed.
 - `SkCanvas.drawArc`, `SkCanvas.drawLine`, `SkCanvas.drawOval`, `SkCanvas.drawRoundRect` exposed.
 - Can import/export a SkPath to an array of commands. See `CanvasKit.MakePathFromCmds` and
   `SkPath.toCmds`.
 - `SkCanvas.drawTextBlob()` and `SkCanvas.SkTextBlob.MakeFromText()` to draw text to a canvas.
 - `CanvasKit.TextEncoding` enum. For use with `SkTextBlob`.
 - Text shaping with `ShapedText` object and `SkCanvas.drawText`. At compile time, one can choose
   between using Harfbuzz/ICU (default) or a primitive one ("primitive_shaper") which just does
   line breaking. Using Harfbuzz/ICU substantially increases code size (4.3 MB to 6.4 MB).

### Changed
 - `SkCanvas.drawText()` now requires an `SkFont` object for raw strings.


### Removed
 -  `SkPaint.setTextSize()`, `SkPaint.getTextSize()`, `SkPaint.setTypeface()`
   which should be replaced by using `SkFont`.
 - Deprecated `CanvasKitInit().then()` interface (see 0.3.1 notes)


### Fixed
 - Potential bug in `ready()` if already loaded.

## [0.3.1] - 2019-01-04
### Added
 - `SkFont` now exposed.
 - `MakeCanvasSurface` can now take a canvas element directly.
 - `MakeWebGLCanvasSurface` can now take a WebGL context as an integer and use it directly.

### Changed
 - `CanvasKitInit(...).then()` is no longer the recommended way to initialize things.
It will be removed in 0.4.0. Use `CanvasKitInit(...).ready()`, which returns a real Promise.

### Removed
- `SkPaint.measureText` - use `SkFont.measureText` instead.

## [0.3.0] - 2018-12-18

### Added
- Add Canvas2D JS layer. This mirrors the HTML Canvas API. This may be omitted at compile time
    it by adding `no_canvas` to the `compile.sh` invocation.
- `CanvasKit.FontMgr.DefaultRef()` and `fontmgr.MakeTypefaceFromData` to load fonts.
- Exposed `SkPath.setVolatile`. Some animations see performance improvements by setting
their paths' volatility to true.

### Fixed
- `SkPath.addRect` now correctly draws counter-clockwise vs clockwise.

### Changed
- `CanvasKit.MakeImageShader` no longer takes encoded bytes, but an `SkImage`, created from
    `CanvasKit.MakeImageFromEncoded`. Additionally, the optional parameters `clampIfUnpremul`
    and `localMatrix` have been exposed.
- `SkPath.arcTo` now takes `startAngle`, `sweepAngle`, `forceMoveTo` as additional parameters.
- `SkPath.stroke` has a new option `precision`  It defaults to 1.0.
- CanvasKit comes with one font (NotoMono) instead of the Skia TestTypeface. Clients are encouraged
  to use the new `fontmgr.MakeTypefaceFromData` for more font variety.

### Removed
- `CanvasKit.initFonts()` - no longer needed.


## [0.2.1] - 2018-11-20
Beginning of Changelog history
