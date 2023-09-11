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
fn test_mat3_mat3_b() -> bool {
  {
    var m: mat3x3<f32>;
    var mm: mat3x3<f32>;
    const z: mat3x3<f32> = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    m = _globalUniforms.testMatrix3x3;
    m = _globalUniforms.testMatrix3x3;
    m = (-1.0 * m);
    mm = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mm = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    let _skTemp0 = (-1.0 * _globalUniforms.testMatrix3x3);
    return (all(m[0] == _skTemp0[0]) && all(m[1] == _skTemp0[1]) && all(m[2] == _skTemp0[2])) && (all(mm[0] == z[0]) && all(mm[1] == z[1]) && all(mm[2] == z[2]));
  }
}
fn test_mat4_mat4_b() -> bool {
  {
    var testMatrix4x4: mat4x4<f32> = mat4x4<f32>(_globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3], _globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3], _globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3], _globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3]);
    var m: mat4x4<f32>;
    var mm: mat4x4<f32>;
    const z: mat4x4<f32> = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    m = testMatrix4x4;
    m = testMatrix4x4;
    m = (-1.0 * m);
    mm = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mm = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    let _skTemp1 = (-1.0 * testMatrix4x4);
    return (all(m[0] == _skTemp1[0]) && all(m[1] == _skTemp1[1]) && all(m[2] == _skTemp1[2]) && all(m[3] == _skTemp1[3])) && (all(mm[0] == z[0]) && all(mm[1] == z[1]) && all(mm[2] == z[2]) && all(mm[3] == z[3]));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_m: mat2x2<f32>;
    var _1_mm: mat2x2<f32>;
    const _3_z: mat2x2<f32> = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    _0_m = _globalUniforms.testMatrix2x2;
    _0_m = _globalUniforms.testMatrix2x2;
    _0_m = (-1.0 * _0_m);
    _1_mm = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    _1_mm = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    var _skTemp2: vec4<f32>;
    var _skTemp3: bool;
    var _skTemp4: bool;
    let _skTemp5 = (-1.0 * _globalUniforms.testMatrix2x2);
    if (all(_0_m[0] == _skTemp5[0]) && all(_0_m[1] == _skTemp5[1])) && (all(_1_mm[0] == _3_z[0]) && all(_1_mm[1] == _3_z[1])) {
      let _skTemp6 = test_mat3_mat3_b();
      _skTemp4 = _skTemp6;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp7 = test_mat4_mat4_b();
      _skTemp3 = _skTemp7;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = _globalUniforms.colorGreen;
    } else {
      _skTemp2 = _globalUniforms.colorRed;
    }
    return _skTemp2;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
