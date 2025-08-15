`SkShader::makeWithWorkingColorSpace()` now accepts an optional output
colorspace parameter. If it is null (the default), it's assumed to be the same
as the input or working colorspace parameter. This allows shaders to actively
participate in colorspace conversion and inform Skia about the space changes
that they apply.
