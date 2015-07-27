SkPaint
=======

*color, stroke, font, effects*

Anytime you draw something in Skia, and want to specify what color it
is, or how it blends with the background, or what style or font to
draw it in, you specify those attributes in a paint.

Unlike `SkCanvas`, paints do not maintain an internal stack of state
(i.e. there is no save/restore on a paint). However, paints are
relatively light-weight, so the client may create and maintain any
number of paint objects, each set up for a particular use. Factoring
all of these color and stylistic attribute out of the canvas state,
and into (multiple) paint objects, allows canvas' save/restore to be
that much more efficient, as all they have to do is maintain the stack
of matrix and clip settings.

<!--?prettify lang=cc?-->

    void draw(SkCanvas* canvas) {
        canvas->clear(SK_ColorWHITE);

        SkPaint paint1, paint2, paint3;

        paint1.setTextSize(64.0f);
        paint1.setAntiAlias(true);
        paint1.setColor(0xFFFF0000);
        paint1.setStyle(SkPaint::kFill_Style);

        paint2.setTextSize(64.f);
        paint2.setAntiAlias(true);
        paint2.setColor(0xFF008800);
        paint2.setStyle(SkPaint::kStroke_Style);
        paint2.setStrokeWidth(SkIntToScalar(3));

        paint3.setTextSize(64.0f);
        paint3.setAntiAlias(true);
        paint3.setColor(0xFF888888);
        paint3.setTextScaleX(SkFloatToScalar(1.5f));

        const char text[] = "Skia!";
        canvas->drawText(text, strlen(text), 20.0f, 64.0f,  paint1);
        canvas->drawText(text, strlen(text), 20.0f, 144.0f, paint2);
        canvas->drawText(text, strlen(text), 20.0f, 224.0f, paint3);
    }

<a href="https://fiddle.skia.org/c/b8e7991ede1ca88e5458aa1f0039caf9">
<img src="https://fiddle.skia.org/i/b8e7991ede1ca88e5458aa1f0039caf9_raster.png"></a>

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

<a href="https://fiddle.skia.org/c/5203b17103f487dd33965b4211d80956">
<img src="https://fiddle.skia.org/i/5203b17103f487dd33965b4211d80956_raster.png"></a>

Beyond simple attributes such as color, strokes, and text values,
paints support effects. These are subclasses of different aspects of
the drawing pipeline, that when referenced by a paint (each of them is
reference-counted), are called to override some part of the drawing
pipeline.

For example, to draw using a gradient instead of a single color,
assign a SkShader to the paint.

<!--?prettify lang=cc?-->

    SkPoint points[2] = {
        SkPoint::Make(0.0f, 0.0f),
        SkPoint::Make(256.0f, 256.0f)
    };
    SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
    SkShader* shader =
            SkGradientShader::CreateLinear(
                     points, colors, NULL, 2,
                     SkShader::kClamp_TileMode, 0, NULL);
    SkPaint paint;
    paint.setShader(shader);
    shader->unref();
    canvas->drawPaint(paint);

<a href="https://fiddle.skia.org/c/f91b5310d57744a5a1475b7e47d4a172">
<img src="https://fiddle.skia.org/i/f91b5310d57744a5a1475b7e47d4a172_raster.png"></a>

Now, anything drawn with that paint will be drawn with the gradient
specified in the call to `CreateLinear()`. The shader object that is
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

ShShader
--------

Several shaders are defined (besides the linear gradient already mentioned):

*   Bitmap Shader

    <!--?prettify lang=cc?-->

        canvas->clear(SK_ColorWHITE);
        SkMatrix matrix;
        matrix.setScale(0.75f, 0.75f);
        matrix.preRotate(30.0f);
        SkShader* shader =
            SkShader::CreateBitmapShader(
                source,
                SkShader::kRepeat_TileMode ,
                SkShader::kRepeat_TileMode ,
                &matrix);
        SkPaint paint;
        paint.setShader(shader);
        shader->unref();
        canvas->drawPaint(paint);

    <a href="https://fiddle.skia.org/c/0e8d08e0a0b342e9e45c364d0e5cea8a">
    <img src="https://fiddle.skia.org/i/0e8d08e0a0b342e9e45c364d0e5cea8a_raster.png"></a>

*   Radial Gradient Shader

    <!--?prettify lang=cc?-->

        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkShader* shader =
                SkGradientShader::CreateRadial(
                        SkPoint::Make(128.0f, 128.0f), 180.0f,
                        colors, NULL, 2, SkShader::kClamp_TileMode, 0, NULL);
        SkPaint paint;
        paint.setShader(shader);
        shader->unref();
        canvas->drawPaint(paint);

    <a href="https://fiddle.skia.org/c/601abd2819e38365900bf62286986024">
    <img src="https://fiddle.skia.org/i/601abd2819e38365900bf62286986024_raster.png"></a>

*  Two-Point Conical Gradient Shader

    <!--?prettify lang=cc?-->

        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkShader* shader =
                SkGradientShader::CreateTwoPointConical(
                         SkPoint::Make(128.0f, 128.0f), 128.0f,
                         SkPoint::Make(128.0f, 16.0f), 16.0f,
                         colors, NULL, 2, SkShader::kClamp_TileMode, 0, NULL);
        SkPaint paint;
        paint.setShader(shader);
        shader->unref();
        canvas->drawPaint(paint);

    <a href="https://fiddle.skia.org/c/991f7d67ff1ccebd6c2c4fab18a76edc">
    <img src="https://fiddle.skia.org/i/991f7d67ff1ccebd6c2c4fab18a76edc_raster.png"></a>


