SkCanvas Overview
========

*The drawing context*

<!-- Updated Mar 4, 2011 -->

Preview
-------

Here is an example of a set of drawing commands to draw a filled
heptagram.  This function can be cut and pasted into
[fiddle.skia.org](https://fiddle.skia.org/).

<fiddle-embed name='@skcanvas_star'></fiddle-embed>

Details
-------

SkCanvas is the drawing context for Skia. It knows where to direct the
drawing (i.e. where the screen of offscreen pixels are), and maintains
a stack of matrices and clips. Note however, that unlike similar
contexts in other APIs like postscript, cairo, or awt, Skia does not
store any other drawing attributes in the context (e.g. color, pen
size). Rather, these are specified explicitly in each draw call, via a
SkPaint.

<fiddle-embed name='@skcanvas_square'></fiddle-embed>

The code above will draw a rectangle rotated by 45 degrees. Exactly
what color and style the rect will be drawn in is described by the
paint, not the canvas.

Check out more detailed info on [creating a SkCanvas object](canvas).

To begin with, we might want to erase the entire canvas. We can do
this by drawing an enormous rectangle, but there are easier ways to do
it.

<!--?prettify lang=cc?-->

    void draw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        canvas->drawPaint(paint);
    }

This fills the entire canvas (though respecting the current clip of
course) with whatever color or shader (and xfermode) is specified by
the paint. If there is a shader in the paint, then it will respect the
current matrix on the canvas as well (see SkShader). If you just want
to draw a color (with an optional xfermode), you can just call
drawColor(), and save yourself having to allocate a paint.

<!--?prettify lang=cc?-->

    void draw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }

All of the other draw APIs are similar, each one ending with a paint
parameter.

<fiddle-embed name='@skcanvas_paint'></fiddle-embed>

In some of the calls, we pass a pointer, rather than a reference, to
the paint. In those instances, the paint parameter may be null. In all
other cases the paint parameter is required.

Next: [SkPaint](/user/api/skpaint_overview)
