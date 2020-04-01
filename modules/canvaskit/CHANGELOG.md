# CanvasKit Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
 - Support for DOMMatrix in addition to the SkMatrix currently supported by some APIs. [WIP]

### Removed
 - Previously deprecated functions MakeSkDashPathEffect, MakeLinearGradientShader,
   MakeRadialGradientShader, MakeTwoPointConicalGradientShader, MakeSkCornerPathEffect,
   MakeSkDiscretePathEffect

### Changed
 - CanvasKit colors are now represented with a TypedArray of four floats.

### Removed
 - SkPaint.setColorf is obsolete and removed. setColor accepts a CanvasKit color which is
   always composed of floats.
 - localmatrix option for `SkShader.Lerp` and `SkShader.Blend`.


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
