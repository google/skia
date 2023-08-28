diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  testMatrix2x2: mat2x2<f32>,
  testMatrix3x3: mat3x3<f32>,
  testInputs: vec4<f32>,
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_no_op_mat2_X_vec2_b() -> bool {
  {
    var v: vec2<f32>;
    var vv: vec2<f32>;
    v = _globalUniforms.testInputs.xy;
    v = _globalUniforms.testInputs.xy;
    if any(v != _globalUniforms.testInputs.xy) {
      return false;
    }
    if any(v != _globalUniforms.testInputs.xy) {
      return false;
    }
    v = -_globalUniforms.testInputs.xy;
    v = -_globalUniforms.testInputs.xy;
    if any(v != (-_globalUniforms.testInputs.xy)) {
      return false;
    }
    vv = vec2<f32>(0.0);
    vv = vec2<f32>(0.0);
    return all(vv == vec2<f32>(0.0));
  }
}
fn test_no_op_mat3_X_vec3_b() -> bool {
  {
    var v: vec3<f32>;
    var vv: vec3<f32>;
    v = _globalUniforms.testInputs.xyz;
    v = _globalUniforms.testInputs.xyz;
    if any(v != _globalUniforms.testInputs.xyz) {
      return false;
    }
    if any(v != _globalUniforms.testInputs.xyz) {
      return false;
    }
    v = -_globalUniforms.testInputs.xyz;
    v = -_globalUniforms.testInputs.xyz;
    if any(v != (-_globalUniforms.testInputs.xyz)) {
      return false;
    }
    vv = vec3<f32>(0.0);
    vv = vec3<f32>(0.0);
    return all(vv == vec3<f32>(0.0));
  }
}
fn test_no_op_mat4_X_vec4_b() -> bool {
  {
    var v: vec4<f32>;
    var vv: vec4<f32>;
    v = _globalUniforms.testInputs;
    v = _globalUniforms.testInputs;
    if any(v != _globalUniforms.testInputs) {
      return false;
    }
    if any(v != _globalUniforms.testInputs) {
      return false;
    }
    v = -_globalUniforms.testInputs;
    v = -_globalUniforms.testInputs;
    if any(v != (-_globalUniforms.testInputs)) {
      return false;
    }
    vv = vec4<f32>(0.0);
    vv = vec4<f32>(0.0);
    return all(vv == vec4<f32>(0.0));
  }
}
fn test_no_op_vec2_X_mat2_b() -> bool {
  {
    const n: vec2<f32> = vec2<f32>(-1.0);
    const i: vec2<f32> = vec2<f32>(1.0);
    const z: vec2<f32> = vec2<f32>(0.0);
    var v: vec2<f32>;
    var vv: vec2<f32> = vec2<f32>(0.0);
    vv = vec2<f32>(0.0);
    if any(vv != z) {
      return false;
    }
    v = i * _globalUniforms.testMatrix2x2;
    if any(v != vec2<f32>(3.0, 7.0)) {
      return false;
    }
    v = _globalUniforms.testMatrix2x2 * i;
    if any(v != vec2<f32>(4.0, 6.0)) {
      return false;
    }
    v = n * _globalUniforms.testMatrix2x2;
    if any(v != vec2<f32>(-3.0, -7.0)) {
      return false;
    }
    v = _globalUniforms.testMatrix2x2 * n;
    return all(v == vec2<f32>(-4.0, -6.0));
  }
}
fn test_no_op_vec3_X_mat3_b() -> bool {
  {
    const n: vec3<f32> = vec3<f32>(-1.0);
    const i: vec3<f32> = vec3<f32>(1.0);
    const z: vec3<f32> = vec3<f32>(0.0);
    var v: vec3<f32>;
    var vv: vec3<f32> = vec3<f32>(0.0);
    vv = vec3<f32>(0.0);
    if any(vv != z) {
      return false;
    }
    v = i * _globalUniforms.testMatrix3x3;
    if any(v != vec3<f32>(6.0, 15.0, 24.0)) {
      return false;
    }
    v = _globalUniforms.testMatrix3x3 * i;
    if any(v != vec3<f32>(12.0, 15.0, 18.0)) {
      return false;
    }
    v = n * _globalUniforms.testMatrix3x3;
    if any(v != vec3<f32>(-6.0, -15.0, -24.0)) {
      return false;
    }
    v = _globalUniforms.testMatrix3x3 * n;
    return all(v == vec3<f32>(-12.0, -15.0, -18.0));
  }
}
fn test_no_op_vec4_X_mat4_b() -> bool {
  {
    const n: vec4<f32> = vec4<f32>(-1.0);
    const i: vec4<f32> = vec4<f32>(1.0);
    const z: vec4<f32> = vec4<f32>(0.0);
    let _skTemp0 = _globalUniforms.testMatrix2x2[0];
    let _skTemp1 = _globalUniforms.testMatrix2x2[1];
    let _skTemp2 = _globalUniforms.testMatrix2x2[0];
    let _skTemp3 = _globalUniforms.testMatrix2x2[1];
    let _skTemp4 = _globalUniforms.testMatrix2x2[0];
    let _skTemp5 = _globalUniforms.testMatrix2x2[1];
    let _skTemp6 = _globalUniforms.testMatrix2x2[0];
    let _skTemp7 = _globalUniforms.testMatrix2x2[1];
    var testMatrix4x4: mat4x4<f32> = mat4x4<f32>(_skTemp0[0], _skTemp0[1], _skTemp1[0], _skTemp1[1], _skTemp2[0], _skTemp2[1], _skTemp3[0], _skTemp3[1], _skTemp4[0], _skTemp4[1], _skTemp5[0], _skTemp5[1], _skTemp6[0], _skTemp6[1], _skTemp7[0], _skTemp7[1]);
    var v: vec4<f32>;
    var vv: vec4<f32> = vec4<f32>(0.0);
    vv = vec4<f32>(0.0);
    if any(vv != z) {
      return false;
    }
    v = i * testMatrix4x4;
    if any(v != vec4<f32>(10.0)) {
      return false;
    }
    v = testMatrix4x4 * i;
    if any(v != vec4<f32>(4.0, 8.0, 12.0, 16.0)) {
      return false;
    }
    v = n * testMatrix4x4;
    if any(v != vec4<f32>(-10.0)) {
      return false;
    }
    v = testMatrix4x4 * n;
    return all(v == vec4<f32>(-4.0, -8.0, -12.0, -16.0));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp8: vec4<f32>;
    var _skTemp9: bool;
    var _skTemp10: bool;
    var _skTemp11: bool;
    var _skTemp12: bool;
    var _skTemp13: bool;
    let _skTemp14 = test_no_op_mat2_X_vec2_b();
    if _skTemp14 {
      let _skTemp15 = test_no_op_mat3_X_vec3_b();
      _skTemp13 = _skTemp15;
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      let _skTemp16 = test_no_op_mat4_X_vec4_b();
      _skTemp12 = _skTemp16;
    } else {
      _skTemp12 = false;
    }
    if _skTemp12 {
      let _skTemp17 = test_no_op_vec2_X_mat2_b();
      _skTemp11 = _skTemp17;
    } else {
      _skTemp11 = false;
    }
    if _skTemp11 {
      let _skTemp18 = test_no_op_vec3_X_mat3_b();
      _skTemp10 = _skTemp18;
    } else {
      _skTemp10 = false;
    }
    if _skTemp10 {
      let _skTemp19 = test_no_op_vec4_X_mat4_b();
      _skTemp9 = _skTemp19;
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      _skTemp8 = _globalUniforms.colorGreen;
    } else {
      _skTemp8 = _globalUniforms.colorRed;
    }
    return _skTemp8;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
