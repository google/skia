`SkImageFilters::Magnifier(srcRect, inset)` is deprecated. These parameters do not provide enough
information for the implementation to correctly respond to canvas transform or participate accurately
in layer bounds planning.

A new `SkImageFilters::Magnifier` function is added that takes additional parameters: the outer
lens bounds and the actual zoom amount (instead of inconsistently reconstructing the target zoom
amount, which was the prior behavior). Additionally, the new factory accepts an SkSamplingOptions
to control the sampling quality.
