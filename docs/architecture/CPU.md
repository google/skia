# Skia CPU Backend Architecture

This document outlines the major pieces of Skia's CPU backend. The CPU backend is responsible for
rendering graphics on the CPU, without the use of a GPU.

## Background and Definitions

*   **Bitmap**: A grid of pixels representing an image, where each pixel has a color. This is the
    digital canvas Skia's CPU backend paints on. Common image file formats like PNG and JPEG store
    bitmap data.
*   **Blend Mode**: A rule that defines how a source color (the one being drawn) is mathematically
    combined with a destination color (the one already on the canvas).
*   **Blitter**: A component that writes the final pixel colors from the rendering pipeline into the
    destination bitmap. It's the "hand" that actually puts the colored pixels onto the canvas.
*   **Clip**: A region that restricts drawing. Anything drawn outside the clip is not visible.
*   **Coverage Mask**: An array of values (typically 8-bit alpha) that represents how much a
    geometric shape covers each pixel. A value of 255 means fully covered, 0 means not covered,
    and intermediate values represent partial coverage for anti-aliasing.
*   **Hairline**: The thinnest visual line, typically one device pixel wide. A stroke width of 0
    in a paint is interpreted as a hairline. Hairlines do not scale with the CTM.
*   **Matrix**: A mathematical tool used to transform (translate, scale, rotate, skew) what is
    being drawn.
*   **Paint**: An object holding stylistic information for drawing, like color, stroke width, and
    effects.
*   **Path**: A sequence of lines and curves that describe a shape. Skia uses quadratic, cubic, and
    conic Bézier curves to create smooth, scalable curves. A quadratic curve has one control point,
    while a cubic curve has two for more complex shapes. A conic curve represents perfect
    circles etc using one control point and one weight.
*   **Rasterization**: Converting vector graphics (math-based shapes) into a grid of pixels (a
    raster image) for display.
*   **Scanline**: A single horizontal row of pixels. Complex shapes are often rendered one scanline
    at a time.
*   **Shader**: A program that calculates the color of a pixel. Shaders can produce solid colors,
    gradients, textures, or procedural patterns.
*   **Tiling**: An optimization strategy where a large drawing operation is broken into a series of
    smaller, tile-sized operations to keep memory usage (e.g., for intermediate buffers) bounded.
*   **Winding Number**: An algorithm used to determine if a point is inside a complex or
    self-intersecting path. Imagine drawing a ray from the point in any fixed direction to
    infinity. The winding number is the count of how many times the path crosses the ray in one
    direction (e.g., clockwise) minus the number of times it crosses in the other direction
    (counter-clockwise). For a non-zero winding rule, any point with a non-zero winding number is
    considered "inside" the path. This correctly handles shapes with holes and overlapping
    sections.

## Data Flow Walkthrough

The following sections describe the journey of a single drawing command (e.g.,
`canvas.drawPath(...)`) through the CPU backend, from the public API call to modifying raw pixel
memory.

See [CPU.dot](./CPU.dot) for a supplementary diagram.

### 1. The API Layer

The journey begins with the user-facing drawing API.

- **`SkSurface`**: Represents the drawing destination. A CPU-backed surface, created via
  **`SkSurfaces::Raster(...)`**, allocates and owns pixel memory via an `SkPixelRef`. The
  `SkSurface` manages an `SkBitmap` using this memory and provides an `SkCanvas` for drawing.
- **`SkImage`**: Represents an immutable snapshot of pixel data. It can be created from an
  `SkSurface` (via `surface->makeImageSnapshot()`), from an `SkBitmap`, or from encoded data
  (e.g., a PNG file). On the CPU backend, this is implemented by **`SkImage_Raster`**, which holds
  an `SkBitmap` that in turn points to the underlying pixels via an `SkPixelRef`. Because it is
  immutable, it can be cached and shared safely across threads. It can be drawn to a canvas via
  `canvas->drawImage(...)`.
