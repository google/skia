diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct UniformBuffer {
  u22: _skMatrix22,
  u23: mat2x3<f32>,
  u24: mat2x4<f32>,
  R_u32: _skMatrix32,
  u33: mat3x3<f32>,
  u34: mat3x4<f32>,
  u42: _skMatrix42,
  u43: mat4x3<f32>,
  u44: mat4x4<f32>,
  au22: array<_skArrayElement_f22, 3>,
  au23: array<mat2x3<f32>, 3>,
  au24: array<mat2x4<f32>, 3>,
  au32: array<_skArrayElement_f32, 3>,
  au33: array<mat3x3<f32>, 3>,
  au34: array<mat3x4<f32>, 3>,
  au42: array<_skArrayElement_f42, 3>,
  au43: array<mat4x3<f32>, 3>,
  au44: array<mat4x4<f32>, 3>,
};
@group(0) @binding(1) var<uniform> _uniform0 : UniformBuffer;
struct StorageBuffer {
  s22: _skMatrix22,
  s23: mat2x3<f32>,
  s24: mat2x4<f32>,
  s32: _skMatrix32,
  s33: mat3x3<f32>,
  s34: mat3x4<f32>,
  s42: _skMatrix42,
  s43: mat4x3<f32>,
  s44: mat4x4<f32>,
  as22: array<_skArrayElement_f22, 3>,
  as23: array<mat2x3<f32>, 3>,
  as24: array<mat2x4<f32>, 3>,
  as32: array<_skArrayElement_f32, 3>,
  as33: array<mat3x3<f32>, 3>,
  as34: array<mat3x4<f32>, 3>,
  as42: array<_skArrayElement_f42, 3>,
  as43: array<mat4x3<f32>, 3>,
  as44: array<mat4x4<f32>, 3>,
};
@group(0) @binding(2) var<storage, read_write> _storage1 : StorageBuffer;
fn _skslMain() -> vec4<f32> {
  {
    var value: f32 = ((((((((((((((((((((((((((((((((((_skUnpacked__uniform0_u22[0].x + _uniform0.u23[0].x) + _uniform0.u24[0].x) + _skUnpacked__uniform0_R_u32[0].x) + _uniform0.u33[0].x) + _uniform0.u34[0].x) + _skUnpacked__uniform0_u42[0].x) + _uniform0.u43[0].x) + _uniform0.au44[0][0].x) + _skUnpacked__uniform0_au22[0][0].x) + _uniform0.au23[0][0].x) + _uniform0.au24[0][0].x) + _skUnpacked__uniform0_au32[0][0].x) + _uniform0.au33[0][0].x) + _uniform0.au34[0][0].x) + _skUnpacked__uniform0_au42[0][0].x) + _uniform0.au43[0][0].x) + _uniform0.au44[0][0].x) + _skUnpacked__storage1_s22[0].x) + _storage1.s23[0].x) + _storage1.s24[0].x) + _skUnpacked__storage1_s32[0].x) + _storage1.s33[0].x) + _storage1.s34[0].x) + _skUnpacked__storage1_s42[0].x) + _storage1.s43[0].x) + _storage1.as44[0][0].x) + _skUnpacked__storage1_as22[0][0].x) + _storage1.as23[0][0].x) + _storage1.as24[0][0].x) + _skUnpacked__storage1_as32[0][0].x) + _storage1.as33[0][0].x) + _storage1.as34[0][0].x) + _skUnpacked__storage1_as42[0][0].x) + _storage1.as43[0][0].x) + _storage1.as44[0][0].x;
    return vec4<f32>(f32(value));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain();
  return _stageOut;
}
struct _skArrayElement_f22 {
  e : _skMatrix22
};
struct _skRow2 {
  @size(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked__storage1_as22: array<mat2x2<f32>, 3>;
struct _skArrayElement_f32 {
  e : _skMatrix32
};
struct _skMatrix32 {
  c : array<_skRow2, 3>
};
var<private> _skUnpacked__storage1_as32: array<mat3x2<f32>, 3>;
struct _skArrayElement_f42 {
  e : _skMatrix42
};
struct _skMatrix42 {
  c : array<_skRow2, 4>
};
var<private> _skUnpacked__storage1_as42: array<mat4x2<f32>, 3>;
var<private> _skUnpacked__storage1_s22: mat2x2<f32>;
var<private> _skUnpacked__storage1_s32: mat3x2<f32>;
var<private> _skUnpacked__storage1_s42: mat4x2<f32>;
var<private> _skUnpacked__uniform0_R_u32: mat3x2<f32>;
var<private> _skUnpacked__uniform0_au22: array<mat2x2<f32>, 3>;
var<private> _skUnpacked__uniform0_au32: array<mat3x2<f32>, 3>;
var<private> _skUnpacked__uniform0_au42: array<mat4x2<f32>, 3>;
var<private> _skUnpacked__uniform0_u22: mat2x2<f32>;
var<private> _skUnpacked__uniform0_u42: mat4x2<f32>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__storage1_as22 = array<mat2x2<f32>, 3>(mat2x2<f32>(_storage1.as22[0].e.c[0].r, _storage1.as22[0].e.c[1].r), mat2x2<f32>(_storage1.as22[1].e.c[0].r, _storage1.as22[1].e.c[1].r), mat2x2<f32>(_storage1.as22[2].e.c[0].r, _storage1.as22[2].e.c[1].r));
  _skUnpacked__storage1_as32 = array<mat3x2<f32>, 3>(mat3x2<f32>(_storage1.as32[0].e.c[0].r, _storage1.as32[0].e.c[1].r, _storage1.as32[0].e.c[2].r), mat3x2<f32>(_storage1.as32[1].e.c[0].r, _storage1.as32[1].e.c[1].r, _storage1.as32[1].e.c[2].r), mat3x2<f32>(_storage1.as32[2].e.c[0].r, _storage1.as32[2].e.c[1].r, _storage1.as32[2].e.c[2].r));
  _skUnpacked__storage1_as42 = array<mat4x2<f32>, 3>(mat4x2<f32>(_storage1.as42[0].e.c[0].r, _storage1.as42[0].e.c[1].r, _storage1.as42[0].e.c[2].r, _storage1.as42[0].e.c[3].r), mat4x2<f32>(_storage1.as42[1].e.c[0].r, _storage1.as42[1].e.c[1].r, _storage1.as42[1].e.c[2].r, _storage1.as42[1].e.c[3].r), mat4x2<f32>(_storage1.as42[2].e.c[0].r, _storage1.as42[2].e.c[1].r, _storage1.as42[2].e.c[2].r, _storage1.as42[2].e.c[3].r));
  _skUnpacked__storage1_s22 = mat2x2<f32>(_storage1.s22.c[0].r, _storage1.s22.c[1].r);
  _skUnpacked__storage1_s32 = mat3x2<f32>(_storage1.s32.c[0].r, _storage1.s32.c[1].r, _storage1.s32.c[2].r);
  _skUnpacked__storage1_s42 = mat4x2<f32>(_storage1.s42.c[0].r, _storage1.s42.c[1].r, _storage1.s42.c[2].r, _storage1.s42.c[3].r);
  _skUnpacked__uniform0_R_u32 = mat3x2<f32>(_uniform0.R_u32.c[0].r, _uniform0.R_u32.c[1].r, _uniform0.R_u32.c[2].r);
  _skUnpacked__uniform0_au22 = array<mat2x2<f32>, 3>(mat2x2<f32>(_uniform0.au22[0].e.c[0].r, _uniform0.au22[0].e.c[1].r), mat2x2<f32>(_uniform0.au22[1].e.c[0].r, _uniform0.au22[1].e.c[1].r), mat2x2<f32>(_uniform0.au22[2].e.c[0].r, _uniform0.au22[2].e.c[1].r));
  _skUnpacked__uniform0_au32 = array<mat3x2<f32>, 3>(mat3x2<f32>(_uniform0.au32[0].e.c[0].r, _uniform0.au32[0].e.c[1].r, _uniform0.au32[0].e.c[2].r), mat3x2<f32>(_uniform0.au32[1].e.c[0].r, _uniform0.au32[1].e.c[1].r, _uniform0.au32[1].e.c[2].r), mat3x2<f32>(_uniform0.au32[2].e.c[0].r, _uniform0.au32[2].e.c[1].r, _uniform0.au32[2].e.c[2].r));
  _skUnpacked__uniform0_au42 = array<mat4x2<f32>, 3>(mat4x2<f32>(_uniform0.au42[0].e.c[0].r, _uniform0.au42[0].e.c[1].r, _uniform0.au42[0].e.c[2].r, _uniform0.au42[0].e.c[3].r), mat4x2<f32>(_uniform0.au42[1].e.c[0].r, _uniform0.au42[1].e.c[1].r, _uniform0.au42[1].e.c[2].r, _uniform0.au42[1].e.c[3].r), mat4x2<f32>(_uniform0.au42[2].e.c[0].r, _uniform0.au42[2].e.c[1].r, _uniform0.au42[2].e.c[2].r, _uniform0.au42[2].e.c[3].r));
  _skUnpacked__uniform0_u22 = mat2x2<f32>(_uniform0.u22.c[0].r, _uniform0.u22.c[1].r);
  _skUnpacked__uniform0_u42 = mat4x2<f32>(_uniform0.u42.c[0].r, _uniform0.u42.c[1].r, _uniform0.u42.c[2].r, _uniform0.u42.c[3].r);
}
