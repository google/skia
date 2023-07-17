### Compilation failed:

error: :5:1 error: structures must have at least one member
struct FSOut {
^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
};
struct _GlobalUniforms {
  x: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;

1 error
