SkPaint
=======

*color, stroke, font, effects*

<!-- Updated Jan 17, 2013 by humper@google.com -->

Anytime you draw something in Skia, and want to specify what color it
is, or how it blends with the background, or what style or font to
draw it in, you specify those attributes in a paint.

Unlike `SkCanvas`, paints do not maintain an internal stack of state
(i.e. there is no save/restore on a paint). However, paints are
relatively light-weight, so the client may create and maintain any
number of paint objects, each setup for a particular use. Factoring
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

This shows three different paints, each setup to draw in a different
style. Now the caller can intermix these paints freely, either using
them as is, or modifying them as the drawing proceeds.

<!--?prettify lang=cc?-->

    canvas->drawRect(..., paint1);
    canvas->drawRect(..., paint2);

    paint2.setStrokeWidth(SkIntToScalar(5));
    canvas->drawOval(..., paint2);

    canvas->drawText(..., paint3);
    paint3.setColor(0xFF0000FF);
    canvas->drawText(..., paint3);


Beyond simple attributes such as color, strokes, and text values,
paints support effects. These are subclasses of different aspects of
the drawing pipeline, that when referenced by a paint (each of them is
reference-counted), are called to override some part of the drawing
pipeline.

For example, to draw using a gradient instead of a single color,
assign a SkShader to the paint.

<!--?prettify lang=cc?-->

    SkShader* shader = SkGradientShader::CreateLinear(...);
    paint.setShader(shader);
    shader->unref();

Now, anything drawn with that paint will be drawn with the gradient
specified in the call to CreateLinear(). The shader object that is
returned is reference-counted. Whenever any effects object, like a
shader, is assigned to a paint, its reference-count is increased by
the paint. To balance this, the caller in the above example calls
unref() on the shader once it has assigned it to the paint. Now the
paint is the only "owner" of that shader, and it will automatically
call unref() on the shader when either the paint goes out of scope, or
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
