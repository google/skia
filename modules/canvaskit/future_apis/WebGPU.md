# WebGPU API

Date Updated: June 16, 2020

## Summary and Links

WebGPU exposes an API for performing operations, such as rendering and computation,
on a Graphics Processing Unit. [Dawn](https://dawn.googlesource.com/dawn) is the underlying
implementation of WebGPU in chromium. In the future, with
[WebGPU bindings provided by emscripten](https://github.com/emscripten-core/emscripten/pull/10218),
CanvasKit should be able to use a WebGPU rendering device.

- [W.I.P. Specification](https://gpuweb.github.io/gpuweb/)
- [WebGPU Samples](https://austineng.github.io/webgpu-samples/)
- [Implementation Status](https://github.com/gpuweb/gpuweb/wiki/Implementation-Status)

Some features are currently available in Chrome Canary behind the `--enable-unsafe-webgpu` flag.
