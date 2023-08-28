diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
struct S {
  a: i32,
  b: i32,
  c: i32,
};
var<private> numSideEffects: i32 = 0;
fn side_effecting_ii(value: i32) -> i32 {
  {
    numSideEffects = numSideEffects + i32(1);
    return value;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_val1: i32 = 2;
    var _1_val2: i32 = 1;
    _0_val1 = _0_val1 - i32(1);
    let _skTemp0 = side_effecting_ii(2);
    var _2_noFlatten0: i32 = S(_0_val1, _skTemp0, 3).a;
    let _skTemp1 = side_effecting_ii(1);
    var _3_noFlatten1: i32 = S(_skTemp1, 2, 3).b;
    _1_val2 = _1_val2 + 1;
    var _4_noFlatten2: i32 = S(1, _1_val2, 3).c;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((_2_noFlatten0 == 1) && (_3_noFlatten1 == 2)) && (_4_noFlatten2 == 3)) && (_0_val1 == 1)) && (_1_val2 == 2)) && (numSideEffects == 2)));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
