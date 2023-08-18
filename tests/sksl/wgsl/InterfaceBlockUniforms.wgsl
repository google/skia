diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct UniformBuffer {
  @size(32) m1: _skMatrix22,
  m2: _skMatrix22,
};
@group(12) @binding(34) var<uniform> _uniform0 : UniformBuffer;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(_skUnpacked__uniform0_m1[0].x, _skUnpacked__uniform0_m1[1].y, _skUnpacked__uniform0_m2[0].x, _skUnpacked__uniform0_m2[1].y);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
struct _skRow2 {
  @size(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked__uniform0_m1: mat2x2<f32>;
var<private> _skUnpacked__uniform0_m2: mat2x2<f32>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__uniform0_m1 = mat2x2<f32>(_uniform0.m1.c[0].r, _uniform0.m1.c[1].r);
  _skUnpacked__uniform0_m2 = mat2x2<f32>(_uniform0.m2.c[0].r, _uniform0.m2.c[1].r);
}