- **`SkCanvas`**: The primary drawing interface with methods like `drawPath`, `drawImage`, etc. The
  user provides geometry (e.g., `SkPath`), images (`SkImage`), styling (`SkPaint`), and font
  information (`SkFont`) to the canvas. The `SkCanvas` holds the current transformation matrix
  and clip. When a draw call is made, the `SkCanvas` forwards all the information to an internal
  `SkDevice` for rendering.
- **`SkPath`**: A high-level object representing vector geometry (lines, curves). It is a
  lightweight, copy-on-write object that holds a shared pointer (`sk_sp`) to an `SkPathRef`.
- **`SkPathRef`**: The reference-counted object that actually stores the raw path data: arrays of
  points, verbs (move, line, quad, conic, cubic, close), and conic weights. `SkPathRef` objects are
  always **heap-allocated** and managed by `sk_sp`. Internally, `SkPathRef` uses
  `skia_private::STArray` for its geometry data storage. This `STArray` is a small-object optimized
  array, meaning that for small paths, the geometry data is stored directly within the `STArray`'s
  internal buffer (part of the heap-allocated `SkPathRef` object). For larger paths, the `STArray`
  dynamically allocates additional memory on the heap to store the geometry data.

#### Text Rendering

Drawing text is a specialized, but common, case. The process involves three key classes:
- **`SkFontMgr`**: A top-level object that discovers and manages the fonts installed on the system.
  It is used to create an `SkTypeface` by matching a font family name (e.g., "Roboto") or by
  loading font data directly from a file or stream. It also handles "fallback", which allows a user
  to find the data to draw a glyph from a collection of fonts and ordering heuristics (e.g. use this
  font for Latin characters and this font for emoji).
- **`SkTypeface`**: An immutable object representing the raw data of a single font face (e.g.,
  "Roboto Bold"). It typically contains the vector paths for each glyph, but also supports bitmap
  glyphs. Vector glyphs are rasterized and stored in a glyph cache (`SkStrikeCache`).
- **`SkFont`**: A lightweight object that holds a reference to an `SkTypeface` plus styling
  attributes like text size, scale, and skew.

When a user calls a method like `canvas.drawSimpleText(...)`, they provide the text and an `SkFont`
to the `SkCanvas`. The backend then uses the `SkFont` to look up the individual glyphs from the
`SkTypeface`. Each glyph is subsequently treated as a standard `SkPath` to be rendered.

**Note on Text Layout:** Core Skia handles rendering individual glyphs, but it does not perform
complex text layout (e.g., line breaking, justification, or bidirectional text). For rich text
layout, Skia provides the **`SkParagraph`** module, which is a higher-level library built on top
of Skia's core components.

#### Paint Configuration

The **`SkPaint`** object holds a suite of optional components that control the styling and
pixel-processing pipeline.
- **`SkShader`**: Generates the source color for the geometry. If no shader is present, the paint's
  color is used. Shaders can produce gradients, bitmap patterns, or procedural colors.
- **`SkColorFilter`**: Modifies the source color produced by the shader or paint. Common uses
  include tinting or applying a color matrix for color correction.
- **`SkMaskFilter`**: Operates on the shape's coverage mask, not its color. Its most common use is
  blurring the shape's edges, such as with a Gaussian blur from `SkMaskFilter::MakeBlur`.
- **`SkPathEffect`**: Modifies the geometry of a shape before it is drawn. For example,
  `SkPathEffect::MakeDash` creates an effect that turns solid lines into dashed lines. This
  happens before rasterization.
- **`SkBlender`**: Controls how the source color (from the shader) is blended with the destination
  color (already on the canvas). It is a more powerful, programmable version of the traditional
  `SkBlendMode` enum. The blending logic is executed as a stage in the `SkRasterPipeline`.
- **`SkImageFilter`**: Applies a complex, multi-pass effect to the output of a drawing operation.
  Unlike other effects, an image filter can operate on the entire result of a draw as a texture.
  This often requires allocating a temporary, offscreen layer. For example, a drop shadow filter
  might draw the shape into a layer, blur it, offset it, and then draw the original shape again
  on top of the blurred shadow.

### 2. The Device Layer: `SkBitmapDevice`

