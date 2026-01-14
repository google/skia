diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  testMatrix2x2: _skMatrix22,
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
  unknownInput: f32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn test_matrix_op_scalar_float_b() -> bool {
  {
    const ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_scalar_half_b() -> bool {
  {
    const ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_matrix_float_b() -> bool {
  {
    const ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_matrix_half_b() -> bool {
  {
    const ok: bool = true;
    return ok;
  }
}
fn test_vector_op_matrix_float_b() -> bool {
  {
    const ok: bool = true;
    return ok;
  }
}
fn test_vector_op_matrix_half_b() -> bool {
  {
    const ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_vector_float_b() -> bool {
  {
    const ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_vector_half_b() -> bool {
  {
    const ok: bool = true;
    return ok;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_ok: bool = true;
    let _skTemp0 = f32(_globalUniforms.unknownInput);
    let _skTemp1 = mat3x3<f32>(_skTemp0, 0.0, 0.0, 0.0, _skTemp0, 0.0, 0.0, 0.0, _skTemp0);
    const _skTemp2 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    const _skTemp3 = mat3x3<f32>(_skTemp2[0][0], _skTemp2[0][1], 0.0, _skTemp2[1][0], _skTemp2[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp1[0] == _skTemp3[0]) && all(_skTemp1[1] == _skTemp3[1]) && all(_skTemp1[2] == _skTemp3[2]));
    let _skTemp4 = mat3x3<f32>(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, f32(_globalUniforms.unknownInput));
    const _skTemp5 = mat2x2<f32>(9.0, 0.0, 0.0, 9.0);
    const _skTemp6 = mat3x3<f32>(_skTemp5[0][0], _skTemp5[0][1], 0.0, _skTemp5[1][0], _skTemp5[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp4[0] == _skTemp6[0]) && all(_skTemp4[1] == _skTemp6[1]) && all(_skTemp4[2] == _skTemp6[2]));
    _0_ok = _0_ok && all(vec4<f32>(_skUnpacked__globalUniforms_testMatrix2x2[0], _skUnpacked__globalUniforms_testMatrix2x2[1]) == vec4<f32>(1.0, 2.0, 3.0, 4.0));
    {
      let _skTemp7 = mat2x2<f32>(_skUnpacked__globalUniforms_testMatrix2x2);
      let _skTemp8 = mat3x3<f32>(mat3x3<f32>(_skTemp7[0][0], _skTemp7[0][1], 0.0, _skTemp7[1][0], _skTemp7[1][1], 0.0, 0.0, 0.0, 1.0));
      _0_ok = _0_ok && all(mat4x4<f32>(_skTemp8[0][0], _skTemp8[0][1], _skTemp8[0][2], 0.0, _skTemp8[1][0], _skTemp8[1][1], _skTemp8[1][2], 0.0, _skTemp8[2][0], _skTemp8[2][1], _skTemp8[2][2], 0.0, 0.0, 0.0, 0.0, 1.0)[0] == vec4<f32>(1.0, 2.0, 0.0, 0.0));
      let _skTemp9 = mat2x2<f32>(_skUnpacked__globalUniforms_testMatrix2x2);
      let _skTemp10 = mat3x3<f32>(mat3x3<f32>(_skTemp9[0][0], _skTemp9[0][1], 0.0, _skTemp9[1][0], _skTemp9[1][1], 0.0, 0.0, 0.0, 1.0));
      _0_ok = _0_ok && all(mat4x4<f32>(_skTemp10[0][0], _skTemp10[0][1], _skTemp10[0][2], 0.0, _skTemp10[1][0], _skTemp10[1][1], _skTemp10[1][2], 0.0, _skTemp10[2][0], _skTemp10[2][1], _skTemp10[2][2], 0.0, 0.0, 0.0, 0.0, 1.0)[1] == vec4<f32>(3.0, 4.0, 0.0, 0.0));
    }
    var _skTemp11: vec4<f32>;
    var _skTemp12: bool;
    var _skTemp13: bool;
    var _skTemp14: bool;
    var _skTemp15: bool;
    var _skTemp16: bool;
    var _skTemp17: bool;
    var _skTemp18: bool;
    var _skTemp19: bool;
    if _0_ok {
      _skTemp19 = test_matrix_op_scalar_float_b();
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      _skTemp18 = test_matrix_op_scalar_half_b();
    } else {
      _skTemp18 = false;
    }
    if _skTemp18 {
      _skTemp17 = test_matrix_op_matrix_float_b();
    } else {
      _skTemp17 = false;
    }
    if _skTemp17 {
      _skTemp16 = test_matrix_op_matrix_half_b();
    } else {
      _skTemp16 = false;
    }
    if _skTemp16 {
      _skTemp15 = test_vector_op_matrix_float_b();
    } else {
      _skTemp15 = false;
    }
    if _skTemp15 {
      _skTemp14 = test_vector_op_matrix_half_b();
    } else {
      _skTemp14 = false;
    }
    if _skTemp14 {
      _skTemp13 = test_matrix_op_vector_float_b();
    } else {
      _skTemp13 = false;
    }
    if _skTemp13 {
      _skTemp12 = test_matrix_op_vector_half_b();
    } else {
      _skTemp12 = false;
    }
    if _skTemp12 {
      _skTemp11 = _globalUniforms.colorGreen;
    } else {
      _skTemp11 = _globalUniforms.colorRed;
    }
    return _skTemp11;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  _skInitializePolyfilledUniforms();
  return _skslMain(_coords);
}
struct _skRow2 {
  @align(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked__globalUniforms_testMatrix2x2: mat2x2<f32>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_testMatrix2x2 = mat2x2<f32>(_globalUniforms.testMatrix2x2.c[0].r, _globalUniforms.testMatrix2x2.c[1].r);
}
