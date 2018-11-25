Skia Overview 2014

< transition =slide>

One API -- many backends
- Raster [8888, 565, A8]
- GPU [opengl]
- PDF
- XPS
- Picture
- Pipe

<transition= fade>

One Team -- many clients
- Chrome
- ChromeOS
- Clank
- Android Framework
- 3rd parties (e.g. FireFox)

<transition= rotate>

<blockstyle = code>
Optimize for CPU variety
- x86 - 32bit (SSE, SSE2, ...), 64bit
- Arm - thumb, arm, NEON, ... 64bit?
- MIPS (just starting)

<transition= zoom>

Optimize for GPU variety
- Nvidia
- Qualcom
- Imagination
- ...
- ES2 -vs- ES3 -vs- Desktop profiles

Lots of testing and measuring
- build-bots
-- unittests, micro-benchmarks, image-regressions
-- http://108.170.217.252:10117/console
- webpage archives (in progress)
-- "map-reduce" server for saerching/historgrams
-- macro-benchmarks, image-reressions
-- gpu : cpu fuzzy compares

Skia Roadmap [Fall '13]

Roadmap in a nutshell
- GPU performance
- Pictures
- Images
- Fonts
- PDF

Roadmap : GPU Performance
- Clipping changes are expensive
- Texture cache optimizations
- Better batching / reordering
- Rely more on multi-sampling
- ES3/desktop features (e.g. path-rendering)
- ... continuo ad absurdum

Roadmap : Pictures
- Playback performance
-- improve culling
-- multi-core support
- Record performance
-- improve hash/cache
-- improve measuring/bbox computation
- Feedback to clients
-- annotations
-- heat-map for time spent drawing
-- peep-hole optimizations

Roadmap : Images
- HQ filtering and mipmaps
- Unpremul support
- sRGB support (future)
- Improve cache / lazy-decoding

Roadmap : Fonts
- Color emoji
- DirectWrite on windows
-- subpixel positioning!
- new FontMgr -- extended styles

Roadmap : PDF
- Android
-- perspective, color-filters
- New Viewer project
-- print-preview and more
-- can output picture / gpu directly
