diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testBlock {
  @size(16) x: f32,
  m: _skMatrix22,
};
@group(0) @binding(123) var<uniform> test : array<testBlock, 2>;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(f32(test[0].x), f32(_skUnpacked_test_m[0][0].y), vec2<f32>(_skUnpacked_test_m[1][1]));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}
struct _skRow2 {
  @size(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked_test_m: array<mat2x2<f32>, 2>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked_test_m[0] = mat2x2<f32>(test[0].m.c[0].r, test[0].m.c[1].r);
  _skUnpacked_test_m[1] = mat2x2<f32>(test[1].m.c[0].r, test[1].m.c[1].r);
}
