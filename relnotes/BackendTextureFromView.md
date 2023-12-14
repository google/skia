`graphite::BackendTexture` can be created from a `WGPUTextureView`. This comes with a
perfomance cost when reading pixels to or writing pixels from the CPU. An intermediate
WGPUTexture is created to support these operations. However, this enables creating
`SkSurface` or `SkImage` from `wgpu::SwapChain::GetCurrentTextureView`.