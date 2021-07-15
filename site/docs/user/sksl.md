---
title: 'SkSL & Runtime Effects'
linkTitle: 'SkSL'
---

## <span id="overview">Overview</span>

**SkSL** is Skia's
[shading language](https://en.wikipedia.org/wiki/Shading_language).
**`SkRuntimeEffect`** is a Skia C++ object that can be used to create `SkShader`
and `SkColorFilter` objects with behavior controlled by SkSL code.

You can experiment with SkSL at https://shaders.skia.org/. The syntax is very
similar to GLSL. When using SkSL effects in your Skia application, there are
important differences (from GLSL) to remember. Most of these differences are
because of one basic fact: **With GPU shading languages, you are programming a
stage of the
[GPU pipeline](https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview).
With SkSL, you are programming a stage of the Skia pipeline.**

---

## <span id="children">Sampling other SkShaders</span>

In GLSL, a fragment shader can sample a texture. With runtime effects, the
object that you bind (in C++) and sample (in SkSL) is an `SkShader`. Skia has
simple methods for creating an `SkShader` from an `SkImage`, so it's easy to use
images in your runtime effects:

<fiddle-embed name='f2885d3af66e5589fab19017953ac59e'></fiddle-embed>

Because the object you bind and sample is an `SkShader`, you can directly use
any Skia shader, without necessarily turning it into an image (texture) first.
For example, you can sample a linear gradient:

<fiddle-embed name='5fa8d1ec3af346e7c16872d138598f87'></fiddle-embed>

You can even sample another runtime effect:

<fiddle-embed name='08745d5050b250c2d8a826a739ea0ee1'></fiddle-embed>

---

## <span id="premul">Premultiplied Alpha</span>

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
  `SkImageShader`, and sample that shader... the resulting colors will be
  `[R*A, G*A, B*A, A]`, **not** `[R, G, B, A]`.
- If your SkSL will return transparent colors, it must be sure to multiply the
  `RGB` by `A`.
- For more complex shaders, you must understand which of your colors are
  premultiplied vs. unpremultiplied. Many operations don't make sense if you mix
  both kinds of color together.

Skia _enforces_ that the color produced by your SkSL is a valid premultiplied
color. In other words, `RGB <= A`. If your SkSL returns colors where that is not
true, they will be clamped, leading to incorrect colors. The image below
demonstrates this: properly premultiplied colors produce a smooth gradient as
alpha decreases. Unpremultipled colors cause the gradient to display
incorrectly, with a shift in hue as the alpha changes. This hue shift is caused
by Skia clamping the color's RGB values to its alpha.

<fiddle-embed name='ceeb91dcae2274c5c0e8f76a45af0678'></fiddle-embed>

---

## <span id="coords">Coordinate Spaces</span>

To understand how coordinates work in SkSL, you first need to understand
[how they work in Skia](/docs/user/coordinates). If you're comfortable with Skia's coordinate
spaces, then just remember that the coordinates supplied to your `main()` are **local**
coordinates. They will be relative to the coordinate space of the `SkShader`. This will match the
local space of the canvas and any `localMatrix` transformations. Additionally, if the shader is
invoked by another, that parent shader may modify them arbitrarily.

In addition, the `SkShader` produced from an `SkImage` does not use normalized coordinates (like a
texture in GLSL). It uses `(0, 0)` in the upper-left corner, and `(w, h)` in the bottom-right
corner. Normally, this is exactly what you want. If you're sampling an `SkImageShader` with
coordinates based on the ones passed to you, the scale is correct. However, if you want to adjust
those coordinates (to do some kind of re-mapping of the image), remember that the coordinates are
scaled up to the dimensions of the image:

<fiddle-embed name='7ae4fd94835aa957aa6beb15f1774b6a'></fiddle-embed>
