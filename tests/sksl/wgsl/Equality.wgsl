diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  f1: f32,
  f2: f32,
  f3: f32,
  h1: f16,
  h2: f16,
  h3: f16,
  v1: vec2<f32>,
  v2: vec2<f32>,
  v3: vec2<f32>,
  hv1: vec2<f16>,
  hv2: vec2<f16>,
  hv3: vec2<f16>,
  m1: _skMatrix22f,
  m2: _skMatrix22f,
  m3: _skMatrix22f,
  hm1: _skMatrix22h,
  hm2: _skMatrix22h,
  hm3: _skMatrix22h,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain() -> vec4<f16> {
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
    ok = ok && (all(_skUnpacked__globalUniforms_m1[0] == _skUnpacked__globalUniforms_m2[0]) && all(_skUnpacked__globalUniforms_m1[1] == _skUnpacked__globalUniforms_m2[1]));
    ok = ok && (all(_skUnpacked__globalUniforms_hm1[0] == _skUnpacked__globalUniforms_hm2[0]) && all(_skUnpacked__globalUniforms_hm1[1] == _skUnpacked__globalUniforms_hm2[1]));
    let _skTemp0 = mat2x2<f32>(_skUnpacked__globalUniforms_hm2);
    ok = ok && (all(_skUnpacked__globalUniforms_m1[0] == _skTemp0[0]) && all(_skUnpacked__globalUniforms_m1[1] == _skTemp0[1]));
    let _skTemp1 = mat2x2<f32>(_skUnpacked__globalUniforms_hm1);
    ok = ok && (all(_skTemp1[0] == _skUnpacked__globalUniforms_m2[0]) && all(_skTemp1[1] == _skUnpacked__globalUniforms_m2[1]));
    ok = ok && (any(_skUnpacked__globalUniforms_m1[0] != _skUnpacked__globalUniforms_m3[0]) || any(_skUnpacked__globalUniforms_m1[1] != _skUnpacked__globalUniforms_m3[1]));
    ok = ok && (any(_skUnpacked__globalUniforms_hm1[0] != _skUnpacked__globalUniforms_hm3[0]) || any(_skUnpacked__globalUniforms_hm1[1] != _skUnpacked__globalUniforms_hm3[1]));
    let _skTemp2 = mat2x2<f32>(_skUnpacked__globalUniforms_hm3);
    ok = ok && (any(_skUnpacked__globalUniforms_m1[0] != _skTemp2[0]) || any(_skUnpacked__globalUniforms_m1[1] != _skTemp2[1]));
    let _skTemp3 = mat2x2<f32>(_skUnpacked__globalUniforms_hm1);
    ok = ok && (any(_skTemp3[0] != _skUnpacked__globalUniforms_m3[0]) || any(_skTemp3[1] != _skUnpacked__globalUniforms_m3[1]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain();
  return _stageOut;
}
struct _skRow2h {
  @align(16) r : vec2<f16>
};
struct _skMatrix22h {
  c : array<_skRow2h, 2>
};
var<private> _skUnpacked__globalUniforms_hm1: mat2x2<f16>;
var<private> _skUnpacked__globalUniforms_hm2: mat2x2<f16>;
var<private> _skUnpacked__globalUniforms_hm3: mat2x2<f16>;
struct _skRow2f {
  @align(16) r : vec2<f32>
};
struct _skMatrix22f {
  c : array<_skRow2f, 2>
};
var<private> _skUnpacked__globalUniforms_m1: mat2x2<f32>;
var<private> _skUnpacked__globalUniforms_m2: mat2x2<f32>;
var<private> _skUnpacked__globalUniforms_m3: mat2x2<f32>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_hm1 = mat2x2<f16>(_globalUniforms.hm1.c[0].r, _globalUniforms.hm1.c[1].r);
  _skUnpacked__globalUniforms_hm2 = mat2x2<f16>(_globalUniforms.hm2.c[0].r, _globalUniforms.hm2.c[1].r);
  _skUnpacked__globalUniforms_hm3 = mat2x2<f16>(_globalUniforms.hm3.c[0].r, _globalUniforms.hm3.c[1].r);
  _skUnpacked__globalUniforms_m1 = mat2x2<f32>(_globalUniforms.m1.c[0].r, _globalUniforms.m1.c[1].r);
  _skUnpacked__globalUniforms_m2 = mat2x2<f32>(_globalUniforms.m2.c[0].r, _globalUniforms.m2.c[1].r);
  _skUnpacked__globalUniforms_m3 = mat2x2<f32>(_globalUniforms.m3.c[0].r, _globalUniforms.m3.c[1].r);
}
