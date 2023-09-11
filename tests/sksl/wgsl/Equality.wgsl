diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  f1: f32,
  f2: f32,
  f3: f32,
  h1: f32,
  h2: f32,
  h3: f32,
  v1: vec2<f32>,
  v2: vec2<f32>,
  v3: vec2<f32>,
  hv1: vec2<f32>,
  hv2: vec2<f32>,
  hv3: vec2<f32>,
  m1: mat2x2<f32>,
  m2: mat2x2<f32>,
  m3: mat2x2<f32>,
  hm1: mat2x2<f32>,
  hm2: mat2x2<f32>,
  hm3: mat2x2<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain() -> vec4<f32> {
  {
    var ok: bool = true;
    ok = ok && (_globalUniforms.f1 == _globalUniforms.f2);
    ok = ok && (_globalUniforms.h1 == _globalUniforms.h2);
    ok = ok && (_globalUniforms.f1 == f32(_globalUniforms.h2));
    ok = ok && (f32(_globalUniforms.h1) == _globalUniforms.f2);
    ok = ok && (_globalUniforms.f1 != _globalUniforms.f3);
    ok = ok && (_globalUniforms.h1 != _globalUniforms.h3);
    ok = ok && (_globalUniforms.f1 != f32(_globalUniforms.h3));
    ok = ok && (f32(_globalUniforms.h1) != _globalUniforms.f3);
    ok = ok && all(_globalUniforms.v1 == _globalUniforms.v2);
    ok = ok && all(_globalUniforms.hv1 == _globalUniforms.hv2);
    ok = ok && all(_globalUniforms.v1 == vec2<f32>(_globalUniforms.hv2));
    ok = ok && all(vec2<f32>(_globalUniforms.hv1) == _globalUniforms.v2);
    ok = ok && any(_globalUniforms.v1 != _globalUniforms.v3);
    ok = ok && any(_globalUniforms.hv1 != _globalUniforms.hv3);
    ok = ok && any(_globalUniforms.v1 != vec2<f32>(_globalUniforms.hv3));
    ok = ok && any(vec2<f32>(_globalUniforms.hv1) != _globalUniforms.v3);
    ok = ok && (all(_globalUniforms.m1[0] == _globalUniforms.m2[0]) && all(_globalUniforms.m1[1] == _globalUniforms.m2[1]));
    ok = ok && (all(_globalUniforms.hm1[0] == _globalUniforms.hm2[0]) && all(_globalUniforms.hm1[1] == _globalUniforms.hm2[1]));
    let _skTemp0 = mat2x2<f32>(_globalUniforms.hm2);
    ok = ok && (all(_globalUniforms.m1[0] == _skTemp0[0]) && all(_globalUniforms.m1[1] == _skTemp0[1]));
    let _skTemp1 = mat2x2<f32>(_globalUniforms.hm1);
    ok = ok && (all(_skTemp1[0] == _globalUniforms.m2[0]) && all(_skTemp1[1] == _globalUniforms.m2[1]));
    ok = ok && (any(_globalUniforms.m1[0] != _globalUniforms.m3[0]) || any(_globalUniforms.m1[1] != _globalUniforms.m3[1]));
    ok = ok && (any(_globalUniforms.hm1[0] != _globalUniforms.hm3[0]) || any(_globalUniforms.hm1[1] != _globalUniforms.hm3[1]));
    let _skTemp2 = mat2x2<f32>(_globalUniforms.hm3);
    ok = ok && (any(_globalUniforms.m1[0] != _skTemp2[0]) || any(_globalUniforms.m1[1] != _skTemp2[1]));
    let _skTemp3 = mat2x2<f32>(_globalUniforms.hm1);
    ok = ok && (any(_skTemp3[0] != _globalUniforms.m3[0]) || any(_skTemp3[1] != _globalUniforms.m3[1]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain();
  return _stageOut;
}
