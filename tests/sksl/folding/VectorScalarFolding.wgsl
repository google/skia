diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_int_b() -> bool {
  {
    var ok: bool = true;
    var x: vec4<i32> = vec4<i32>(6, 6, 7, 8);
    ok = ok && all(x == vec4<i32>(6, 6, 7, 8));
    x = vec4<i32>(7, 9, 9, 9);
    ok = ok && all(x == vec4<i32>(7, 9, 9, 9));
    x = vec4<i32>(9, 9, 10, 10);
    ok = ok && all(x == vec4<i32>(9, 9, 10, 10));
    x = vec4<i32>((vec3<i32>(6)), x.w).xyzw;
    ok = ok && all(x == vec4<i32>(6, 6, 6, 10));
    x = vec4<i32>((vec2<i32>(3)), x.zw).xyzw;
    ok = ok && all(x == vec4<i32>(3, 3, 6, 10));
    x = vec4<i32>(6);
    ok = ok && all(x == vec4<i32>(6));
    x = vec4<i32>(6, 6, 7, 8);
    ok = ok && all(x == vec4<i32>(6, 6, 7, 8));
    x = vec4<i32>(-7, -9, -9, -9);
    ok = ok && all(x == vec4<i32>(-7, -9, -9, -9));
    x = vec4<i32>(9, 9, 10, 10);
    ok = ok && all(x == vec4<i32>(9, 9, 10, 10));
    x = vec4<i32>((vec3<i32>(6)), x.w).xyzw;
    ok = ok && all(x == vec4<i32>(6, 6, 6, 10));
    x = vec4<i32>((vec2<i32>(8)), x.zw).xyzw;
    ok = ok && all(x == vec4<i32>(8, 8, 6, 10));
    x = vec4<i32>(200, 100, 50, 25);
    ok = ok && all(x == vec4<i32>(200, 100, 50, 25));
    x = vec4<i32>(6);
    ok = ok && all(x == vec4<i32>(6));
    var unknown: i32 = i32(_globalUniforms.unknownInput);
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(0);
    ok = ok && all(x == vec4<i32>(0));
    x = vec4<i32>(0) / unknown;
    ok = ok && all(x == vec4<i32>(0));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(0);
    ok = ok && all(x == vec4<i32>(0));
    x = 0 / vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(0));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(0);
    ok = ok && all(x == vec4<i32>(0));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    x = x + 1;
    x = x - 1;
    ok = ok && all(x == vec4<i32>(unknown));
    x = vec4<i32>(unknown);
    x = x + 1;
    x = x - 1;
    ok = ok && all(x == vec4<i32>(unknown));
    return ok;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_ok: bool = true;
    var _1_x: vec4<f32> = vec4<f32>(6.0, 6.0, 7.0, 8.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(6.0, 6.0, 7.0, 8.0));
    _1_x = vec4<f32>(7.0, 9.0, 9.0, 9.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(7.0, 9.0, 9.0, 9.0));
    _1_x = vec4<f32>(9.0, 9.0, 10.0, 10.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(9.0, 9.0, 10.0, 10.0));
    _1_x = vec4<f32>((vec3<f32>(6.0)), _1_x.w).xyzw;
    _0_ok = _0_ok && all(_1_x == vec4<f32>(6.0, 6.0, 6.0, 10.0));
    _1_x = vec4<f32>((vec2<f32>(3.0)), _1_x.zw).xyzw;
    _0_ok = _0_ok && all(_1_x == vec4<f32>(3.0, 3.0, 6.0, 10.0));
    _1_x = vec4<f32>(6.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(6.0));
    _1_x = vec4<f32>(6.0, 6.0, 7.0, 8.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(6.0, 6.0, 7.0, 8.0));
    _1_x = vec4<f32>(-7.0, -9.0, -9.0, -9.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(-7.0, -9.0, -9.0, -9.0));
    _1_x = vec4<f32>(9.0, 9.0, 10.0, 10.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(9.0, 9.0, 10.0, 10.0));
    _1_x = vec4<f32>((vec3<f32>(6.0)), _1_x.w).xyzw;
    _0_ok = _0_ok && all(_1_x == vec4<f32>(6.0, 6.0, 6.0, 10.0));
    _1_x = vec4<f32>((vec2<f32>(8.0)), _1_x.zw).xyzw;
    _0_ok = _0_ok && all(_1_x == vec4<f32>(8.0, 8.0, 6.0, 10.0));
    _1_x = vec4<f32>(2.0, 1.0, 0.5, 0.25);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(2.0, 1.0, 0.5, 0.25));
    _1_x = vec4<f32>(6.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(6.0));
    var _2_unknown: f32 = _globalUniforms.unknownInput;
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(0.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(0.0));
    _1_x = vec4<f32>(0.0) / _2_unknown;
    _0_ok = _0_ok && all(_1_x == vec4<f32>(0.0));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(0.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(0.0));
    _1_x = 0.0 / vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(0.0));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(0.0);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(0.0));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _1_x = _1_x + 1.0;
    _1_x = _1_x - 1.0;
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    _1_x = vec4<f32>(_2_unknown);
    _1_x = _1_x + 1.0;
    _1_x = _1_x - 1.0;
    _0_ok = _0_ok && all(_1_x == vec4<f32>(_2_unknown));
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    if _0_ok {
      let _skTemp2 = test_int_b();
      _skTemp1 = _skTemp2;
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
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
