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
- Color space conversion code, as part of Skia's [color management](/docs/user/color).

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

<fiddle-embed-sk name='a998917a6e0cdf5cb3a06fe9d9630af3'></fiddle-embed-sk>

Because the object you bind and evaluate is an `SkShader`, you can directly use
any Skia shader, without necessarily turning it into an image (texture) first.
For example, you can evaluate a linear gradient. In this example, there is no
texture created to hold the gradient. Skia generates a single fragment shader
that computes the gradient color, samples from the image's texture, and then
multiplies the two together:

<fiddle-embed-sk name='7085607d39028050efb91c0fe6131210'></fiddle-embed-sk>

Of course, you can even invoke another runtime effect, allowing you to combine
shader snippets dynamically:

<fiddle-embed-sk name='d2648c424e41cdd700330b7196283766'></fiddle-embed-sk>

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

<fiddle-embed-sk name='30a9eb3750ceaa3b88342938ee353071'></fiddle-embed-sk>

---

## Color Spaces

Applications using Skia are usually [color managed](/docs/user/color). The color
space of a surface (destination) determines the working color space for a draw.
Source content (like shaders, including `SkImageShader`) also have color spaces.
By default, inputs to your SkSL shader will be transformed to the working color
space. Some inputs require special care to get (or inhibit) this behavior, though.

