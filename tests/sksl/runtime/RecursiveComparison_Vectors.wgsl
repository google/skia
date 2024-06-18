diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_same_vectors_bbffff(eq: bool, f1: f32, f2: f32, f3: f32, f4: f32) -> bool {
  {
    let one: f32 = f32(_globalUniforms.colorGreen.x + 1.0);
    let a: vec4<f32> = vec4<f32>(f1, f2, f3, f4);
    let b: vec4<f32> = vec4<f32>(f1 * one, f2 * one, f3 * one, f4 * one);
    var _skTemp0: bool;
    if eq {
      _skTemp0 = all(a == b);
    } else {
      _skTemp0 = any(a != b);
    }
    return _skTemp0;
  }
}
fn test_diff_vectors_bbffff(eq: bool, f1: f32, f2: f32, f3: f32, f4: f32) -> bool {
  {
    let two: f32 = f32(_globalUniforms.colorGreen.x + 2.0);
    let a: vec4<f32> = vec4<f32>(f1, f2, f3, f4);
    let b: vec4<f32> = vec4<f32>(f1 * two, f2 * two, f3 * two, f4 * two);
    var _skTemp1: bool;
    if eq {
      _skTemp1 = all(a == b);
    } else {
      _skTemp1 = any(a != b);
    }
    return _skTemp1;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let NAN1: f32 = f32(_globalUniforms.colorGreen.x / _globalUniforms.colorGreen.z);
    let NAN2: f32 = f32(_globalUniforms.colorGreen.z / _globalUniforms.colorGreen.x);
    let ZP: f32 = f32(_globalUniforms.colorGreen.x * _globalUniforms.colorGreen.z);
    let ZM: f32 = f32(-_globalUniforms.colorGreen.x * _globalUniforms.colorGreen.z);
    let F42: f32 = f32(_globalUniforms.colorGreen.y * 42.0);
    let F43: f32 = f32(_globalUniforms.colorGreen.y * 43.0);
    let F44: f32 = f32(_globalUniforms.colorGreen.y * 44.0);
    let F45: f32 = f32(_globalUniforms.colorGreen.y * 45.0);
    const EQ: bool = true;
    const NE: bool = false;
    let _0_one: f32 = f32(_globalUniforms.colorGreen.x + 1.0);
    let _1_a: vec4<f32> = vec4<f32>(F42, ZM, ZP, F43);
    let _2_b: vec4<f32> = vec4<f32>(F42 * _0_one, ZM * _0_one, ZP * _0_one, F43 * _0_one);
    var _skTemp2: vec4<f32>;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    var _skTemp8: bool;
    var _skTemp9: bool;
    var _skTemp10: bool;
    if EQ {
      _skTemp10 = all(_1_a == _2_b);
    } else {
      _skTemp10 = any(_1_a != _2_b);
    }
    if _skTemp10 {
      let _skTemp11 = test_same_vectors_bbffff(NE, F42, ZM, ZP, F43);
      _skTemp9 = !_skTemp11;
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      let _skTemp12 = test_same_vectors_bbffff(NE, F42, NAN1, NAN2, F43);
      _skTemp8 = _skTemp12;
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      let _skTemp13 = test_same_vectors_bbffff(EQ, F42, NAN1, NAN2, F43);
      _skTemp7 = !_skTemp13;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp14 = test_diff_vectors_bbffff(NE, F42, F43, F44, F45);
      _skTemp6 = _skTemp14;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp15 = test_diff_vectors_bbffff(EQ, F42, F43, F44, F45);
      _skTemp5 = !_skTemp15;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp16 = test_diff_vectors_bbffff(NE, NAN1, ZM, ZP, F42);
      _skTemp4 = _skTemp16;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp17 = test_diff_vectors_bbffff(EQ, NAN1, ZM, ZP, F42);
      _skTemp3 = !_skTemp17;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = _globalUniforms.colorGreen;
    } else {
      _skTemp2 = _globalUniforms.colorRed;
    }
    return vec4<f32>(_skTemp2);
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
