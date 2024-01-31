In native builds the default use of `wgpu::Device::Tick` to detect GPU progress has been updated
to use `wgpu::Instance::ProcessEvents` instead. To simulate the non-yielding behavior of `Context`
in native `DawnBackendContext::fTick` may still be explicitly set to `nullptr`.