First, let's see Skia's color management in action. Here, we're drawing a portion
of the mandrill image twice. The first time, we've drawn it normally, respecting
the color space stored in the file (this happens to be the [sRGB](https://en.wikipedia.org/wiki/SRGB)
color space. The second time, we've assigned the Rec. 2020 color space to the image.
This simply tells Skia to treat the image as if the colors stored are actually in
that color space. Skia then transforms those values from Rec. 2020 to the
destination surface's color space (sRGB). As a result, all of the colors look more
vivid. More importantly, if the image really *were* in some other color space, or
if the destination surface were in some other color space, this automatic conversion
is desirable, because it ensures content looks consistently correct on any user's
screen.

<fiddle-embed-sk name='555684b3908afcaaefd37d5f859ceb17'></fiddle-embed-sk>

### Uniforms

Skia and SkSL doesn't know if your `uniform` variables contain colors, so it won't
automatically apply color conversion to them. In the below example, there are two
uniforms declared: `color` and `not_a_color`. The SkSL simply fades in one of the
two uniform "colors" horizontally, choosing a different uniform for the top and
bottom half of the shader. The code passes the same values to both uniforms, four
floating point values `{1,0,0,1}` that represent "red".

To really see the effect of automatic uniform conversion, the fiddle draws to an
offscreen surface in the [Rec. 2020](https://en.wikipedia.org/wiki/Rec._2020) color
space. Rec. 2020 has a very _wide gamut_, which means that it can represent more
vivid colors than the common default [sRGB](https://en.wikipedia.org/wiki/SRGB)
color space. In particular, the purest red in sRGB is fairly dull compared to pure
red in Rec. 2020.

To understand what happens in this fiddle, we'll explain the steps for the two
different cases. For the top half, we use `not_a_color`. Skia and SkSL don't know
that you intend to use this as a color, so the raw floating point values you supply
are fed directly to the SkSL shader. In other words - when the SkSL executes,
`not_a_color` will contain `{1,0,0,1}`, regardless of the surface's color space.
This produces the most vivid red possible in the destination's color space (which
ends up looking like a very bright red in this case).

For the bottom half, we have declared the uniform `color` with the special syntax
`layout(color)`. That tells SkSL that this variable will be used as a color.
`layout(color)` can only be used on uniform values that are `vec3` (i.e., RGB) or
`vec4` (i.e., RGBA). In either case, the colors you supply when providing uniform data
should be unpremultiplied sRGB colors. Those colors can include values outside of
the range `[0,1]`, if you want to supply wide gamut colors. This is the same way
that Skia accepts and stores colors on `SkPaint`. When the SkSL executes, Skia
transforms the uniform value to the working color space. In this case, that means
that `color` (which starts out as sRGB red) is turned into whatever values represent
that same color in the Rec. 2020 color space.

The overall effect here is to make the correctly labeled uniform much duller, but
that is actually what you want when working with uniform colors. By labeling uniform
colors this way, your source colors (that you place in uniforms) will represent the
same, consistent color regardless of the color space of the destination surface.

<fiddle-embed-sk name='4f9365db3b8a52f767c57484eb6cad29'></fiddle-embed-sk>

### Raw Image Shaders

Although most images contain colors that should be color managed, some images
contain data that isn't actually colors. This includes images storing normals,
material properties (e.g., roughness), heightmaps, or any other purely
mathematical data that happens to be stored in an image. When using these kinds
of images in SkSL, you probably want to use a *raw* image shader, created with
`SkImage::makeRawShader`. These work like regular image shaders (including
filtering and tiling), with a few major differences:
  - No color space transformation is ever applied (the color space of the image
    is ignored).
  - Images with an alpha type of kUnpremul are **not** automatically premultiplied.
  - Bicubic filtering is not supported. Requesting bicubic filtering when
    calling `makeRawShader` will return `nullptr`.

Here, we create an image holding a spherical normal map. Then we use that with
a lighting shader to show what happens when rendering to a different color space.
If we use a regular image shader, the normals will be treated as colors, and
transformed to the working color space. This alters the normals, incorrectly.
For the final draw, we use a raw image shader, which returns the original
normals, ignoring the working color space.

<fiddle-embed-sk name='d91814fd4dc87a46c0ab4a0587260718'></fiddle-embed-sk>

### Working In a Known Color Space

Within an SkSL shader, you don't know what the working color space is. For many
effects, this is fine - evaluating image shaders, and doing simple color math
is usually going to give reasonable results (particularly if you know that
the working color space for an application is always sRGB, for example). For
certain effects, though, it may be important to do some math in a fixed, known
color space. The most common example is lighting -- to get physically accurate
lighting, math should be done in a _linear_ color space. To help with this,
SkSL provides two intrinsic functions:

```
vec3 toLinearSrgb(vec3 color);
vec3 fromLinearSrgb(vec3 color);
```

These convert colors between the working color space and the linear sRGB color
space. That space uses the sRGB color primaries (gamut), and a linear transfer
function. It represents values outside of the sRGB gamut using extended range
values (below 0.0 and above 1.0). This corresponds to Android's
[LINEAR_EXTENDED_SRGB](https://developer.android.com/reference/android/graphics/ColorSpace.Named.html#LINEAR_EXTENDED_SRGB)
or Apple's
[extendedLinearSRGB](https://developer.apple.com/documentation/coregraphics/cgcolorspace/1690961-extendedlinearsrgb),
for example.

Here's an example showing a sphere, with lighting math being done in the default
working space (sRGB), and again with the math done in a linear space:

<fiddle-embed-sk name='9e52c269cf3d47e5957e74d21288dd59'></fiddle-embed-sk>

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

<fiddle-embed-sk name='91a25662aab86b212b946cb4df8b12ca'></fiddle-embed-sk>

---

## Minified SkSL

Skia includes a minifier tool which can automatically reduce the size of your Runtime Effect
or SkMesh code. The tool eliminates whitespace and comments, shortens function and variable names,
and deletes unreferenced code.

As an example, here is the previous demo in its minified form. The shader code is reduced to
approximately half of its original size, while displaying the exact same result.

<fiddle-embed-sk name='a6edbedab51be34f74b15c26e276ecc1'></fiddle-embed-sk>

To enable this tool, add `skia_compile_modules = true` to your gn argument list. (At the command
line, type `gn args out/yourbuild` to access the arguments, or edit the file `out/yourbuild/args.gn`
directly.) Use `ninja` to compile Skia once more, and you will now have a new utility called
`sksl-minify` in the output directory.

When minifying a mesh program, you must supply `struct Varyings` and `struct Attributes` which
correspond to the SkMeshSpecification. These structs will be eliminated from the minified program
for convenience.

`sksl-minify` takes the following command line arguments:

- An output path, e.g. `MyShader.minified.sksl`
- An input path, e.g. `MyShader.sksl`
- (Optional) Pass `--stringify` to wrap the minified SkSL text in a quoted C++ string.
  By default, the output file will contain plain SkSL. The minified shader string in the example
  code above was created with --stringify.
- (Optional) Pass `--shader`, `--colorfilter`, `--blender`, `--meshfrag` or `--meshvert` to set
  the program kind. The default value is `--shader`.
