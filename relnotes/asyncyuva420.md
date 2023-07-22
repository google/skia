New methods are added to `SkImage`, `SkSurface`, and `skgpu::graphite::context` named
`asyncRescaleAndReadPixeksYUVA420`. These function identically to the existing
`asyncRescaleAndReadPixelsYUV420` methods but return a fourth plane containing alpha at full
resolution.