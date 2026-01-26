diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct _GlobalUniforms {
  x: f32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
