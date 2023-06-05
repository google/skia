`SkShaders` is now a namespace (was previously a non-constructable class with only static
functions). `SkPerlinNoiseShader::MakeFractalNoise` and `SkPerlinNoiseShader::MakeTurbulence` have
been moved to the `SkShaders` namespace and `SkPerlinNoiseShader` (the public non-constructable
class) has been slated for moving into private internals of Skia.
There are no functional differences in the moved functions, however the change of some #includes
in `include/core/SkShader.h`, `include/effects/SkGradientShader.h`, and
`include/effects/SkPerlinNoiseShader.h` may cause clients who were depending on the transitive
dependencies to now fail to compile.