`skgpu::graphite::ContextOptions` has a new field, `fNeverYieldToWebGPU`. This new option
is only valid with the Dawn backend. It indicates that `skgpu::graphite::Context` should never yield
to Dawn. In native this means `wgpu::Device::Tick()` is never called. In Emscripten it
means `Context` never yields to the main thread event loop.

When the option is enabled, `skgpu::SyncToCpu::kYes` is ignored when passed to
`Context::submit()`. Moreover, it is a fatal error to have submitted but unfinished
GPU work before deleting `Context`. A new method, `hasUnfinishedGpuWork()` is added
to `Context` that can be used to test this condition.

The intent of this option is to be able to use Graphite in WASM without requiring Asyncify.