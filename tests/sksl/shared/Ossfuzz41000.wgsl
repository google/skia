### Compilation failed:

error: :4:1 error: structures must have at least one member
struct FSOut {
^^^^^^


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
