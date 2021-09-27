---
title: 'SkSL & Runtime Effects'
linkTitle: 'SkSL'
---

## Overview

**SkSL** is Skia's
[shading language](https://en.wikipedia.org/wiki/Shading_language).
**`SkRuntimeEffect`** is a Skia C++ object that can be used to create
`SkShader`, `SkColorFilter`, and `SkBlender` objects with behavior controlled by
SkSL code.

You can experiment with SkSL at https://shaders.skia.org/. The syntax is very
similar to GLSL. When using SkSL effects in your Skia application, there are
important differences (from GLSL) to remember. Most of these differences are
because of one basic fact: **With GPU shading languages, you are programming a
stage of the
[GPU pipeline](https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview).
With SkSL, you are programming a stage of the Skia pipeline.**

In particular, a GLSL fragment shader controls the entire behavior of the GPU
between the rasterizer and the blending hardware. That shader does all of the
work to compute a color, and the color it generates is exactly what is fed to
the fixed-function blending stage of the pipeline.

SkSL effects exist as part of the larger Skia pipeline. When you issue a canvas
drawing operation, Skia (generally) assembles a single GPU fragment shader to do
all of the required work. This shader typically includes several pieces. For
example, it might include:

- Evaluating whether a pixel falls inside or outside of the shape being drawn
  (or on the border, where it might apply antialiasing).
- Evaluating whether a pixel falls inside or outside of the clipping region
  (again, with possible antialiasing logic for border pixels).
- Logic for the `SkShader` on the `SkPaint`. The `SkShader` can actually be a
  tree of objects (due to `SkShaders::Blend` and other features described
  below).
- Similar logic for the `SkColorFilter` (which can also be a tree, due to
  `SkColorFilters::Compose`, `SkColorFilters::Blend`, and features described
  below).
- Blending code (for certain `SkBlendMode`s, or for custom blending specified
  with `SkPaint::setBlender`).

Even if the `SkPaint` has a complex tree of objects in the `SkShader`,
`SkColorFilter`, or `SkBlender` fields, there is still only a _single_ GPU
fragment shader. Each node in that tree creates a single function. The clipping
code and geometry code each create a function. The blending code might create a
function. The overall fragment shader then calls all of these functions (which
may call other functions, e.g. in the case of an `SkShader` tree).

**Your SkSL effect contributes a function to the GPU's fragment shader.**

---

## Evaluating (sampling) other SkShaders

In GLSL, a fragment shader can sample a texture. With runtime effects, the
object that you bind (in C++) is an `SkShader`, represented by a `shader` in
SkSL. To make it clear that you are operating on an object that will emit its
own shader code, you don't use `sample`. Instead, the `shader` object has a
`.eval()` method. Regardless, Skia has simple methods for creating an `SkShader`
from an `SkImage`, so it's easy to use images in your runtime effects:

<fiddle-embed name='8a895f12c8fd7b976bb68e6002f85a8e'></fiddle-embed>

Because the object you bind and evaluate is an `SkShader`, you can directly use
any Skia shader, without necessarily turning it into an image (texture) first.
For example, you can evaluate a linear gradient. In this example, there is no
texture created to hold the gradient. Skia generates a single fragment shader
that computes the gradient color, samples from the image's texture, and then
multiplies the two together:

<fiddle-embed name='f282a4411782ed92057350e339586502'></fiddle-embed>

Of course, you can even invoke another runtime effect, allowing you to combine
shader snippets dynamically:

<fiddle-embed name='2151b061428f47844a2500b57c887ddf'></fiddle-embed>

---

## Premultiplied Alpha

When dealing with transparent colors, there are two (common)
[possible representations](https://en.wikipedia.org/wiki/Alpha_compositing#Straight_versus_premultiplied).
Skia calls these _unpremultiplied_ (what Wikipedia calls _straight_), and
_premultiplied_. In the Skia pipeline, every `SkShader` returns premultiplied
colors.

If you're familiar with OpenGL blending, you can think of it in terms of the
blend equation. For common alpha blending (called
[source-over](https://developer.android.com/reference/android/graphics/PorterDuff.Mode#SRC_OVER)),
you would normally configure your blend function as
`(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`. Skia defines source-over blending as
if the blend function were `(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)`.

Skia's use of premultiplied alpha implies:

- If you start with an unpremultiplied `SkImage` (like a PNG), turn that into an
  `SkImageShader`, and evaluate that shader... the resulting colors will be
  `[R*A, G*A, B*A, A]`, **not** `[R, G, B, A]`.
- If your SkSL will return transparent colors, it must be sure to multiply the
  `RGB` by `A`.
- For more complex shaders, you must understand which of your colors are
  premultiplied vs. unpremultiplied. Many operations don't make sense if you mix
  both kinds of color together.

The image below demonstrates this: properly premultiplied colors produce a smooth
gradient as alpha decreases. Unpremultipled colors cause the gradient to display
incorrectly, becoming too bright and shifting hue as the alpha changes.

<fiddle-embed name='4aa28e27a9682fec18d8c0ca265151ad'></fiddle-embed>

---

## Coordinate Spaces

To understand how coordinates work in SkSL, you first need to understand
[how they work in Skia](/docs/user/coordinates). If you're comfortable with
Skia's coordinate spaces, then just remember that the coordinates supplied to
your `main()` are **local** coordinates. They will be relative to the coordinate
space of the `SkShader`. This will match the local space of the canvas and any
`localMatrix` transformations. Additionally, if the shader is invoked by
another, that parent shader may modify them arbitrarily.

In addition, the `SkShader` produced from an `SkImage` does not use normalized
coordinates (like a texture in GLSL). It uses `(0, 0)` in the upper-left corner,
and `(w, h)` in the bottom-right corner. Normally, this is exactly what you
want. If you're evaluating an `SkImageShader` with coordinates based on the ones
passed to you, the scale is correct. However, if you want to adjust those
coordinates (to do some kind of re-mapping of the image), remember that the
coordinates are scaled up to the dimensions of the image:

<fiddle-embed name='cc49d5a7b6b88d6a4dca1619e6df8763'></fiddle-embed>
