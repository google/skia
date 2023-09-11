diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct _GlobalUniforms {
  x: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
