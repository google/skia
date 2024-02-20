diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testBlock {
  @size(4) x: f32,
  @size(12) w: i32,
  @size(32) y: array<_skArrayElement_h, 2>,
  z: mat3x3<f32>,
};
@group(0) @binding(0) var<uniform> _uniform0 : testBlock;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(_uniform0.x, _skUnpacked__uniform0_y[0], _skUnpacked__uniform0_y[1], 0.0);
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
struct _skArrayElement_h {
  @size(16) e : f32
};
var<private> _skUnpacked__uniform0_y: array<f32, 2>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__uniform0_y = array<f32, 2>(_uniform0.y[0].e, _uniform0.y[1].e);
}
