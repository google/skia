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
fn test_no_op_scalar_X_mat2_b() -> bool {
  {
    var m: mat2x2<f32>;
    var mm: mat2x2<f32>;
    const z: mat2x2<f32> = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    m = _globalUniforms.testMatrix2x2;
    m = _globalUniforms.testMatrix2x2;
    if (any(m[0] != _globalUniforms.testMatrix2x2[0]) || any(m[1] != _globalUniforms.testMatrix2x2[1])) {
      return false;
    }
    if (any(m[0] != _globalUniforms.testMatrix2x2[0]) || any(m[1] != _globalUniforms.testMatrix2x2[1])) {
      return false;
    }
    if (any(m[0] != _globalUniforms.testMatrix2x2[0]) || any(m[1] != _globalUniforms.testMatrix2x2[1])) {
      return false;
    }
    m = (-1.0 * m);
    let _skTemp0 = (-1.0 * _globalUniforms.testMatrix2x2);
    if (any(m[0] != _skTemp0[0]) || any(m[1] != _skTemp0[1])) {
      return false;
    }
    mm = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    mm = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    return (all(mm[0] == z[0]) && all(mm[1] == z[1]));
  }
}
fn test_no_op_scalar_X_mat3_b() -> bool {
  {
    var m: mat3x3<f32>;
    var mm: mat3x3<f32>;
    const z: mat3x3<f32> = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    m = _globalUniforms.testMatrix3x3;
    m = _globalUniforms.testMatrix3x3;
    if (any(m[0] != _globalUniforms.testMatrix3x3[0]) || any(m[1] != _globalUniforms.testMatrix3x3[1]) || any(m[2] != _globalUniforms.testMatrix3x3[2])) {
      return false;
    }
    if (any(m[0] != _globalUniforms.testMatrix3x3[0]) || any(m[1] != _globalUniforms.testMatrix3x3[1]) || any(m[2] != _globalUniforms.testMatrix3x3[2])) {
      return false;
    }
    if (any(m[0] != _globalUniforms.testMatrix3x3[0]) || any(m[1] != _globalUniforms.testMatrix3x3[1]) || any(m[2] != _globalUniforms.testMatrix3x3[2])) {
      return false;
    }
    m = (-1.0 * m);
    let _skTemp1 = (-1.0 * _globalUniforms.testMatrix3x3);
    if (any(m[0] != _skTemp1[0]) || any(m[1] != _skTemp1[1]) || any(m[2] != _skTemp1[2])) {
      return false;
    }
    mm = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mm = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    return (all(mm[0] == z[0]) && all(mm[1] == z[1]) && all(mm[2] == z[2]));
  }
}
fn test_no_op_scalar_X_mat4_b() -> bool {
  {
    var testMatrix4x4: mat4x4<f32> = mat4x4<f32>(_globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3], _globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3], _globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3], _globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3]);
    var m: mat4x4<f32>;
    var mm: mat4x4<f32>;
    const z: mat4x4<f32> = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    m = testMatrix4x4;
    m = testMatrix4x4;
    if (any(m[0] != testMatrix4x4[0]) || any(m[1] != testMatrix4x4[1]) || any(m[2] != testMatrix4x4[2]) || any(m[3] != testMatrix4x4[3])) {
      return false;
    }
    if (any(m[0] != testMatrix4x4[0]) || any(m[1] != testMatrix4x4[1]) || any(m[2] != testMatrix4x4[2]) || any(m[3] != testMatrix4x4[3])) {
      return false;
    }
    if (any(m[0] != testMatrix4x4[0]) || any(m[1] != testMatrix4x4[1]) || any(m[2] != testMatrix4x4[2]) || any(m[3] != testMatrix4x4[3])) {
      return false;
    }
    m = (-1.0 * m);
    let _skTemp2 = (-1.0 * testMatrix4x4);
    if (any(m[0] != _skTemp2[0]) || any(m[1] != _skTemp2[1]) || any(m[2] != _skTemp2[2]) || any(m[3] != _skTemp2[3])) {
      return false;
    }
    mm = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mm = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    return (all(mm[0] == z[0]) && all(mm[1] == z[1]) && all(mm[2] == z[2]) && all(mm[3] == z[3]));
  }
}
fn test_no_op_mat2_X_scalar_b() -> bool {
  {
    var m: mat2x2<f32>;
    var mm: mat2x2<f32>;
    const z: mat2x2<f32> = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    const s: mat2x2<f32> = mat2x2<f32>(vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3]);
    var scalar: f32 = _globalUniforms.testInputs.x;
    m = mat2x2<f32>(scalar, 0.0, 0.0, scalar);
    m = mat2x2<f32>(scalar, 0.0, 0.0, scalar);
    let _skTemp3 = mat2x2<f32>(scalar, 0.0, 0.0, scalar);
    if (any(m[0] != _skTemp3[0]) || any(m[1] != _skTemp3[1])) {
      return false;
    }
    m = mat2x2<f32>(scalar / s[0], scalar / s[1]);
    let _skTemp4 = mat2x2<f32>(scalar, scalar, scalar, scalar);
    if (any(m[0] != _skTemp4[0]) || any(m[1] != _skTemp4[1])) {
      return false;
    }
    m = mat2x2<f32>(scalar + z[0], scalar + z[1]);
    m = mat2x2<f32>(z[0] + scalar, z[1] + scalar);
    let _skTemp5 = mat2x2<f32>(scalar, scalar, scalar, scalar);
    if (any(m[0] != _skTemp5[0]) || any(m[1] != _skTemp5[1])) {
      return false;
    }
    m = mat2x2<f32>(scalar - z[0], scalar - z[1]);
    m = mat2x2<f32>(z[0] - scalar, z[1] - scalar);
    let _skTemp6 = (-1.0 * mat2x2<f32>(scalar, scalar, scalar, scalar));
    if (any(m[0] != _skTemp6[0]) || any(m[1] != _skTemp6[1])) {
      return false;
    }
    mm = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    mm = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    return (all(mm[0] == z[0]) && all(mm[1] == z[1]));
  }
}
fn test_no_op_mat3_X_scalar_b() -> bool {
  {
    var m: mat3x3<f32>;
    var mm: mat3x3<f32>;
    const z: mat3x3<f32> = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    const s: mat3x3<f32> = mat3x3<f32>(vec3<f32>(1.0)[0], vec3<f32>(1.0)[1], vec3<f32>(1.0)[2], vec3<f32>(1.0)[0], vec3<f32>(1.0)[1], vec3<f32>(1.0)[2], vec3<f32>(1.0)[0], vec3<f32>(1.0)[1], vec3<f32>(1.0)[2]);
    var scalar: f32 = _globalUniforms.testInputs.x;
    var scalar3: vec3<f32> = vec3<f32>(scalar);
    m = mat3x3<f32>(scalar, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, scalar);
    m = mat3x3<f32>(scalar, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, scalar);
    let _skTemp7 = mat3x3<f32>(scalar, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, scalar);
    if (any(m[0] != _skTemp7[0]) || any(m[1] != _skTemp7[1]) || any(m[2] != _skTemp7[2])) {
      return false;
    }
    m = mat3x3<f32>(scalar / s[0], scalar / s[1], scalar / s[2]);
    let _skTemp8 = mat3x3<f32>(scalar3[0], scalar3[1], scalar3[2], scalar3[0], scalar3[1], scalar3[2], scalar3[0], scalar3[1], scalar3[2]);
    if (any(m[0] != _skTemp8[0]) || any(m[1] != _skTemp8[1]) || any(m[2] != _skTemp8[2])) {
      return false;
    }
    m = mat3x3<f32>(scalar + z[0], scalar + z[1], scalar + z[2]);
    m = mat3x3<f32>(z[0] + scalar, z[1] + scalar, z[2] + scalar);
    let _skTemp9 = mat3x3<f32>(scalar3[0], scalar3[1], scalar3[2], scalar3[0], scalar3[1], scalar3[2], scalar3[0], scalar3[1], scalar3[2]);
    if (any(m[0] != _skTemp9[0]) || any(m[1] != _skTemp9[1]) || any(m[2] != _skTemp9[2])) {
      return false;
    }
    m = mat3x3<f32>(scalar - z[0], scalar - z[1], scalar - z[2]);
    m = mat3x3<f32>(z[0] - scalar, z[1] - scalar, z[2] - scalar);
    let _skTemp10 = (-1.0 * mat3x3<f32>(scalar3[0], scalar3[1], scalar3[2], scalar3[0], scalar3[1], scalar3[2], scalar3[0], scalar3[1], scalar3[2]));
    if (any(m[0] != _skTemp10[0]) || any(m[1] != _skTemp10[1]) || any(m[2] != _skTemp10[2])) {
      return false;
    }
    mm = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mm = mat3x3<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    return (all(mm[0] == z[0]) && all(mm[1] == z[1]) && all(mm[2] == z[2]));
  }
}
fn test_no_op_mat4_X_scalar_b() -> bool {
  {
    var m: mat4x4<f32>;
    var mm: mat4x4<f32>;
    const z: mat4x4<f32> = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    const s: mat4x4<f32> = mat4x4<f32>(vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3], vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3], vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3], vec4<f32>(1.0)[0], vec4<f32>(1.0)[1], vec4<f32>(1.0)[2], vec4<f32>(1.0)[3]);
    var scalar: f32 = _globalUniforms.testInputs.x;
    var scalar4: vec4<f32> = vec4<f32>(scalar);
    m = mat4x4<f32>(scalar, 0.0, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, 0.0, scalar);
    m = mat4x4<f32>(scalar, 0.0, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, 0.0, scalar);
    let _skTemp11 = mat4x4<f32>(scalar, 0.0, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, 0.0, scalar, 0.0, 0.0, 0.0, 0.0, scalar);
    if (any(m[0] != _skTemp11[0]) || any(m[1] != _skTemp11[1]) || any(m[2] != _skTemp11[2]) || any(m[3] != _skTemp11[3])) {
      return false;
    }
    m = mat4x4<f32>(scalar / s[0], scalar / s[1], scalar / s[2], scalar / s[3]);
    let _skTemp12 = mat4x4<f32>(scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3]);
    if (any(m[0] != _skTemp12[0]) || any(m[1] != _skTemp12[1]) || any(m[2] != _skTemp12[2]) || any(m[3] != _skTemp12[3])) {
      return false;
    }
    m = mat4x4<f32>(scalar + z[0], scalar + z[1], scalar + z[2], scalar + z[3]);
    m = mat4x4<f32>(z[0] + scalar, z[1] + scalar, z[2] + scalar, z[3] + scalar);
    let _skTemp13 = mat4x4<f32>(scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3]);
    if (any(m[0] != _skTemp13[0]) || any(m[1] != _skTemp13[1]) || any(m[2] != _skTemp13[2]) || any(m[3] != _skTemp13[3])) {
      return false;
    }
    m = mat4x4<f32>(scalar - z[0], scalar - z[1], scalar - z[2], scalar - z[3]);
    m = mat4x4<f32>(z[0] - scalar, z[1] - scalar, z[2] - scalar, z[3] - scalar);
    let _skTemp14 = (-1.0 * mat4x4<f32>(scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3], scalar4[0], scalar4[1], scalar4[2], scalar4[3]));
    if (any(m[0] != _skTemp14[0]) || any(m[1] != _skTemp14[1]) || any(m[2] != _skTemp14[2]) || any(m[3] != _skTemp14[3])) {
      return false;
    }
    mm = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mm = mat4x4<f32>(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    return (all(mm[0] == z[0]) && all(mm[1] == z[1]) && all(mm[2] == z[2]) && all(mm[3] == z[3]));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp15: vec4<f32>;
    var _skTemp16: bool;
    var _skTemp17: bool;
    var _skTemp18: bool;
    var _skTemp19: bool;
    var _skTemp20: bool;
    let _skTemp21 = test_no_op_scalar_X_mat2_b();
    if _skTemp21 {
      let _skTemp22 = test_no_op_scalar_X_mat3_b();
      _skTemp20 = _skTemp22;
    } else {
      _skTemp20 = false;
    }
    if _skTemp20 {
      let _skTemp23 = test_no_op_scalar_X_mat4_b();
      _skTemp19 = _skTemp23;
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      let _skTemp24 = test_no_op_mat2_X_scalar_b();
      _skTemp18 = _skTemp24;
    } else {
      _skTemp18 = false;
    }
    if _skTemp18 {
      let _skTemp25 = test_no_op_mat3_X_scalar_b();
      _skTemp17 = _skTemp25;
    } else {
      _skTemp17 = false;
    }
    if _skTemp17 {
      let _skTemp26 = test_no_op_mat4_X_scalar_b();
      _skTemp16 = _skTemp26;
    } else {
      _skTemp16 = false;
    }
    if _skTemp16 {
      _skTemp15 = _globalUniforms.colorGreen;
    } else {
      _skTemp15 = _globalUniforms.colorRed;
    }
    return _skTemp15;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
