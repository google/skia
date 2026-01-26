diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct testBlock {
  @size(4) x: f16,
  @size(12) w: i32,
  @size(32) y: array<_skArrayElement_h, 2>,
  z: _skMatrix33h,
};
@group(0) @binding(0) var<uniform> _uniform0 : testBlock;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f16>(_uniform0.x, _skUnpacked__uniform0_y[0], _skUnpacked__uniform0_y[1], 0.0h);
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
struct _skArrayElement_h {
  @align(16) e : f16
};
var<private> _skUnpacked__uniform0_y: array<f16, 2>;
struct _skRow3h {
  @align(16) r : vec3<f16>
};
struct _skMatrix33h {
  c : array<_skRow3h, 3>
};
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__uniform0_y = array<f16, 2>(_uniform0.y[0].e, _uniform0.y[1].e);
}
