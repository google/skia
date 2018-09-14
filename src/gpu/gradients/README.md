Gradients on the GPU
====================

Gradients can be thought of, at a very high level, as three pieces:

1. A color interpolator that is one dimensional, returning a color for an input
   within the range [0.0, 1.0]. This obfuscates the the definition of specific
   color stops and how to wrap, tile, or clamp out of bound inputs. A color
   interpolator will be named GrYGradientColorizer
2. A layout that converts from 2D geometry/position to the one dimensional
   domain of the color interpolator. This is how a linear or radial gradient
   distinguishes itself. When designing a new gradient, this is the component
   that you have to implement. A layout will generally be named
   GrXGradientLayout
3. A master effect that composes the layout and color interpolator together. It
   is also responsible for implementing the clamping behavior that can be
   abstracted away from both the layout and colorization.


GrClampedGradientEffect handles clamped and decal tile modes, while
GrTiledGradientEffect implements repeat and mirror tile modes. The
GrClampedGradientEffect requires border colors to be specified outside of its
colorizer child, but these border colors may be defined by the gradient color
stops. Both of these master effects delegate calculating a t interpolant to a
child process, perform their respective tile mode operations, and possibly
convert the tiled t value (guaranteed to be within 0 and 1) into an output
color using their child colorizer process.

Because of how child processors are currently defined, where they have a single
half4 input and a single half4 output, their is a type mismatch between the 1D
t value and the 4D inputs/outputs of the layout and colorizer processes. For
now, the master effect assumes an untiled t is output in sk_OutColor.x by the
layout and it tiles solely off of that value.

However, layouts can output a negative value in the y component to invalidate
the gradient location (currently on the two point conical gradient does this).
When invalidated, the master effect outputs transparent black and does not
invoke the child processor. Other than this condition, any value in y, z, or w
are passed into the colorizer unmodified. The colorizer should assume that the
valid tiled t value is in sk_InColor.x and can safely ignore y, z, and w.

Currently there are color interpolators (colorizers) for analytic color cases
(evaluated directly on the GPU) and sampling a generated texture map.

GrGradientShader provides static factory functions to create
GrFragmentProcessor graphs that reproduce a particular SkGradientShader.

Optimization Flags
==================

At an abstract level, gradient shaders are compatible with coverage as alpha
and, under certain conditions, preserve opacity when the inputs are opaque. To
reduce the amount of duplicate code and boilerplate, these optimization
decisions are implemented in the master effects and not in the colorizers. It
is assumed that all colorizer FPs will be compatible with coverage as alpha and
will preserve opacity if input colors are opaque. Since this is assumed by the
master effects, they do not need to report these optimizations or check input
opacity (this does mean if the colorizers are used independently from the
master effect shader that the reported flags might not be optimal, but since
that is unlikely, this convention really simplifies the colorizer
implementations).

Unlike colorizers, which do not need to report any optimization flags, layout
FPs should report opacity preserving optimizations because they can impact the
opacity of a pixel outside of how the gradient would otherwise color it.
Layouts that potentially reject pixels (i.e. could output a negative y value)
must not report kPreservesOpaqueInput_OptimizationFlag. Layouts that never
reject a pixel should report kPreservesOpaqueInput_OptimizationFlag since the
master effects can optimize away checking if the layout rejects a pixel.


