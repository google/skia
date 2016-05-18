SkPaint
=======

*color, stroke, font, effects*

-   [SkXfermode](#SkXfermode) - transfer modes
-   [ShShader](#ShShader) - gradients and patterns
-   [SkMaskFilter](#SkMaskFilter) - modifications to the alpha mask
-   [SkColorFilter](#SkColorFilter) - modify the source color before applying the
-   [SkPathEffect](#SkPathEffect) - modify to the geometry before it
    generates an alpha mask.

Anytime you draw something in Skia, and want to specify what color it
is, or how it blends with the background, or what style or font to
draw it in, you specify those attributes in a paint.

Unlike `SkCanvas`, paints do not maintain an internal stack of state
(i.e. there is no save/restore on a paint). However, paints are
relatively light-weight, so the client may create and maintain any
number of paint objects, each set up for a particular use. Factoring
all of these color and stylistic attributes out of the canvas state,
and into (multiple) paint objects, allows canvas' save/restore to be
that much more efficient, as all they have to do is maintain the stack
of matrix and clip settings.

<!--?prettify lang=cc?-->

    void draw(SkCanvas* canvas) {
        canvas->clear(SK_ColorWHITE);

        SkPaint paint1, paint2, paint3;

        paint1.setTextSize(64.0f);
        paint1.setAntiAlias(true);
        paint1.setColor(SkColorSetRGB(255, 0, 0));
        paint1.setStyle(SkPaint::kFill_Style);

        paint2.setTextSize(64.f);
        paint2.setAntiAlias(true);
        paint2.setColor(SkColorSetRGB(0, 136, 0));
        paint2.setStyle(SkPaint::kStroke_Style);
        paint2.setStrokeWidth(SkIntToScalar(3));

        paint3.setTextSize(64.0f);
        paint3.setAntiAlias(true);
        paint3.setColor(SkColorSetRGB(136, 136, 136));
        paint3.setTextScaleX(SkFloatToScalar(1.5f));

        const char text[] = "Skia!";
        canvas->drawText(text, strlen(text), 20.0f, 64.0f,  paint1);
        canvas->drawText(text, strlen(text), 20.0f, 144.0f, paint2);
        canvas->drawText(text, strlen(text), 20.0f, 224.0f, paint3);
    }

<a href='https://fiddle.skia.org/c/@skpaint_skia'><img
  src='https://fiddle.skia.org/i/@skpaint_skia_raster.png'></a>

This shows three different paints, each set up to draw in a different
style. Now the caller can intermix these paints freely, either using
them as is, or modifying them as the drawing proceeds.

<!--?prettify lang=cc?-->

    SkPaint paint1, paint2, paint3;
    paint2.setStyle(SkPaint::kStroke_Style);
    paint2.setStrokeWidth(3);
    paint3.setAntiAlias(true);
    paint3.setColor(SK_ColorRED);
    paint3.setTextSize(80);

    canvas->drawRect(SkRect::MakeXYWH(10,10,60,20), paint1);
    canvas->drawRect(SkRect::MakeXYWH(80,10,60,20), paint2);

    paint2.setStrokeWidth(SkIntToScalar(5));
    canvas->drawOval(SkRect::MakeXYWH(150,10,60,20), paint2);

    canvas->drawText("SKIA", 4, 20, 120, paint3);
    paint3.setColor(SK_ColorBLUE);
    canvas->drawText("SKIA", 4, 20, 220, paint3);

<a href='https://fiddle.skia.org/c/@skpaint_mix'><img
  src='https://fiddle.skia.org/i/@skpaint_mix_raster.png'></a>

Beyond simple attributes such as color, strokes, and text values,
paints support effects. These are subclasses of different aspects of
the drawing pipeline, that when referenced by a paint (each of them is
reference-counted), are called to override some part of the drawing
pipeline.

For example, to draw using a gradient instead of a single color,
assign a SkShader to the paint.

<!--?prettify lang=cc?-->

    void draw(SkCanvas* canvas) {
        SkPoint points[2] = {
            SkPoint::Make(0.0f, 0.0f),
            SkPoint::Make(256.0f, 256.0f)
        };
        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeLinear(
                         points, colors, nullptr, 2,
                         SkShader::kClamp_TileMode, 0, nullptr));
        canvas->drawPaint(paint);
    }

<a href='https://fiddle.skia.org/c/@skpaint_shader'><img
  src='https://fiddle.skia.org/i/@skpaint_shader_raster.png'></a>

Now, anything drawn with that paint will be drawn with the gradient
specified in the call to `MakeLinear()`. The shader object that is
returned is reference-counted. Whenever any effects object, like a
shader, is assigned to a paint, its reference-count is increased by
the paint. To balance this, the caller in the above example calls
`unref()` on the shader once it has assigned it to the paint. Now the
paint is the only "owner" of that shader, and it will automatically
call `unref()` on the shader when either the paint goes out of scope, or
if another shader (or null) is assigned to it.

There are 6 types of effects that can be assigned to a paint:

*   **SkPathEffect** - modifications to the geometry (path) before it
    generates an alpha mask (e.g. dashing)
*   **SkRasterizer** - composing custom mask layers (e.g. shadows)
*   **SkMaskFilter** - modifications to the alpha mask before it is
    colorized and drawn (e.g. blur, emboss)
*   **SkShader** - e.g. gradients (linear, radial, sweep), bitmap patterns
    (clamp, repeat, mirror)
*   **SkColorFilter** - modify the source color(s) before applying the
    xfermode (e.g. color matrix)
*   **SkXfermode** - e.g. porter-duff transfermodes, blend modes

Paints also hold a reference to a SkTypeface. The typeface represents
a specific font style, to be used for measuring and drawing
text. Speaking of which, paints are used not only for drawing text,
but also for measuring it.

<!--?prettify lang=cc?-->

    paint.measureText(...);
    paint.getTextBounds(...);
    paint.textToGlyphs(...);
    paint.getFontMetrics(...);

<span id="SkXfermode"></span>

SkXfermode
----------

The following example demonstrates all of the Skia's standard transfer
modes.  In this example the source is a solid magenta color with a
horizontal alpha gradient and the destination is a solid cyan color
with a vertical alpha gradient.

<!--?prettify lang=cc?-->

    void draw(SkCanvas* canvas) {
        SkXfermode::Mode modes[] = {
            SkXfermode::kClear_Mode,
            SkXfermode::kSrc_Mode,
            SkXfermode::kDst_Mode,
            SkXfermode::kSrcOver_Mode,
            SkXfermode::kDstOver_Mode,
            SkXfermode::kSrcIn_Mode,
            SkXfermode::kDstIn_Mode,
            SkXfermode::kSrcOut_Mode,
            SkXfermode::kDstOut_Mode,
            SkXfermode::kSrcATop_Mode,
            SkXfermode::kDstATop_Mode,
            SkXfermode::kXor_Mode,
            SkXfermode::kPlus_Mode,
            SkXfermode::kModulate_Mode,
            SkXfermode::kScreen_Mode,
            SkXfermode::kOverlay_Mode,
            SkXfermode::kDarken_Mode,
            SkXfermode::kLighten_Mode,
            SkXfermode::kColorDodge_Mode,
            SkXfermode::kColorBurn_Mode,
            SkXfermode::kHardLight_Mode,
            SkXfermode::kSoftLight_Mode,
            SkXfermode::kDifference_Mode,
            SkXfermode::kExclusion_Mode,
            SkXfermode::kMultiply_Mode,
            SkXfermode::kHue_Mode,
            SkXfermode::kSaturation_Mode,
            SkXfermode::kColor_Mode,
            SkXfermode::kLuminosity_Mode,
        };
        SkRect rect = SkRect::MakeWH(64.0f, 64.0f);
        SkPaint text, stroke, src, dst;
        stroke.setStyle(SkPaint::kStroke_Style);
        text.setTextSize(24.0f);
        text.setAntiAlias(true);
        SkPoint srcPoints[2] = {
            SkPoint::Make(0.0f, 0.0f),
            SkPoint::Make(64.0f, 0.0f)
        };
        SkColor srcColors[2] = {
            SK_ColorMAGENTA & 0x00FFFFFF,
            SK_ColorMAGENTA};
        src.setShader(SkGradientShader::MakeLinear(
                    srcPoints, srcColors, nullptr, 2,
                    SkShader::kClamp_TileMode, 0, nullptr));

        SkPoint dstPoints[2] = {
            SkPoint::Make(0.0f, 0.0f),
            SkPoint::Make(0.0f, 64.0f)
        };
        SkColor dstColors[2] = {
            SK_ColorCYAN & 0x00FFFFFF,
            SK_ColorCYAN};
        dst.setShader(SkGradientShader::MakeLinear(
                    dstPoints, dstColors, nullptr, 2,
                    SkShader::kClamp_TileMode, 0, nullptr));
        canvas->clear(SK_ColorWHITE);
        size_t N = sizeof(modes) / sizeof(modes[0]);
        size_t K = (N - 1) / 3 + 1;
        SkASSERT(K * 64 == 640);  // tall enough
        for (size_t i = 0; i < N; ++i) {
            SkAutoCanvasRestore autoCanvasRestore(canvas, true);
            canvas->translate(192.0f * (i / K), 64.0f * (i % K));
            const char* desc = SkXfermode::ModeName(modes[i]);
            canvas->drawText(desc, strlen(desc), 68.0f, 30.0f, text);
            canvas->clipRect(SkRect::MakeWH(64.0f, 64.0f));
            canvas->drawColor(SK_ColorLTGRAY);
            (void)canvas->saveLayer(nullptr, nullptr);
            canvas->clear(SK_ColorTRANSPARENT);
            canvas->drawPaint(dst);
            src.setXfermodeMode(modes[i]);
            canvas->drawPaint(src);
            canvas->drawRect(rect, stroke);
        }
    }

<a href='https://fiddle.skia.org/c/@skpaint_xfer'><img
  src='https://fiddle.skia.org/i/@skpaint_xfer_raster.png'></a>

<span id="ShShader"></span>

ShShader
--------

Several shaders are defined (besides the linear gradient already mentioned):

*   Bitmap Shader

    <!--?prettify lang=cc?-->

        canvas->clear(SK_ColorWHITE);
        SkMatrix matrix;
        matrix.setScale(0.75f, 0.75f);
        matrix.preRotate(30.0f);
        SkPaint paint;
        paint.setShader(SkShader::MakeBitmapShader(source,
                    SkShader::kRepeat_TileMode,
                    SkShader::kRepeat_TileMode,
                    &matrix));
        canvas->drawPaint(paint);

    <a href='https://fiddle.skia.org/c/@skpaint_bitmap_shader'><img
      src='https://fiddle.skia.org/i/@skpaint_bitmap_shader_raster.png'></a>

*   Radial Gradient Shader

    <!--?prettify lang=cc?-->

        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeRadial(
                    SkPoint::Make(128.0f, 128.0f), 180.0f,
                    colors, nullptr, 2, SkShader::kClamp_TileMode, 0, nullptr));
        canvas->drawPaint(paint);

    <a href='https://fiddle.skia.org/c/@skpaint_radial'><img
      src='https://fiddle.skia.org/i/@skpaint_radial_raster.png'></a>

*  Two-Point Conical Gradient Shader

    <!--?prettify lang=cc?-->

        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeTwoPointConical(
              SkPoint::Make(128.0f, 128.0f), 128.0f,
              SkPoint::Make(128.0f, 16.0f), 16.0f,
              colors, nullptr, 2, SkShader::kClamp_TileMode, 0, nullptr));
        canvas->drawPaint(paint);

    <a href='https://fiddle.skia.org/c/@skpaint_2pt'><img
      src='https://fiddle.skia.org/i/@skpaint_2pt_raster.png'></a>


*   Sweep Gradient Shader

    <!--?prettify lang=cc?-->

        SkColor colors[4] = {
            SK_ColorCYAN, SK_ColorMAGENTA, SK_ColorYELLOW, SK_ColorCYAN};
        SkPaint paint;
        paint.setShader(SkGradientShader::MakeSweep(
                    128.0f, 128.0f, colors, nullptr, 4, 0, nullptr));
        canvas->drawPaint(paint);

    <a href='https://fiddle.skia.org/c/@skpaint_sweep'><img
      src='https://fiddle.skia.org/i/@skpaint_sweep_raster.png'></a>

*   Fractal Perlin Noise Shader

    <!--?prettify lang=cc?-->

        canvas->clear(SK_ColorWHITE);
        SkPaint paint;
        paint.setShader(SkPerlinNoiseShader::MakeFractalNoise(
                 0.05f, 0.05f, 4, 0.0f, nullptr));
        canvas->drawPaint(paint);

    <a href='https://fiddle.skia.org/c/@skpaint_perlin'><img
    src='https://fiddle.skia.org/i/@skpaint_perlin_raster.png'></a>

*   Turbulence Perlin Noise Shader

    <!--?prettify lang=cc?-->

        canvas->clear(SK_ColorWHITE);
        SkPaint paint;
        paint.setShader(SkPerlinNoiseShader::MakeTurbulence(
                 0.05f, 0.05f, 4, 0.0f, nullptr));
        canvas->drawPaint(paint);

    <a href='https://fiddle.skia.org/c/@skpaint_turb'><img
      src='https://fiddle.skia.org/i/@skpaint_turb_raster.png'></a>

*   Compose Shader

    <!--?prettify lang=cc?-->

        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkPaint paint;
        paint.setShader(
            SkShader::MakeComposeShader(
                SkGradientShader::MakeRadial(
                    SkPoint::Make(128.0f, 128.0f), 180.0f,
                    colors, nullptr, 2, SkShader::kClamp_TileMode, 0, nullptr),
                SkPerlinNoiseShader::MakeTurbulence(0.025f, 0.025f, 2, 0.0f, nullptr),
                SkXfermode::kDifference_Mode)
            );
        canvas->drawPaint(paint);

    <a href='https://fiddle.skia.org/c/@skpaint_compose_shader'><img
      src='https://fiddle.skia.org/i/@skpaint_compose_shader_raster.png'></a>


<span id="SkMaskFilter"></span>

SkMaskFilter
------------

*   Blur Mask Filter

    <!--?prettify lang=cc?-->

        canvas->drawText(text, strlen(text), 0, 160, paint);
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(120);
        paint.setMaskFilter(SkBlurMaskFilter::Make(
                kNormal_SkBlurStyle, 5.0f, 0));
        const char text[] = "Skia";
        canvas->drawText(text, strlen(text), 0, 160, paint);

    <a href='https://fiddle.skia.org/c/@skpaint_blur_mask_filter'><img
      src='https://fiddle.skia.org/i/@skpaint_blur_mask_filter_raster.png'></a>

*   Emboss Mask Filter

    <!--?prettify lang=cc?-->

        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(120);
        SkScalar direction[3] = {1.0f, 1.0f, 1.0f};
        paint.setMaskFilter(SkBlurMaskFilter::MakeEmboss(
                2.0f, direction, 0.3f, 0.1f));
        const char text[] = "Skia";
        canvas->drawText(text, strlen(text), 0, 160, paint);

    <a href='https://fiddle.skia.org/c/@skpaint_emboss'><img
      src='https://fiddle.skia.org/i/@skpaint_emboss_raster.png'></a>


<span id="SkColorFilter"></span>

SkColorFilter
-------------

*   Color Matrix Color Filter

    <!--?prettify lang=cc?-->

        void f(SkCanvas* c, SkScalar x, SkScalar y, SkScalar colorMatrix[20]) {
            SkPaint paint;
            paint.setColorFilter(SkColorFilter::MakeMatrixFilterRowMajor255(colorMatrix));
            c->drawBitmap(source, x, y, &paint);
        }

        void draw(SkCanvas* c) {
            c->scale(0.25, 0.25);
            SkScalar colorMatrix1[20] = {
                0, 1, 0, 0, 0,
                0, 0, 1, 0, 0,
                1, 0, 0, 0, 0,
                0, 0, 0, 1, 0};
            f(c, 0, 0, colorMatrix1);

            SkScalar grayscale[20] = {
                0.21, 0.72, 0.07, 0.0, 0.0,
                0.21, 0.72, 0.07, 0.0, 0.0,
                0.21, 0.72, 0.07, 0.0, 0.0,
                0.0,  0.0,  0.0,  1.0, 0.0};
            f(c, 512, 0, grayscale);

            SkScalar colorMatrix3[20] = {
                -1, 1, 1, 0, 0,
                1, -1, 1, 0, 0,
                1, 1, -1, 0, 0,
                0, 0, 0, 1, 0};
            f(c, 0, 512, colorMatrix3);

            SkScalar colorMatrix4[20] = {
                0.0, 0.5, 0.5, 0, 0,
                0.5, 0.0, 0.5, 0, 0,
                0.5, 0.5, 0.0, 0, 0,
                0.0, 0.0, 0.0, 1, 0};
            f(c, 512, 512, colorMatrix4);

            SkScalar highContrast[20] = {
                4.0, 0.0, 0.0, 0.0, -4.0 * 255 / (4.0 - 1),
                0.0, 4.0, 0.0, 0.0, -4.0 * 255 / (4.0 - 1),
                0.0, 0.0, 4.0, 0.0, -4.0 * 255 / (4.0 - 1),
                0.0, 0.0, 0.0, 1.0, 0.0};
            f(c, 1024, 0, highContrast);

            SkScalar colorMatrix6[20] = {
                0, 0, 1, 0, 0,
                1, 0, 0, 0, 0,
                0, 1, 0, 0, 0,
                0, 0, 0, 1, 0};
            f(c, 1024, 512, colorMatrix6);

            SkScalar sepia[20] = {
                0.393, 0.769, 0.189, 0.0, 0.0,
                0.349, 0.686, 0.168, 0.0, 0.0,
                0.272, 0.534, 0.131, 0.0, 0.0,
                0.0,   0.0,   0.0,   1.0, 0.0};
            f(c, 1536, 0, sepia);

            SkScalar inverter[20] = {
                -1,  0,  0, 0, 255,
                 0, -1,  0, 0, 255,
                 0,  0, -1, 0, 255,
                 0,  0,  0, 1, 0};
            f(c, 1536, 512, inverter);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_matrix_color_filter'><img
    src='https://fiddle.skia.org/i/@skpaint_matrix_color_filter_raster.png'></a>

*   Color Table Color Filter

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
            canvas->scale(0.5, 0.5);
            uint8_t ct[256];
            for (int i = 0; i < 256; ++i) {
                int x = (i - 96) * 255 / 64;
                ct[i] = x < 0 ? 0 : x > 255 ? 255 : x;
            }
            SkPaint paint;
          paint.setColorFilter(SkTableColorFilter::MakeARGB(nullptr, ct, ct, ct));
            canvas->drawBitmap(source, 0, 0, &paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_color_table_filter'><img
      src='https://fiddle.skia.org/i/@skpaint_color_table_filter_raster.png'></a>


<span id="SkPathEffect"></span>

SkPathEffect
------------

*   SkPath2DPathEffect: Stamp the specified path to fill the shape,
    using the matrix to define the latice.

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
          SkScalar scale = 10.0f;
          SkPath path;
            static const int8_t pts[] = { 2, 2, 1, 3, 0, 3, 2, 1, 3, 1,
                 4, 0, 4, 1, 5, 1, 4, 2, 4, 3, 2, 5, 2, 4, 3, 3, 2, 3 };
            path.moveTo(2 * scale, 3 * scale);
            for (size_t i = 0 ; i < sizeof(pts)/sizeof(pts[0]); i += 2) {
                path.lineTo(pts[i] * scale, pts[i + 1] * scale);
            }
            path.close();
            SkMatrix matrix = SkMatrix::MakeScale(4 * scale);
            SkPaint paint;
            paint.setPathEffect(SkPath2DPathEffect::Make(matrix, path));
            paint.setAntiAlias(true);
            canvas->clear(SK_ColorWHITE);
            SkRect bounds;
            (void)canvas->getClipBounds(&bounds);
            bounds.outset(2 * scale, 2 * scale);
            canvas->drawRect(bounds, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_path_2d_path_effect'><img
      src='https://fiddle.skia.org/i/@skpaint_path_2d_path_effect_raster.png'></a>

*   SkLine2DPathEffect: a special case of SkPath2DPathEffect where the
    path is a straight line to be stroked, not a path to be filled.

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
            SkPaint paint;
            SkMatrix lattice;
            lattice.setScale(8.0f, 8.0f);
            lattice.preRotate(30.0f);
            paint.setPathEffect(SkLine2DPathEffect::Make(0.0f, lattice));
            paint.setAntiAlias(true);
            SkRect bounds;
            (void)canvas->getClipBounds(&bounds);
            bounds.outset(8.0f, 8.0f);
            canvas->clear(SK_ColorWHITE);
            canvas->drawRect(bounds, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_line_2d_path_effect'><img
      src='https://fiddle.skia.org/i/@skpaint_line_2d_path_effect_raster.png'></a>

*   SkPath1DPathEffect: create dash-like effects by replicating the specified path along the drawn path.

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
            SkPaint paint;
            SkPath path;
            path.addOval(SkRect::MakeWH(16.0f, 6.0f));
            paint.setPathEffect(SkPath1DPathEffect::Make(
                    path, 32.0f, 0.0f, SkPath1DPathEffect::kRotate_Style));
            paint.setAntiAlias(true);
            canvas->clear(SK_ColorWHITE);
            canvas->drawCircle(128.0f, 128.0f, 122.0f, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_path_1d_path_effect'><img
      src='https://fiddle.skia.org/i/@skpaint_path_1d_path_effect_raster.png'></a>

*   SkArcToPathEffect

	The following few examples use this function:

    <!--?prettify lang=cc?-->

        SkPath star() {
            const SkScalar R = 115.2f, C = 128.0f;
            SkPath path;
            path.moveTo(C + R, C);
            for (int i = 1; i < 8; ++i) {
                SkScalar a = 2.6927937f * i;
                path.lineTo(C + R * cos(a), C + R * sin(a));
            }
            return path;
        }
        void draw(SkCanvas* canvas) {
            SkPaint paint;
            paint.setPathEffect(SkArcToPathEffect::Make(8.0f));
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setAntiAlias(true);
            canvas->clear(SK_ColorWHITE);
            SkPath path(star());
            canvas->drawPath(path, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_arc_to_path_effect'><img
      src='https://fiddle.skia.org/i/@skpaint_arc_to_path_effect_raster.png'></a>


*   SkCornerPathEffect: a path effect that can turn sharp corners into
    various treatments (e.g. rounded corners).

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
            SkPaint paint;
            paint.setPathEffect(SkCornerPathEffect::Make(32.0f));
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setAntiAlias(true);
            canvas->clear(SK_ColorWHITE);
            const SkScalar R = 115.2f;
            SkPath path(star());
            canvas->drawPath(path, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_corner_path_effects'><img src='https://fiddle.skia.org/i/@skpaint_corner_path_effects_raster.png'></a>

*   SkDashPathEffect:  a path effect that implements dashing.

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
            const SkScalar intervals[] = { 10.0f, 5.0f, 2.0f, 5.0f };
            size_t count  = sizeof(intervals) / sizeof(intervals[0]);
            SkPaint paint;
            paint.setPathEffect(SkDashPathEffect::Make(intervals, count, 0.0f));
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2.0f);
            paint.setAntiAlias(true);
            canvas->clear(SK_ColorWHITE);
            SkPath path(star());
            canvas->drawPath(path, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_dash_path_effect'><img src='https://fiddle.skia.org/i/@skpaint_dash_path_effect_raster.png'></a>

*   SkDiscretePathEffect: This path effect chops a path into discrete
    segments, and randomly displaces them.

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
            SkPaint paint;
            paint.setPathEffect(SkDiscretePathEffect::Make(10.0f, 4.0f));
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2.0f);
            paint.setAntiAlias(true);
            canvas->clear(SK_ColorWHITE);
            SkPath path(star());
            canvas->drawPath(path, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_discrete_path_effect'><img
      src='https://fiddle.skia.org/i/@skpaint_discrete_path_effect_raster.png'></a>

*   SkComposePathEffect: a pathEffect whose effect is to apply
    first the inner pathEffect and the the outer pathEffect (i.e.
    outer(inner(path))).

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
            const SkScalar intervals[] = { 10.0f, 5.0f, 2.0f, 5.0f };
            size_t count  = sizeof(intervals) / sizeof(intervals[0]);
            SkPaint paint;
            paint.setPathEffect(SkComposePathEffect::Make(
                SkDashPathEffect::Make(intervals, count, 0.0f),
                SkDiscretePathEffect::Make(10.0f, 4.0f)
            ));
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2.0f);
            paint.setAntiAlias(true);
            canvas->clear(SK_ColorWHITE);
            SkPath path(star());
            canvas->drawPath(path, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_compose_path_effect'><img
      src='https://fiddle.skia.org/i/@skpaint_compose_path_effect_raster.png'></a>

*    SkSumPathEffect: a pathEffect whose effect is to apply two effects,
     in sequence (i.e. first(path) + second(path)).

    <!--?prettify lang=cc?-->

        void draw(SkCanvas* canvas) {
            SkPaint paint;
            paint.setPathEffect(SkSumPathEffect::Make(
                SkDiscretePathEffect::Make(10.0f, 4.0f),
                SkDiscretePathEffect::Make(10.0f, 4.0f, 1245u)
            ));
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2.0f);
            paint.setAntiAlias(true);
            canvas->clear(SK_ColorWHITE);
            SkPath path(star());
            canvas->drawPath(path, paint);
        }

    <a href='https://fiddle.skia.org/c/@skpaint_sum_path_effect'><img
      src='https://fiddle.skia.org/i/@skpaint_sum_path_effect_raster.png'></a>

