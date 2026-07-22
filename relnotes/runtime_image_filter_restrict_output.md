`SkImageFilters::RuntimeShader` factories now take an optional
`restrictOutputToInputBounds` parameter (default `false`). When `true`, the
caller promises that the SkSL evaluates to transparent black wherever its child
shaders are transparent black, allowing the filter's output to be restricted to
the union of its inputs' bounds instead of being unbounded. This lets effects
composed after the runtime shader (e.g. a blur using a non-decal tile mode)
observe the intended content bounds.
