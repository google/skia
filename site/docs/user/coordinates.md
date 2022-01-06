---
title: 'Skia Coordinate Spaces'
linkTitle: 'Coordinates'
---

## Overview

Skia generally refers to two different coordinate spaces: **device** and
**local**. Device coordinates are defined by the surface (or other device) that
you're rendering to. They range from `(0, 0)` in the upper-left corner of the
surface, to `(w, h)` in the bottom-right corner - they are effectively measured
in pixels.

---

## Local Coordinates

The local coordinate space is how all geometry and shaders are supplied to the
`SkCanvas`. By default, the local and device coordinate systems are the same.
This means that geometry is typically specified in pixel units. Here, we
position a rectangle at `(100, 50)`, and specify that it is `50` units wide and
tall:

<fiddle-embed-sk name='96f782b723c5240aab440242f4c7cbfb'></fiddle-embed-sk>

Local coordinates are also used to define and evaluate any `SkShader` on the
paint. Here, we define a linear gradient shader that goes from green (when
`x == 0`) to blue (when `x == 50`):

<fiddle-embed-sk name='97cf81a465fdeff01d2298e07a0802a3'></fiddle-embed-sk>

---

## Shaders Do Not Move With Geometry

Now, let's try to draw the gradient-filled square at `(100, 50)`:

<fiddle-embed-sk name='3adc73d23d57084f954f52c6b14c8772'></fiddle-embed-sk>

What happened? Remember, the local coordinate space has not changed. The origin
is still in the upper-left corner of the surface. We have specified that the
geometry should be positioned at `(100, 50)`, but the `SkShader` is still
producing a gradient as `x` goes from `0` to `50`. We have slid the rectangle
across the gradient defined by the `SkShader`. Shaders do not move with the
geometry.

---

## Transforming Local Coordinate Space

To get the desired effect, we could create a new gradient shader, with the
positions moved to `100` and `150`. That makes our shaders difficult to reuse.
Instead, we can use methods on `SkCanvas` to **change the local coordinate
space**. This causes all local coordinates (geometry and shaders) to be
evaluated in the new space defined by the canvas' transformation matrix:

<fiddle-embed-sk name='ce89b326b2bbe41587eec738706bf155'></fiddle-embed-sk>

---

## <span>Transforming Shader Coordinate Space</span>

Finally, it is possible to transform the coordinate space of the `SkShader`,
relative to the canvas local coordinate space. To do this, you supply a
`localMatrix` parameter when creating the `SkShader`. In this situation, the
geometry is transformed by the `SkCanvas` matrix. The `SkShader` is transformed
by the `SkCanvas` matrix **and** the `localMatrix` for that shader. The other
way to think about this: The `localMatrix` defines a transform that maps the
shader's coordinates to the coordinate space of the geometry.

To help illustrate the difference, here's our gradient-filled box. It's first
been translated `50` units over and down. Then, we apply a `45` degree rotation
(pivoting on the center of the box) to the canvas. This rotates the geometry of
the box, and the gradient inside it:

<fiddle-embed-sk name='d4b52d94342f1b55900d489c7ba8fd21'></fiddle-embed-sk>

Compare that to the second example. We still translate `50` units over and down.
Here, though, we apply the `45` degree rotation _only to the shader_, by
specifying it as a `localMatrix` to the `SkGradientShader::MakeLinear` function.
Now, the box remains un-rotated, but the gradient rotates inside the box:

<fiddle-embed-sk name='886fa46943b67e0d6aa78486dcfbcc2c'></fiddle-embed-sk>
