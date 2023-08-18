diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testBlock {
  @size(16) s: f32,
  @size(32) m: _skMatrix22,
  @size(32) a: array<_skArrayElement_f, 2>,
  am: array<_skArrayElement_f22, 2>,
};
@group(0) @binding(123) var<uniform> test : array<testBlock, 2>;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(f32(test[0].s), f32(_skUnpacked_test_m[1][1].x), f32(_skUnpacked_test_a[0][1]), f32(_skUnpacked_test_am[1][1][0].y));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
struct _skArrayElement_f {
  @size(16) e : f32
};
var<private> _skUnpacked_test_a: array<array<f32, 2>, 2>;
struct _skArrayElement_f22 {
  e : _skMatrix22
};
struct _skRow2 {
  @size(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked_test_am: array<array<mat2x2<f32>, 2>, 2>;
var<private> _skUnpacked_test_m: array<mat2x2<f32>, 2>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked_test_a[0] = array<f32, 2>(test[0].a[0].e, test[0].a[1].e);
  _skUnpacked_test_a[1] = array<f32, 2>(test[1].a[0].e, test[1].a[1].e);
  _skUnpacked_test_am[0] = array<mat2x2<f32>, 2>(mat2x2<f32>(test[0].am[0].e.c[0].r, test[0].am[0].e.c[1].r), mat2x2<f32>(test[0].am[1].e.c[0].r, test[0].am[1].e.c[1].r));
  _skUnpacked_test_am[1] = array<mat2x2<f32>, 2>(mat2x2<f32>(test[1].am[0].e.c[0].r, test[1].am[0].e.c[1].r), mat2x2<f32>(test[1].am[1].e.c[0].r, test[1].am[1].e.c[1].r));
  _skUnpacked_test_m[0] = mat2x2<f32>(test[0].m.c[0].r, test[0].m.c[1].r);
  _skUnpacked_test_m[1] = mat2x2<f32>(test[1].m.c[0].r, test[1].m.c[1].r);
}
