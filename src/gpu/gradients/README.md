Gradients on the GPU
====================

Gradients can be thought of, at a very high level, as three pieces:

1. A color interpolator ("colorizer") that is one dimensional, returning a color
   for an interpolant value "t" within the range [0.0, 1.0]. This encapsulates
   the definition of specific color stops and how to wrap, tile, or clamp out
   of bound inputs. A color interpolator will be named `GrXxxxxGradientColorizer`.
2. A layout that converts from 2D geometry/position to the one dimensional
   domain of the color interpolator. This is how a linear or radial gradient
   distinguishes itself. When designing a new gradient shape, this is the
   component that will need to be implemented. A layout will generally be
   named `GrYyyyyGradientLayout`.
3. A top-level effect that composes the layout and color interpolator together.
   This processor is also responsible for implementing the clamping behavior,
   which is abstracted away from both the layout and colorization.

`GrClampedGradientEffect` handles clamped and decal tile modes, while
`GrTiledGradientEffect` implements repeat and mirror tile modes. The
`GrClampedGradientEffect` requires border colors to be specified outside of its
colorizer child, but these border colors may be defined by the gradient color
stops. Both of these top-level effects delegate calculating the t interpolant to
the layout child processor, then perform their respective tile mode operations,
and finally convert the tiled t value (guaranteed to be within 0 and 1) into an
output color using the colorizer child processor.

Fragment processors only support returning colors; conceptually, however,
layout processors need to generate an interpolant, not a color. So the
layout processor encodes its result into a color as follows:

-  `sk_OutColor.r`: computed t interpolant [0.0, 1.0], untiled
-  `sk_OutColor.g`: Positive value = render, negative value = discard pixel.
-  `sk_OutColor.b`: unused
-  `sk_OutColor.a`: unused

Layouts can report "invalid gradient location" by outputting a negative value
into the `sk_OutColor.g` component. (Currently, the two-point conical gradient
does this.) When this happens, the top-level effect immediately returns transparent
black and does not invoke the colorizer at all. When the gradient location is valid,
the top-level effect samples from the colorizer at the explicit coordinate (t, 0). The
y coordinate will always be zero and can be ignored by the colorizer.

There are several hand-written colorizers for analytic color cases; these are
evaluated directly in the shader. Generated texture maps can also be used to
colorize a gradient; in this case, a `GrTextureEffect`  will be used as the colorizer.

`GrGradientShader` provides static factory functions to create
`GrFragmentProcessor` graphs that reproduce a particular `SkGradientShader`.

Optimization Flags
==================

At an abstract level, gradient shaders are compatible with coverage as alpha
and, under certain conditions, preserve opacity when the inputs are opaque. To
reduce the amount of duplicate code and boilerplate, these optimization
decisions are implemented in the top-level effects and not in the colorizers. It
is assumed that all colorizer FPs will be compatible with coverage as alpha and
will preserve opacity if input colors are opaque. Since this is assumed by the
top-level effects, they do not need to report these optimizations or check input
opacity (this does mean if the colorizers are used independently from the
top-level effect shader that the reported flags might not be optimal, but since
that is unlikely, this convention really simplifies the colorizer
implementations).

Unlike colorizers, which do not need to report any optimization flags, layout
FPs should report opacity preserving optimizations because they can impact the
opacity of a pixel outside of how the gradient would otherwise color it.
Layouts that potentially reject pixels (i.e. could output a negative y value)
must not report kPreservesOpaqueInput_OptimizationFlag. Layouts that never
reject a pixel should report kPreservesOpaqueInput_OptimizationFlag since the
top-level effects can optimize away checking if the layout rejects a pixel.
