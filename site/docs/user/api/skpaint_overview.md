---
title: 'SkPaint Overview'
linkTitle: 'SkPaint Overview'

weight: 280
---

Anytime you draw something in Skia, and want to specify what color it is, or how
it blends with the background, or what style or font to draw it in, you specify
those attributes in a paint.

Unlike `SkCanvas`, paints do not maintain an internal stack of state (i.e. there
is no save/restore on a paint). However, paints are relatively light-weight, so
the client may create and maintain any number of paint objects, each set up for
a particular use. Factoring all of these color and stylistic attributes out of
the canvas state, and into (multiple) paint objects, allows canvas' save/restore
to be that much more efficient, as all they have to do is maintain the stack of
matrix and clip settings.

<fiddle-embed-sk name='@skpaint_skia'></fiddle-embed-sk>

This shows three different paints, each set up to draw in a different style. Now
the caller can intermix these paints freely, either using them as is, or
modifying them as the drawing proceeds.

<fiddle-embed-sk name='@skpaint_mix'></fiddle-embed-sk>

Beyond simple attributes such as color, strokes, and text values, paints support
effects. These are subclasses of different aspects of the drawing pipeline, that
when referenced by a paint (each of them is reference-counted), are called to
override some part of the drawing pipeline.

For example, to draw using a gradient instead of a single color, assign a
SkShader to the paint.

<fiddle-embed-sk name='@skpaint_shader'></fiddle-embed-sk>

Now, anything drawn with that paint will be drawn with the gradient specified in
the call to `MakeLinear()`. The shader object that is returned is
reference-counted. Whenever any effects object, like a shader, is assigned to a
paint, its reference-count is increased by the paint. To balance this, the
caller in the above example calls `unref()` on the shader once it has assigned
it to the paint. Now the paint is the only "owner" of that shader, and it will
automatically call `unref()` on the shader when either the paint goes out of
scope, or if another shader (or null) is assigned to it.

There are 6 types of effects that can be assigned to a paint:

- **SkPathEffect** - modifications to the geometry (path) before it generates an
  alpha mask (e.g. dashing)
- **SkRasterizer** - composing custom mask layers (e.g. shadows)
- **SkMaskFilter** - modifications to the alpha mask before it is colorized and
  drawn (e.g. blur)
- **SkShader** - e.g. gradients (linear, radial, sweep), bitmap patterns (clamp,
  repeat, mirror)
- **SkColorFilter** - modify the source color(s) before applying the xfermode
  (e.g. color matrix)
- **SkXfermode** - e.g. porter-duff transfermodes, blend modes

Paints also hold a reference to a SkTypeface. The typeface represents a specific
font style, to be used for measuring and drawing text. Speaking of which, paints
are used not only for drawing text, but also for measuring it.

<!--?prettify lang=cc?-->

    paint.measureText(...);
    paint.getTextBounds(...);
    paint.textToGlyphs(...);
    paint.getFontMetrics(...);

## SkXfermode

The following example demonstrates all of the Skia's standard transfer modes. In
this example the source is a solid magenta color with a horizontal alpha
gradient and the destination is a solid cyan color with a vertical alpha
gradient.

<fiddle-embed-sk name='@skpaint_xfer'></fiddle-embed-sk>

## SkShader

Several shaders are defined (besides the linear gradient already mentioned):

- Bitmap Shader

  <fiddle-embed-sk name='@skpaint_bitmap_shader'></fiddle-embed-sk>

- Radial Gradient Shader

  <fiddle-embed-sk name='@skpaint_radial'></fiddle-embed-sk>

- Two-Point Conical Gradient Shader

  <fiddle-embed-sk name='@skpaint_2pt'></fiddle-embed-sk>

- Sweep Gradient Shader

  <fiddle-embed-sk name='@skpaint_sweep'></fiddle-embed-sk>

- Fractal Perlin Noise Shader

  <fiddle-embed-sk name='@skpaint_perlin'></fiddle-embed-sk>

- Turbulence Perlin Noise Shader

  <fiddle-embed-sk name='@skpaint_turb'></fiddle-embed-sk>

- Compose Shader

  <fiddle-embed-sk name='@skpaint_compose_shader'></fiddle-embed-sk>

## SkMaskFilter

- Blur Mask Filter

  <fiddle-embed-sk name='@skpaint_blur_mask_filter'></fiddle-embed-sk>

## SkColorFilter

- Color Matrix Color Filter

  <fiddle-embed-sk name='@skpaint_matrix_color_filter'></fiddle-embed-sk>

- Color Table Color Filter

  <fiddle-embed-sk name='@skpaint_color_table_filter'></fiddle-embed-sk>

## SkPathEffect

- SkPath2DPathEffect: Stamp the specified path to fill the shape, using the
  matrix to define the latice.

  <fiddle-embed-sk name='@skpaint_path_2d_path_effect'></fiddle-embed-sk>

- SkLine2DPathEffect: a special case of SkPath2DPathEffect where the path is a
  straight line to be stroked, not a path to be filled.

  <fiddle-embed-sk name='@skpaint_line_2d_path_effect'></fiddle-embed-sk>

- SkPath1DPathEffect: create dash-like effects by replicating the specified path
  along the drawn path.

  <fiddle-embed-sk name='@skpaint_path_1d_path_effect'></fiddle-embed-sk>

- SkCornerPathEffect: a path effect that can turn sharp corners into various
  treatments (e.g. rounded corners).

  <fiddle-embed-sk name='@skpaint_corner_path_effects'></fiddle-embed-sk>

- SkDashPathEffect: a path effect that implements dashing.

  <fiddle-embed-sk name='@skpaint_dash_path_effect'></fiddle-embed-sk>

- SkDiscretePathEffect: This path effect chops a path into discrete segments,
  and randomly displaces them.

  <fiddle-embed-sk name='@skpaint_discrete_path_effect'></fiddle-embed-sk>

- SkComposePathEffect: a pathEffect whose effect is to apply first the inner
  pathEffect and the the outer pathEffect (i.e. outer(inner(path))).

  <fiddle-embed-sk name='@skpaint_compose_path_effect'></fiddle-embed-sk>

- SkSumPathEffect: a pathEffect whose effect is to apply two effects, in
  sequence (i.e. first(path) + second(path)).

  <fiddle-embed-sk name='@skpaint_sum_path_effect'></fiddle-embed-sk>