*   Sweep Gradient Shader

    <!--?prettify lang=cc?-->

        SkColor colors[4] = {
            SK_ColorCYAN, SK_ColorMAGENTA, SK_ColorYELLOW, SK_ColorCYAN};
        SkShader* shader =
                SkGradientShader::CreateSweep(
                    128.0f, 128.0f, colors, NULL, 4, 0, NULL);
        SkPaint paint;
        paint.setShader(shader);
        shader->unref();
        canvas->drawPaint(paint);

    <a href="https://fiddle.skia.org/c/cee9d1831f6679c3d88170f857995d12">
    <img src="https://fiddle.skia.org/i/cee9d1831f6679c3d88170f857995d12_raster.png"></a>

*   Fractal Perlin Noise Shader

    <!--?prettify lang=cc?-->

        canvas->clear(SK_ColorWHITE);
        SkShader* shader = SkPerlinNoiseShader::CreateFractalNoise(
                 0.05f, 0.05f, 4, 0.0f, NULL);
        SkPaint paint;
        paint.setShader(shader);
        shader->unref();
        canvas->drawPaint(paint);

    <a href="https://fiddle.skia.org/c/cc45c5349c3b31f97da7c1af7f84162a">
    <img src="https://fiddle.skia.org/i/cc45c5349c3b31f97da7c1af7f84162a_raster.png"></a>

*   Turbulence Perlin Noise Shader

    <!--?prettify lang=cc?-->

        canvas->clear(SK_ColorWHITE);
        SkShader* shader = SkPerlinNoiseShader::CreateTurbulence(
                 0.05f, 0.05f, 4, 0.0f, NULL);
        SkPaint paint;
        paint.setShader(shader);
        shader->unref();
        canvas->drawPaint(paint);

    <a href="https://fiddle.skia.org/c/52729ed3a71b89a6dba4f60e8eb67727">
    <img src="https://fiddle.skia.org/i/52729ed3a71b89a6dba4f60e8eb67727_raster.png"></a>

*   Compose Shader

    <!--?prettify lang=cc?-->

        SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
        SkShader* shader1 =
                SkGradientShader::CreateRadial(
                    SkPoint::Make(128.0f, 128.0f), 180.0f,
                    colors, NULL, 2, SkShader::kClamp_TileMode, 0, NULL);
        SkShader* shader2 = SkPerlinNoiseShader::CreateTurbulence(
                 0.025f, 0.025f, 2, 0.0f, NULL);
        SkShader* shader =
            new SkComposeShader(shader1, shader2);
        SkPaint paint;
        paint.setShader(shader);
        shader->unref();
        shader2->unref();
        shader1->unref();
        canvas->drawPaint(paint);

    <a href="https://fiddle.skia.org/c/1209b7a29d870302edcc43dc0916e8d5">
    <img src="https://fiddle.skia.org/i/1209b7a29d870302edcc43dc0916e8d5_raster.png"></a>


SkMaskFilter
------------

*   Blur Mask Filter

    <!--?prettify lang=cc?-->

        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(120);
        SkMaskFilter* filter =
            SkBlurMaskFilter::Create(
                kNormal_SkBlurStyle, 5.0f, 0);
        paint.setMaskFilter(filter);
        filter->unref();
        const char text[] = "Skia";
        canvas->drawText(text, strlen(text), 0, 160, paint);

    <a href="https://fiddle.skia.org/c/0e004664122851691d67a291007b64d7">
    <img src="https://fiddle.skia.org/i/0e004664122851691d67a291007b64d7_raster.png"></a>

*   Emboss Mask Filter

    <!--?prettify lang=cc?-->

        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(120);
        SkScalar direction[3] = {1.0f, 1.0f, 1.0f};
        SkMaskFilter* filter =
            SkBlurMaskFilter::CreateEmboss(
                2.0f, direction, 0.3f, 0.1f);
        paint.setMaskFilter(filter);
        filter->unref();
        const char text[] = "Skia";
        canvas->drawText(text, strlen(text), 0, 160, paint);

    <a href="https://fiddle.skia.org/c/1ef71be7fb749a2d81a55721b2d2c77d">
    <img src="https://fiddle.skia.org/i/1ef71be7fb749a2d81a55721b2d2c77d_raster.png"></a>


SkColorFilter
-------------

*   Color Matrix Color Filter

    <!--?prettify lang=cc?-->

        void f(SkCanvas* c, SkScalar x, SkScalar y, SkScalar colorMatrix[20]) {
            SkColorFilter* cf = SkColorMatrixFilter::Create(colorMatrix);
            SkPaint paint;
            paint.setColorFilter(cf);
            cf->unref();
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

    <a href="https://fiddle.skia.org/c/91fb5341ee7903c9682df20bb3d73dbb">
    <img src="https://fiddle.skia.org/i/91fb5341ee7903c9682df20bb3d73dbb_raster.png"></a>

*   Color Table Color Filter

    <!--?prettify lang=cc?-->

        canvas->scale(0.5, 0.5);
        uint8_t ct[256];
        for (int i = 0; i < 256; ++i) {
            int x = (i - 96) * 255 / 64;
            ct[i] = x < 0 ? 0 : x > 255 ? 255 : x;
        }
        SkColorFilter* cf = SkTableColorFilter::CreateARGB(NULL, ct, ct, ct);
        SkPaint paint;
        paint.setColorFilter(cf);
        cf->unref();
        canvas->drawBitmap(source, 0, 0, &paint);

    <a href="https://fiddle.skia.org/c/0d3d339543afa1b10c60f9826f264c3f">
    <img src="https://fiddle.skia.org/i/0d3d339543afa1b10c60f9826f264c3f_raster.png"></a>
