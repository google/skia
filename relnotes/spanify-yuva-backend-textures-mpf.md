`GrYUVABackendTextures` now has a constructor that takes its plane textures as an `SkSpan`, and
`SkJpegGainmapEncoder::MakeMPF()` now has an overload that takes its images as an `SkSpan`
(without a separate count), so buffer sizes are checked. The existing pointer-based overloads are
deprecated; they remain available and forward to the new `SkSpan` overloads, but clients should
migrate to the `SkSpan` versions. Note that a C-style array of `GrBackendTexture` still selects
the pointer-based `GrYUVABackendTextures` constructor.
