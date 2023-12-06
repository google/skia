`skgpu::graphite::ContextOptions::fNeverYieldToWebGPU` is removed. Instead, yielding in an
Emscripten build is controlled by installing a client-provided function on
`skgpu::graphite::DawnBackendContext`. The client may install a function that uses Asyncify to
yield to the main thread loop. If no function is installed then the Context has the same
restrictions as with the old option.

In native builds the default is to use `wgpu::Device::Tick` to detect GPU progress. To simulate the
non-yielding behavior of `Context` in native `DawnBackendContext::fTick` may be explicitly set to 
to `nullptr`.

By externalizing the use of Asyncify it is possible to build Skia without generated JS
code that relies on Asyncify.
