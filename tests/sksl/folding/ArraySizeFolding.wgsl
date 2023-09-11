diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn check_array_is_int_2_bi(x: array<i32, 2>) -> bool {
  {
    return true;
  }
}
fn check_array_is_float_3_bf(x: array<f32, 3>) -> bool {
  {
    return true;
  }
}
fn test_param_bff(a: array<f32, 3>, b: array<f32, 3>) -> bool {
  {
    var _skTemp0: bool;
    if true {
      let _skTemp1 = check_array_is_float_3_bf(b);
      _skTemp0 = _skTemp1;
    } else {
      _skTemp0 = false;
    }
    return _skTemp0;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var f: array<f32, 3>;
    var g: array<f32, 3>;
    var _3_b: array<i32, 2>;
    var _4_c: array<i32, 2>;
    var _5_d: array<i32, 2>;
    var _6_e: array<i32, 2>;
    var _7_f: array<i32, 2>;
    var _skTemp2: vec4<f32>;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    let _skTemp8 = check_array_is_int_2_bi(_3_b);
    if _skTemp8 {
      let _skTemp9 = check_array_is_int_2_bi(_4_c);
      _skTemp7 = _skTemp9;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp10 = check_array_is_int_2_bi(_5_d);
      _skTemp6 = _skTemp10;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp11 = check_array_is_int_2_bi(_6_e);
      _skTemp5 = _skTemp11;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp12 = check_array_is_int_2_bi(_7_f);
      _skTemp4 = _skTemp12;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp13 = test_param_bff(f, g);
      _skTemp3 = _skTemp13;
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
