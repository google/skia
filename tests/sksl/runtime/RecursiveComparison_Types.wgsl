diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  f1: f32,
  v2: vec2<f32>,
};
fn test_same_types_bbffffff(eq: bool, f1: f32, v2: f32, f3: f32, f4: f32, f5: f32, f6: f32) -> bool {
  {
    var one: f32 = f32(_globalUniforms.colorGreen.x + 1.0);
    var a: array<S, 2>;
    a[0].f1 = f1;
    a[0].v2 = vec2<f32>(v2, f3);
    a[1].f1 = f4;
    a[1].v2 = vec2<f32>(f5, f6);
    var b: array<S, 2>;
    b[0].f1 = f1 * one;
    b[0].v2 = vec2<f32>(v2 * one, f3 * one);
    b[1].f1 = f4 * one;
    b[1].v2 = vec2<f32>(f5 * one, f6 * one);
    var _skTemp0: bool;
    if eq {
      _skTemp0 = (((a[0].f1 == b[0].f1) && all(a[0].v2 == b[0].v2)) && ((a[1].f1 == b[1].f1) && all(a[1].v2 == b[1].v2)));
    } else {
      _skTemp0 = (((a[0].f1 != b[0].f1) || any(a[0].v2 != b[0].v2)) || ((a[1].f1 != b[1].f1) || any(a[1].v2 != b[1].v2)));
    }
    return _skTemp0;
  }
}
fn test_diff_types_bbffffff(eq: bool, f1: f32, v2: f32, f3: f32, f4: f32, f5: f32, f6: f32) -> bool {
  {
    var two: f32 = f32(_globalUniforms.colorGreen.x + 2.0);
    var a: array<S, 2>;
    a[0].f1 = f1;
    a[0].v2 = vec2<f32>(v2, f3);
    a[1].f1 = f4;
    a[1].v2 = vec2<f32>(f5, f6);
    var b: array<S, 2>;
    b[0].f1 = f1 * two;
    b[0].v2 = vec2<f32>(v2 * two, f3 * two);
    b[1].f1 = f4 * two;
    b[1].v2 = vec2<f32>(f5 * two, f6);
    var _skTemp1: bool;
    if eq {
      _skTemp1 = (((a[0].f1 == b[0].f1) && all(a[0].v2 == b[0].v2)) && ((a[1].f1 == b[1].f1) && all(a[1].v2 == b[1].v2)));
    } else {
      _skTemp1 = (((a[0].f1 != b[0].f1) || any(a[0].v2 != b[0].v2)) || ((a[1].f1 != b[1].f1) || any(a[1].v2 != b[1].v2)));
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
    var F45: f32 = f32(_globalUniforms.colorGreen.y * 45.0);
    var F46: f32 = f32(_globalUniforms.colorGreen.y * 46.0);
    var F47: f32 = f32(_globalUniforms.colorGreen.y * 47.0);
    var EQ: bool = true;
    var NE: bool = false;
    var _0_one: f32 = f32(_globalUniforms.colorGreen.x + 1.0);
    var _1_a: array<S, 2>;
    _1_a[0].f1 = F42;
    _1_a[0].v2 = vec2<f32>(ZM, ZP);
    _1_a[1].f1 = F43;
    _1_a[1].v2 = vec2<f32>(F44, F45);
    var _2_b: array<S, 2>;
    _2_b[0].f1 = F42 * _0_one;
    _2_b[0].v2 = vec2<f32>(ZM * _0_one, ZP * _0_one);
    _2_b[1].f1 = F43 * _0_one;
    _2_b[1].v2 = vec2<f32>(F44 * _0_one, F45 * _0_one);
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
      _skTemp10 = (((_1_a[0].f1 == _2_b[0].f1) && all(_1_a[0].v2 == _2_b[0].v2)) && ((_1_a[1].f1 == _2_b[1].f1) && all(_1_a[1].v2 == _2_b[1].v2)));
    } else {
      _skTemp10 = (((_1_a[0].f1 != _2_b[0].f1) || any(_1_a[0].v2 != _2_b[0].v2)) || ((_1_a[1].f1 != _2_b[1].f1) || any(_1_a[1].v2 != _2_b[1].v2)));
    }
    if _skTemp10 {
      let _skTemp11 = test_same_types_bbffffff(NE, F42, ZM, ZP, F43, F44, F45);
      _skTemp9 = !_skTemp11;
    } else {
      _skTemp9 = false;
    }
    if _skTemp9 {
      let _skTemp12 = test_same_types_bbffffff(NE, F42, NAN1, NAN2, F43, F44, F45);
      _skTemp8 = _skTemp12;
    } else {
      _skTemp8 = false;
    }
    if _skTemp8 {
      let _skTemp13 = test_same_types_bbffffff(EQ, F42, NAN1, NAN2, F43, F44, F45);
      _skTemp7 = !_skTemp13;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp14 = test_diff_types_bbffffff(NE, F42, F43, F44, F45, F46, F47);
      _skTemp6 = _skTemp14;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp15 = test_diff_types_bbffffff(EQ, F42, F43, F44, F45, F46, F47);
      _skTemp5 = !_skTemp15;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp16 = test_diff_types_bbffffff(NE, NAN1, ZM, ZP, F42, F43, F44);
      _skTemp4 = _skTemp16;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp17 = test_diff_types_bbffffff(EQ, NAN1, ZM, ZP, F42, F43, F44);
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
