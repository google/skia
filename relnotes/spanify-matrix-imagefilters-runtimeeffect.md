`SkMatrix::getMinMaxScales()` and the multi-child `SkImageFilters::RuntimeShader()` factories now
have overloads that take `SkSpan` instead of raw pointers (`RuntimeShader()` takes
`SkSpan<const std::string_view>` child shader names and `SkSpan<const sk_sp<SkImageFilter>>`
inputs instead of two arrays plus a count), so buffer sizes are checked. The
`SkRuntimeEffect::makeShader()` overload that takes an `sk_sp<SkShader>` array plus a count is
deprecated in favor of the existing `SkSpan<const SkRuntimeEffect::ChildPtr>` overload. The
pointer-based overloads remain available and forward to the `SkSpan` overloads, but clients
should migrate to the `SkSpan` versions. Note that C-style array arguments still select the
pointer-based overloads. The `SkSpan` `RuntimeShader()` factories return nullptr if
`childShaderNames` and `inputs` have different sizes; passing a trailing count to them is a
compile error.
