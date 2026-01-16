diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  x: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
