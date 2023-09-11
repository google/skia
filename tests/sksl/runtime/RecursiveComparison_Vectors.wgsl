struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_same_vectors_bbffff(_skParam0: bool, _skParam1: f32, _skParam2: f32, _skParam3: f32, _skParam4: f32) -> bool {
  let eq = _skParam0;
  let f1 = _skParam1;
  let f2 = _skParam2;
  let f3 = _skParam3;
  let f4 = _skParam4;
  {
    var one: f32 = f32(_globalUniforms.colorGreen.x + 1.0);
    var a: vec4<f32> = vec4<f32>(f1, f2, f3, f4);
    var b: vec4<f32> = vec4<f32>(f1 * one, f2 * one, f3 * one, f4 * one);
    return select(any(a != b), all(a == b), eq);
  }
}
fn test_diff_vectors_bbffff(_skParam0: bool, _skParam1: f32, _skParam2: f32, _skParam3: f32, _skParam4: f32) -> bool {
  let eq = _skParam0;
  let f1 = _skParam1;
  let f2 = _skParam2;
  let f3 = _skParam3;
  let f4 = _skParam4;
  {
    var two: f32 = f32(_globalUniforms.colorGreen.x + 2.0);
    var a: vec4<f32> = vec4<f32>(f1, f2, f3, f4);
    var b: vec4<f32> = vec4<f32>(f1 * two, f2 * two, f3 * two, f4 * two);
    return select(any(a != b), all(a == b), eq);
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var NAN1: f32 = f32(_globalUniforms.colorGreen.x / _globalUniforms.colorGreen.z);
    var NAN2: f32 = f32(_globalUniforms.colorGreen.z / _globalUniforms.colorGreen.x);
    var ZP: f32 = f32(_globalUniforms.colorGreen.x * _globalUniforms.colorGreen.z);
    var ZM: f32 = f32(-_globalUniforms.colorGreen.x * _globalUniforms.colorGreen.z);
    var F42: f32 = f32(_globalUniforms.colorGreen.y * 42.0);
    var F43: f32 = f32(_globalUniforms.colorGreen.y * 43.0);
    var F44: f32 = f32(_globalUniforms.colorGreen.y * 44.0);
    var F45: f32 = f32(_globalUniforms.colorGreen.y * 45.0);
    var EQ: bool = true;
    var NE: bool = false;
    var _0_one: f32 = f32(_globalUniforms.colorGreen.x + 1.0);
    var _1_a: vec4<f32> = vec4<f32>(F42, ZM, ZP, F43);
    var _2_b: vec4<f32> = vec4<f32>(F42 * _0_one, ZM * _0_one, ZP * _0_one, F43 * _0_one);
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    if select(any(_1_a != _2_b), all(_1_a == _2_b), EQ) {
      let _skTemp8 = test_same_vectors_bbffff(NE, F42, ZM, ZP, F43);
      _skTemp7 = !_skTemp8;
    } else {
      _skTemp7 = false;
    }
    if _skTemp7 {
      let _skTemp9 = test_same_vectors_bbffff(NE, F42, NAN1, NAN2, F43);
      _skTemp6 = _skTemp9;
    } else {
      _skTemp6 = false;
    }
    if _skTemp6 {
      let _skTemp10 = test_same_vectors_bbffff(EQ, F42, NAN1, NAN2, F43);
      _skTemp5 = !_skTemp10;
    } else {
      _skTemp5 = false;
    }
    if _skTemp5 {
      let _skTemp11 = test_diff_vectors_bbffff(NE, F42, F43, F44, F45);
      _skTemp4 = _skTemp11;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp12 = test_diff_vectors_bbffff(EQ, F42, F43, F44, F45);
      _skTemp3 = !_skTemp12;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp13 = test_diff_vectors_bbffff(NE, NAN1, ZM, ZP, F42);
      _skTemp2 = _skTemp13;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp14 = test_diff_vectors_bbffff(EQ, NAN1, ZM, ZP, F42);
      _skTemp1 = !_skTemp14;
    } else {
      _skTemp1 = false;
    }
    if _skTemp1 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return vec4<f32>(_skTemp0);
  }
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return main(_coords);
}
