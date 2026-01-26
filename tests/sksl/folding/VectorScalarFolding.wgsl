diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  colorGreen: vec4<f16>,
  unknownInput: f16,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn test_int_b() -> bool {
  {
    var ok: bool = true;
    var x: vec4<i32> = vec4<i32>(6, 6, 7, 8);
    ok = ok && all(x == vec4<i32>(6, 6, 7, 8));
    x = vec4<i32>(7, 9, 9, 9);
    ok = ok && all(x == vec4<i32>(7, 9, 9, 9));
    x = vec4<i32>(9, 9, 10, 10);
    ok = ok && all(x == vec4<i32>(9, 9, 10, 10));
    x = vec4<i32>((vec3<i32>(6)), x.w);
    ok = ok && all(x == vec4<i32>(6, 6, 6, 10));
    x = vec4<i32>((vec2<i32>(3)), x.zw);
    ok = ok && all(x == vec4<i32>(3, 3, 6, 10));
    x = vec4<i32>(6);
    ok = ok && all(x == vec4<i32>(6));
    x = vec4<i32>(6, 6, 7, 8);
    ok = ok && all(x == vec4<i32>(6, 6, 7, 8));
    x = vec4<i32>(-7, -9, -9, -9);
    ok = ok && all(x == vec4<i32>(-7, -9, -9, -9));
    x = vec4<i32>(9, 9, 10, 10);
    ok = ok && all(x == vec4<i32>(9, 9, 10, 10));
    x = vec4<i32>((vec3<i32>(6)), x.w);
    ok = ok && all(x == vec4<i32>(6, 6, 6, 10));
    x = vec4<i32>((vec2<i32>(8)), x.zw);
    ok = ok && all(x == vec4<i32>(8, 8, 6, 10));
    x = vec4<i32>(200, 100, 50, 25);
    ok = ok && all(x == vec4<i32>(200, 100, 50, 25));
    x = vec4<i32>(6);
    ok = ok && all(x == vec4<i32>(6));
    let unknown: i32 = i32(_globalUniforms.unknownInput);
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
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var _0_ok: bool = true;
    var _1_x: vec4<f16> = vec4<f16>(6.0h, 6.0h, 7.0h, 8.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(6.0h, 6.0h, 7.0h, 8.0h));
    _1_x = vec4<f16>(7.0h, 9.0h, 9.0h, 9.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(7.0h, 9.0h, 9.0h, 9.0h));
    _1_x = vec4<f16>(9.0h, 9.0h, 10.0h, 10.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(9.0h, 9.0h, 10.0h, 10.0h));
    _1_x = vec4<f16>((vec3<f16>(6.0h)), _1_x.w);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(6.0h, 6.0h, 6.0h, 10.0h));
    _1_x = vec4<f16>((vec2<f16>(3.0h)), _1_x.zw);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(3.0h, 3.0h, 6.0h, 10.0h));
    _1_x = vec4<f16>(6.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(6.0h));
    _1_x = vec4<f16>(6.0h, 6.0h, 7.0h, 8.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(6.0h, 6.0h, 7.0h, 8.0h));
    _1_x = vec4<f16>(-7.0h, -9.0h, -9.0h, -9.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(-7.0h, -9.0h, -9.0h, -9.0h));
    _1_x = vec4<f16>(9.0h, 9.0h, 10.0h, 10.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(9.0h, 9.0h, 10.0h, 10.0h));
    _1_x = vec4<f16>((vec3<f16>(6.0h)), _1_x.w);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(6.0h, 6.0h, 6.0h, 10.0h));
    _1_x = vec4<f16>((vec2<f16>(8.0h)), _1_x.zw);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(8.0h, 8.0h, 6.0h, 10.0h));
    _1_x = vec4<f16>(2.0h, 1.0h, 0.5h, 0.25h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(2.0h, 1.0h, 0.5h, 0.25h));
    _1_x = vec4<f16>(6.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(6.0h));
    let _2_unknown: f16 = _globalUniforms.unknownInput;
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(0.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(0.0h));
    _1_x = vec4<f16>(0.0h) / _2_unknown;
    _0_ok = _0_ok && all(_1_x == vec4<f16>(0.0h));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(0.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(0.0h));
    _1_x = 0.0h / vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(0.0h));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(0.0h);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(0.0h));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _1_x = _1_x + 1.0h;
    _1_x = _1_x - 1.0h;
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    _1_x = vec4<f16>(_2_unknown);
    _1_x = _1_x + 1.0h;
    _1_x = _1_x - 1.0h;
    _0_ok = _0_ok && all(_1_x == vec4<f16>(_2_unknown));
    var _skTemp0: vec4<f16>;
    var _skTemp1: bool;
    if _0_ok {
      _skTemp1 = test_int_b();
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
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f16> {
  return _skslMain(_coords);
}