For the CPU backend, the `SkCanvas` forwards draw calls to an **`SkBitmapDevice`**. This object is
the concrete target for all CPU-based drawing. It manages the destination **`SkBitmap`** and
initiates rendering by creating and dispatching to an `SkDraw` object.

**A Note on Clipping:** The device manages an `SkRasterClipStack` corresponding to the canvas's
`save()`/`restore()` calls. For each draw, it resolves this stack into a single `SkRasterClip`
and passes it to `SkDraw`, which is stateless regarding the clip.

**A Note on Tiling:** For large or complex draws, the device may use tiling. It breaks the
operation into smaller, tile-sized chunks, adjusting the clip for each. This bounds the memory
required for intermediate buffers by invoking the draw process once per tile.

### 3. The Orchestrator: `SkDraw`

The `SkBitmapDevice` creates an **`SkDraw`** object for each primitive. `SkDraw` orchestrates a
single draw, bundling the geometry (`SkPath`), styling (`SkPaint`), transform, clip, and
destination `SkPixmap`. It uses `SkScan` to convert the vector shape into raster operations.

### 4. The Rasterizer: `SkScan`

`SkDraw` passes the `SkPath` to **`SkScan`**, the core rasterizer. `SkScan` converts the vector
path into horizontal scanlines, calculating a coverage mask (alpha values) for each. It is
unaware of color or effects. `SkDraw` provides `SkScan` with a `SkBlitter`, which `SkScan`
invokes for each scanline to render the coverage data.

**A Note on Scan Converters:** The `isAntiAlias()` flag on the `SkPaint` selects the scan
converter:
- **Aliased (`SkScan_Path.cpp`):** When anti-aliasing (AA) is off, it produces a binary (0% or
  100%) coverage mask based on whether pixel centers are inside the path, resulting in hard
  edges. For each horizontal run of covered pixels, it calls `blitter->blitH(x, y, width)`.
- **Analytic Anti-Aliased (`SkScan_AAAPath.cpp`):** When AA is on, it analytically calculates the
  exact geometric area of intersection with each pixel, producing fractional coverage values for
  smooth edges. For each horizontal run of partially-covered pixels, it calls
  `blitter->blitAntiH(x, y, alphas, runs)`.

### 5. The Pixel Writer: `SkRasterPipelineBlitter`

The **`SkRasterPipelineBlitter`** is the primary `SkBlitter` in the CPU backend. It implements the
`SkBlitter` interface, but instead of writing pixels directly, it translates calls like `blitH`
or `blitRect` from `SkScan` into an execution of its internal `SkRasterPipeline`. It configures
the pipeline with the correct coverage information from the blitter call and then runs the
pipeline to compute and write the final pixel colors to those lines.

### 6. The Workhorse: `SkRasterPipeline`

The **`SkRasterPipeline`** calculates final pixel colors by chaining together single-purpose
**stages**. For example, a draw might use stages for loading coverage (`load_a8`), setting a
source color (`uniform_color`), loading the destination, blending (`srcover`), and storing the
result (`store_8888`). This stage-based design avoids a combinatorial explosion of functions.

The pipeline is assembled into a sequence of pre-compiled "stages", which are executed in a loop
over the covered area. These functions use **SIMD (e.g., SSE, NEON)** to process multiple pixels
at once for high performance. The pipeline gets memory layout information from an `SkPixmap` to
load and store pixels.

### 7. The Pixel Memory View: `SkPixmap`, `SkBitmap`, and `SkPixelRef`

This is the final stop where pixels are modified.
- **`SkPixmap`**: A lightweight object with a pointer to pixel memory and its metadata
  (dimensions, color type). It provides the `SkRasterPipeline` with the exact memory addresses
  for reading and writing.
- **`SkBitmap`**: Held by `SkBitmapDevice`, it pairs an `SkImageInfo` with the pixel storage.
- **`SkPixelRef`**: A smart pointer that owns the heap-allocated pixel memory, originally created
  by the `SkSurface`.

The `SkRasterPipeline`'s final stage uses the address from the `SkPixmap` to write the new color
into the memory owned by the `SkPixelRef`, completing the draw.