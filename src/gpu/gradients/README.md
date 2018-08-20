Gradients on the GPU
====================

Gradients can be thought of, at a very high level, as two pieces:

1. A color interpolator that is one dimensional, returning a color for an input
   within the range [0.0, 1.0]. This obfuscates the the definition of specific
   color stops and how to wrap, tile, or clamp out of bound inputs.
2. A layout that converts from 2D geometry/position to the one dimensional
   domain of the color interpolator. This is how a linear or radial gradient
   distinguishes itself. When designing a new gradient, this is the component
   that you have to implement.


GrGradientEffect is a master effect that composes a color interpolator and a
layout, both of which are represented as child fragment processors. A layout
will generally be named GrXGradientLayout and a color interpolator will be
named GrYGradientColorizer. Currently there are color interpolators
(colorizers) for analytic color cases (evaluated directly on the GPU) and
sampling a generated texture map.
