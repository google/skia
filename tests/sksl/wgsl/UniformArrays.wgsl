diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct UniformBuffer {
  uf: array<_skArrayElement_f, 3>,
  uf2: array<_skArrayElement_f2, 3>,
  uf3: array<_skArrayElement_f3, 3>,
  uf4: array<vec4<f32>, 3>,
  uh: array<_skArrayElement_h, 3>,
  uh2: array<_skArrayElement_h2, 3>,
  uh3: array<_skArrayElement_h3, 3>,
  uh4: array<vec4<f32>, 3>,
  ui: array<_skArrayElement_i, 3>,
  ui2: array<_skArrayElement_i2, 3>,
  ui3: array<_skArrayElement_i3, 3>,
  ui4: array<vec4<i32>, 3>,
};
@group(0) @binding(1) var<uniform> _uniform0 : UniformBuffer;
struct StorageBuffer {
  sf: array<_skArrayElement_f, 4>,
  sf2: array<_skArrayElement_f2, 4>,
  sf3: array<_skArrayElement_f3, 4>,
  sf4: array<vec4<f32>, 4>,
  sh: array<_skArrayElement_h, 4>,
  sh2: array<_skArrayElement_h2, 4>,
  sh3: array<_skArrayElement_h3, 4>,
  sh4: array<vec4<f32>, 4>,
  si: array<_skArrayElement_i, 4>,
  si2: array<_skArrayElement_i2, 4>,
  si3: array<_skArrayElement_i3, 4>,
  si4: array<vec4<i32>, 4>,
};
@group(0) @binding(2) var<storage, read_write> _storage1 : StorageBuffer;
fn _skslMain() -> vec4<f32> {
  {
    var value: f32 = ((((((((((((((((((((((_skUnpacked__uniform0_uf[1] + _skUnpacked__uniform0_uf2[1].x) + _skUnpacked__uniform0_uf3[1].x) + _uniform0.uf4[1].x) + f32(_skUnpacked__uniform0_uh[1])) + f32(_skUnpacked__uniform0_uh2[1].x)) + f32(_skUnpacked__uniform0_uh3[1].x)) + f32(_uniform0.uh4[1].x)) + f32(_skUnpacked__uniform0_ui[1])) + f32(_skUnpacked__uniform0_ui2[1].x)) + f32(_skUnpacked__uniform0_ui3[1].x)) + f32(_uniform0.ui4[1].x)) + _skUnpacked__storage1_sf[1]) + _skUnpacked__storage1_sf2[1].x) + _skUnpacked__storage1_sf3[1].x) + _storage1.sf4[1].x) + f32(_skUnpacked__storage1_sh[1])) + f32(_skUnpacked__storage1_sh2[1].x)) + f32(_skUnpacked__storage1_sh3[1].x)) + f32(_storage1.sh4[1].x)) + f32(_skUnpacked__storage1_si[1])) + f32(_skUnpacked__storage1_si2[1].x)) + f32(_skUnpacked__storage1_si3[1].x)) + f32(_storage1.si4[1].x);
    return vec4<f32>(f32(value));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain();
  return _stageOut;
}
struct _skArrayElement_f {
  @size(16) e : f32
};
var<private> _skUnpacked__storage1_sf: array<f32, 4>;
struct _skArrayElement_f2 {
  @size(16) e : vec2<f32>
};
var<private> _skUnpacked__storage1_sf2: array<vec2<f32>, 4>;
struct _skArrayElement_f3 {
  @size(16) e : vec3<f32>
};
var<private> _skUnpacked__storage1_sf3: array<vec3<f32>, 4>;
struct _skArrayElement_h {
  @size(16) e : f32
};
var<private> _skUnpacked__storage1_sh: array<f32, 4>;
struct _skArrayElement_h2 {
  @size(16) e : vec2<f32>
};
var<private> _skUnpacked__storage1_sh2: array<vec2<f32>, 4>;
struct _skArrayElement_h3 {
  @size(16) e : vec3<f32>
};
var<private> _skUnpacked__storage1_sh3: array<vec3<f32>, 4>;
struct _skArrayElement_i {
  @size(16) e : i32
};
var<private> _skUnpacked__storage1_si: array<i32, 4>;
struct _skArrayElement_i2 {
  @size(16) e : vec2<i32>
};
var<private> _skUnpacked__storage1_si2: array<vec2<i32>, 4>;
struct _skArrayElement_i3 {
  @size(16) e : vec3<i32>
};
var<private> _skUnpacked__storage1_si3: array<vec3<i32>, 4>;
var<private> _skUnpacked__uniform0_uf: array<f32, 3>;
var<private> _skUnpacked__uniform0_uf2: array<vec2<f32>, 3>;
var<private> _skUnpacked__uniform0_uf3: array<vec3<f32>, 3>;
var<private> _skUnpacked__uniform0_uh: array<f32, 3>;
var<private> _skUnpacked__uniform0_uh2: array<vec2<f32>, 3>;
var<private> _skUnpacked__uniform0_uh3: array<vec3<f32>, 3>;
var<private> _skUnpacked__uniform0_ui: array<i32, 3>;
var<private> _skUnpacked__uniform0_ui2: array<vec2<i32>, 3>;
var<private> _skUnpacked__uniform0_ui3: array<vec3<i32>, 3>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__storage1_sf = array<f32, 4>(_storage1.sf[0].e, _storage1.sf[1].e, _storage1.sf[2].e, _storage1.sf[3].e);
  _skUnpacked__storage1_sf2 = array<vec2<f32>, 4>(_storage1.sf2[0].e, _storage1.sf2[1].e, _storage1.sf2[2].e, _storage1.sf2[3].e);
  _skUnpacked__storage1_sf3 = array<vec3<f32>, 4>(_storage1.sf3[0].e, _storage1.sf3[1].e, _storage1.sf3[2].e, _storage1.sf3[3].e);
  _skUnpacked__storage1_sh = array<f32, 4>(_storage1.sh[0].e, _storage1.sh[1].e, _storage1.sh[2].e, _storage1.sh[3].e);
  _skUnpacked__storage1_sh2 = array<vec2<f32>, 4>(_storage1.sh2[0].e, _storage1.sh2[1].e, _storage1.sh2[2].e, _storage1.sh2[3].e);
  _skUnpacked__storage1_sh3 = array<vec3<f32>, 4>(_storage1.sh3[0].e, _storage1.sh3[1].e, _storage1.sh3[2].e, _storage1.sh3[3].e);
  _skUnpacked__storage1_si = array<i32, 4>(_storage1.si[0].e, _storage1.si[1].e, _storage1.si[2].e, _storage1.si[3].e);
  _skUnpacked__storage1_si2 = array<vec2<i32>, 4>(_storage1.si2[0].e, _storage1.si2[1].e, _storage1.si2[2].e, _storage1.si2[3].e);
  _skUnpacked__storage1_si3 = array<vec3<i32>, 4>(_storage1.si3[0].e, _storage1.si3[1].e, _storage1.si3[2].e, _storage1.si3[3].e);
  _skUnpacked__uniform0_uf = array<f32, 3>(_uniform0.uf[0].e, _uniform0.uf[1].e, _uniform0.uf[2].e);
  _skUnpacked__uniform0_uf2 = array<vec2<f32>, 3>(_uniform0.uf2[0].e, _uniform0.uf2[1].e, _uniform0.uf2[2].e);
  _skUnpacked__uniform0_uf3 = array<vec3<f32>, 3>(_uniform0.uf3[0].e, _uniform0.uf3[1].e, _uniform0.uf3[2].e);
  _skUnpacked__uniform0_uh = array<f32, 3>(_uniform0.uh[0].e, _uniform0.uh[1].e, _uniform0.uh[2].e);
  _skUnpacked__uniform0_uh2 = array<vec2<f32>, 3>(_uniform0.uh2[0].e, _uniform0.uh2[1].e, _uniform0.uh2[2].e);
  _skUnpacked__uniform0_uh3 = array<vec3<f32>, 3>(_uniform0.uh3[0].e, _uniform0.uh3[1].e, _uniform0.uh3[2].e);
  _skUnpacked__uniform0_ui = array<i32, 3>(_uniform0.ui[0].e, _uniform0.ui[1].e, _uniform0.ui[2].e);
  _skUnpacked__uniform0_ui2 = array<vec2<i32>, 3>(_uniform0.ui2[0].e, _uniform0.ui2[1].e, _uniform0.ui2[2].e);
  _skUnpacked__uniform0_ui3 = array<vec3<i32>, 3>(_uniform0.ui3[0].e, _uniform0.ui3[1].e, _uniform0.ui3[2].e);
}
