Skia Update

Skia : Overview
- portable 2D graphics engine
- src : geometry, images, text
- attr: shaders, filters, antialiasing, blending
- dst : raster, gpu, pdf, picture

Skia : Porting
- C++ and some SIMD assembly
- Fonts : CoreText, FreeType, GDI, DirectWrite
- Threads : wrappers for native apis
- Memory : wrappers for [new, malloc, discardable]

Skia : Clients
- Blink : under the GraphicsContext hood
- Chrome : ui/gfx and compositor
- Android framework
- third parties : e.g. Mozilla
- sites.google.com/site/skiadocs

Skia In Blink

Skia In Blink : Fonts
- SkTypeface and SkFontMgr : platform agnostic
- Runtime switch between GDI and DirectWrite
- SkTextBlob to encapsulate runs of text
- Push LCD decision-making out of Blink

Skia In Blink : Record-Time-Rasterization
- Direct rendering during “Paint” pass
-- Image scaling, filters
-- SVG patterns, masks
- Problematic in modern Blink
-- CTM not always known/knowable
-- Rendering backend not always known (gpu or cpu)
-- Rasterization takes (too much) time

Skia In Blink : RTR response
- SkImageFilter w/ CPU and GPU implementations
- FilterLevel : none, low, medium (mipmaps), high
- SkPicture for caching SVG
- SkPicture + saveLayer() for masks
-- PathOps for resolving complex paths
- SkPictureShader for device-independent patterns

Skia In Blink : Recording
- GraphicsContext usuaually backed by SkPicture
-- draw commands are recorded for later playback
-- all parameters must be copied or (safely) ref'd
-- may record more than is currently visible
- Resulting picture may be replayed multiple times

Skia In Blink : Recording response
- New implementation
- Optimized for recording speed
-- shallow copies whenever possible
-- rearchitect all Skia effects to be immutable
- Reentrant-safe for playback in multiple threads
-- also affected effect subclasses

Skia In Blink : Playback
- Separate pass for optimizations (optional)
-- peep-holes rewrites
-- compute bounding-box hierarchy for faster tiling
-- can be done outside of Blink thread
- GPU optimizations
-- layer "hoisting"
-- distance field fonts

Skia : Roadmap

Skia In Blink : Roadmap
- GPU performance
-- extended OpenGL features (e.g. geometry shaders)
-- reordering for increased batching
-- support for new low-level OpenGL APIs
- Cross process support
-- immediate mode ala SkGPipe
-- serialize pictures

Skia API Roadmap
- Direct support for sRGB
- Stable C API / ABI
-- bindings for JS, Go, Python, Lua
- Robust file format

Demo

