`SkImages::PromiseTextureFromYUVA()` (Ganesh) now has an overload that takes its
`textureContexts` as an `SkSpan`; the span must contain at least one context per plane, and this
is now enforced at runtime. The existing pointer-based overload is deprecated; it remains
available and forwards to the new `SkSpan` overload, but clients should migrate to the `SkSpan`
version. Note that C-style array arguments still select the pointer-based overload.
