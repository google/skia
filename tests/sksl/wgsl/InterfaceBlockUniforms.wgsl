diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct UniformBuffer {
  @size(32) m1: _skMatrix22h,
  m2: _skMatrix22h,
};
@group(12) @binding(34) var<uniform> _uniform0 : UniformBuffer;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f16>(_skUnpacked__uniform0_m1[0].x, _skUnpacked__uniform0_m1[1].y, _skUnpacked__uniform0_m2[0].x, _skUnpacked__uniform0_m2[1].y);
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
struct _skRow2h {
  @align(16) r : vec2<f16>
};
struct _skMatrix22h {
  c : array<_skRow2h, 2>
};
var<private> _skUnpacked__uniform0_m1: mat2x2<f16>;
var<private> _skUnpacked__uniform0_m2: mat2x2<f16>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__uniform0_m1 = mat2x2<f16>(_uniform0.m1.c[0].r, _uniform0.m1.c[1].r);
  _skUnpacked__uniform0_m2 = mat2x2<f16>(_uniform0.m2.c[0].r, _uniform0.m2.c[1].r);
}
