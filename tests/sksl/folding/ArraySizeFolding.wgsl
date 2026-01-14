diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
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
      _skTemp0 = check_array_is_float_3_bf(b);
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
    var _skTemp1: vec4<f32>;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    if check_array_is_int_2_bi(_3_b) {
      _skTemp6 = check_array_is_int_2_bi(_4_c);
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      _skTemp5 = check_array_is_int_2_bi(_5_d);
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      _skTemp4 = check_array_is_int_2_bi(_6_e);
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = check_array_is_int_2_bi(_7_f);
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = test_param_bff(f, g);
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = _globalUniforms.colorGreen;
    } else {
      _skTemp1 = _globalUniforms.colorRed;
    }
    return _skTemp1;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
