# CanvasKit Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
