Skia Update

Skia : Access
- https://skia.org
- https://skia.googlesource.com/skia

Skia : Overview
- portable graphics engine
- 2D transformations + perspective
- primitives: text, geometry, images
- effects: shaders, filters, antialiasing, blending

Skia : Porting
- C++ and some SIMD assembly
- Fonts : CoreText, FreeType, GDI, DirectWrite
- Threads : wrappers for native apis
- Memory : wrappers for [new, malloc, discardable]

Skia : Backends
- Surface
-- raster : ARGB, RGB16, A8 in software
-- gpu : transcribe to OpenGL
- Document
-- transcribe to PDF or XPS
- Record and Playback
-- Picture
-- Pipe

Skia : Clients
- Blink : under the GraphicsContext hood
- Chrome : ui/gfx and compositor
- Android : framework
- third parties : e.g. Mozilla

Skia In Blink

Skia In Blink : Fonts
- SkTypeface and SkFontMgr : platform agnostic
- Runtime switch between GDI and DirectWrite
- SkTextBlob to encapsulate runs of text
- Push LCD decision-making out of Blink

Skia In Blink : Record-Time-Rasterization
- What? : direct rendering during “Paint” pass
-- Image scaling, filters
-- SVG patterns, masks
- Problematic in modern Blink
-- CTM not always known/knowable
-- Rendering backend not always known (gpu or cpu)
-- Rasterization takes (too much) time

Skia In Blink : RTR response
- SkImageFilter w/ CPU and GPU implementations
- Bitmap scaling : bilerp, mipmaps, fancy
- SkPicture for caching SVG
- SkPicture + saveLayer() for masks
-- PathOps for resolving complex paths
- SkPictureShader for device-independent patterns

Skia In Blink : Recording
- GraphicsContext (now) backed by SkPicture
-- draw commands are recorded for later playback
-- all parameters must be copied or (safely) ref'd
-- may record more than is currently visible
- Resulting picture may be replayed multiple times
-- from different thread(s)

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
-- distance fields : fonts and concave paths

Skia In Blink : multi-picture-draw
- mpd(canvas[], picture[], matrix[], paint[])
- Requires independent canvas objects
-- all other parameters can be shared
-- draw order is unspecified
- Examples
-- 1 picture drawing to multiple tiles (canvases)
-- multiple pictures each drawing to its own layer

Skia In Blink : MPD optimizations*
- GPU
-- "layer hoisting" to reduce rendertarget switching
-- layer atlasing (also applies to imagefilters)
-- pre-uploading of textures
-- atlas yuv (from jpeg) to convert on gpu
- CPU
-- parallel execution using thread pool
-- pre-decoding of images based on visibility

Skia : Roadmap

Skia : Roadmap - performance
- GPU
-- extended OpenGL features (e.g. geometry shaders)
-- reordering for increased batching
-- support for new low-level OpenGL APIs
- CPU
-- SIMD applied to floats
-- smarter culling in pictures

Skia : Roadmap - API
- Cross process support
- Direct support for sRGB
- Robust file format
- Support PDF viewing
- Stable C ABI
-- bindings for JS, Go, Python, Lua

Demo
