Skia Update

Skia : Overview
- portable 2D graphics engine
- src: geometry, images, text
- dst : raster, gpu, pdf, displaylist, *user-defined
- attr: shaders, filters, antialiasing, blending, *user-defined

Skia : Clients
- Blink : direct and via GraphicsContext
- Chrome : ui/gfx and compositor
- Android framework
- third parties : e.g. Mozilla
- code.google.com/p/skia

Skia : Porting
- C++ and some SIMD assembly
- Fonts : CoreText, FreeType, GDI, DirectWrite, *user-define
- Threads : wrappers for native apis
- Memory : wrappers for [new, malloc, discardable]

Skia : API
- SkCanvas
-- save, saveLayer, restore
-- translate, scale, rotate, concat
-- clipRect, clipPath
- SkPaint
-- color, stroking, antialiasing, filtering
-- typeface, textSize, text-flags
-- effects: shader, color-filter, image-filter, mask-filter, xfermode

<blockstyle = code>
void onDraw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setFoo(...);
    canvas->drawRect(..., paint);
    paint.setBar(...);
    canvas->drawOval(..., paint);
}

<blockstyle = code>
void onDraw(SkCanvas* canvas) {
    canvas->drawRect(..., fPaint0);
    canvas->drawOval(..., fPaint1);
}

Skia In Blink : GraphicsContext
- Similar
-- rects, paths, images, text
-- matrices, clips
- Different
-- save/restore affect matrix+clip PLUS all paint settings
-- both fill and stroke settings are specified
-- hence: fillRect(), strokeRect(), drawRect()

<blockstyle = code>
void onDraw(GraphicsContext* gc) {
    gc->save();
    gc->setFoo(...);
    gc->fillRect(...);
    gc->setBar(...);
    gc->fillOval(...);
    gc->restore();
}

Skia In Blink : more than GraphicsContext
- Simple wrappers
-- FloatRect -- SkRect
-- Path -- SkPath
- Font.h + 21 others
-- SkTypeface + flags
- Image.h + 25 others
-- SkBitmap, SkImage

Skia In Blink : Fonts
- Assist with code-sharing between platforms
- Runtime switch between GDI and DirectWrite
- Add SkFontMgr for selection
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
- SkPaint::FilterLevel : none, low, medium (mipmaps), high
- SkPicture for caching SVG
- SkPicture + saveLayer() for masks
-- PathOps for resolving complex paths
- SkPictureShader for device-independent patterns
