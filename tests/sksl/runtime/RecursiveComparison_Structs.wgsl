diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  f1: f32,
  f2: f32,
  f3: f32,
};
fn test_same_structs_bbfff(eq: bool, f1: f32, f2: f32, f3: f32) -> bool {
  {
    var one: f32 = f32(_globalUniforms.colorGreen.x + 1.0);
    var a: S;
    a.f1 = f1;
    a.f2 = f2;
    a.f3 = f3;
    var b: S;
    b.f1 = f1 * one;
    b.f2 = f2 * one;
    b.f3 = f3 * one;
    var _skTemp0: bool;
    if eq {
      _skTemp0 = ((a.f1 == b.f1) && (a.f2 == b.f2) && (a.f3 == b.f3));
    } else {
      _skTemp0 = ((a.f1 != b.f1) || (a.f2 != b.f2) || (a.f3 != b.f3));
    }
    return _skTemp0;
  }
}
fn test_diff_structs_bbfff(eq: bool, f1: f32, f2: f32, f3: f32) -> bool {
  {
    var two: f32 = f32(_globalUniforms.colorGreen.x + 2.0);
    var a: S;
    a.f1 = f1;
    a.f2 = f2;
    a.f3 = f3;
    var b: S;
    b.f1 = f1 * two;
    b.f2 = f2 * two;
    b.f3 = f3;
    var _skTemp1: bool;
    if eq {
      _skTemp1 = ((a.f1 == b.f1) && (a.f2 == b.f2) && (a.f3 == b.f3));
    } else {
      _skTemp1 = ((a.f1 != b.f1) || (a.f2 != b.f2) || (a.f3 != b.f3));
    }
    return _skTemp1;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var NAN1: f32 = f32(_globalUniforms.colorGreen.x / _globalUniforms.colorGreen.z);
    var NAN2: f32 = f32(_globalUniforms.colorGreen.z / _globalUniforms.colorGreen.x);
    var ZP: f32 = f32(_globalUniforms.colorGreen.x * _globalUniforms.colorGreen.z);
    var ZM: f32 = f32(-_globalUniforms.colorGreen.x * _globalUniforms.colorGreen.z);
    var F42: f32 = f32(_globalUniforms.colorGreen.y * 42.0);
    var F43: f32 = f32(_globalUniforms.colorGreen.y * 43.0);
    var F44: f32 = f32(_globalUniforms.colorGreen.y * 44.0);
    var EQ: bool = true;
    var NE: bool = false;
    var _0_one: f32 = f32(_globalUniforms.colorGreen.x + 1.0);
    var _1_a: S;
    _1_a.f1 = F42;
    _1_a.f2 = ZM;
    _1_a.f3 = ZP;
    var _2_b: S;
    _2_b.f1 = F42 * _0_one;
    _2_b.f2 = ZM * _0_one;
    _2_b.f3 = ZP * _0_one;
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
      _skTemp10 = ((_1_a.f1 == _2_b.f1) && (_1_a.f2 == _2_b.f2) && (_1_a.f3 == _2_b.f3));
    } else {
      _skTemp10 = ((_1_a.f1 != _2_b.f1) || (_1_a.f2 != _2_b.f2) || (_1_a.f3 != _2_b.f3));
    }
    if _skTemp10 {
      let _skTemp11 = test_same_structs_bbfff(NE, F42, ZM, ZP);
      _skTemp9 = !_skTemp11;
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      let _skTemp12 = test_same_structs_bbfff(NE, F42, NAN1, NAN2);
      _skTemp8 = _skTemp12;
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      let _skTemp13 = test_same_structs_bbfff(EQ, F42, NAN1, NAN2);
      _skTemp7 = !_skTemp13;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp14 = test_diff_structs_bbfff(NE, F42, F43, F44);
      _skTemp6 = _skTemp14;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp15 = test_diff_structs_bbfff(EQ, F42, F43, F44);
      _skTemp5 = !_skTemp15;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp16 = test_diff_structs_bbfff(NE, NAN1, ZM, ZP);
      _skTemp4 = _skTemp16;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp17 = test_diff_structs_bbfff(EQ, NAN1, ZM, ZP);
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
