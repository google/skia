diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_eq_half_b() -> bool {
  {
    var ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_matrix_float_b() -> bool {
  {
    var ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_matrix_half_b() -> bool {
  {
    var ok: bool = true;
    return ok;
  }
}
fn test_vector_op_matrix_float_b() -> bool {
  {
    var ok: bool = true;
    return ok;
  }
}
fn test_vector_op_matrix_half_b() -> bool {
  {
    var ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_vector_float_b() -> bool {
  {
    var ok: bool = true;
    return ok;
  }
}
fn test_matrix_op_vector_half_b() -> bool {
  {
    var ok: bool = true;
    return ok;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const _0_ok: bool = true;
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    if _0_ok {
      let _skTemp8 = test_eq_half_b();
      _skTemp7 = _skTemp8;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp9 = test_matrix_op_matrix_float_b();
      _skTemp6 = _skTemp9;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp10 = test_matrix_op_matrix_half_b();
      _skTemp5 = _skTemp10;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp11 = test_vector_op_matrix_float_b();
      _skTemp4 = _skTemp11;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp12 = test_vector_op_matrix_half_b();
      _skTemp3 = _skTemp12;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp13 = test_matrix_op_vector_float_b();
      _skTemp2 = _skTemp13;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp14 = test_matrix_op_vector_half_b();
      _skTemp1 = _skTemp14;
    } else {
      _skTemp1 = false;
    }
    if _skTemp1 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
