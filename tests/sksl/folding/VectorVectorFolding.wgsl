diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  unknownInput: f32,
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_int_b() -> bool {
  {
    var unknown: i32 = i32(_globalUniforms.unknownInput);
    var ok: bool = true;
    ok = ok && all((vec4<i32>(0) / vec4<i32>(unknown)) == vec4<i32>(0));
    var val: vec4<i32> = vec4<i32>(unknown);
    val = val + vec4<i32>(1);
    val = val - vec4<i32>(1);
    val = val + vec4<i32>(1);
    val = val - vec4<i32>(1);
    ok = ok && all(val == vec4<i32>(unknown));
    val = val * vec4<i32>(2);
    val = val / vec4<i32>(2);
    val = val * vec4<i32>(2);
    val = val / vec4<i32>(2);
    ok = ok && all(val == vec4<i32>(unknown));
    return ok;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_unknown: f32 = _globalUniforms.unknownInput;
    var _1_ok: bool = true;
    _1_ok = _1_ok && all((vec4<f32>(0.0) / vec4<f32>(_0_unknown)) == vec4<f32>(0.0));
    var _2_val: vec4<f32> = vec4<f32>(_0_unknown);
    _2_val = _2_val + vec4<f32>(1.0);
    _2_val = _2_val - vec4<f32>(1.0);
    _2_val = _2_val + vec4<f32>(1.0);
    _2_val = _2_val - vec4<f32>(1.0);
    _1_ok = _1_ok && all(_2_val == vec4<f32>(_0_unknown));
    _2_val = _2_val * vec4<f32>(2.0);
    _2_val = _2_val * vec4<f32>(0.5);
    _2_val = _2_val * vec4<f32>(2.0);
    _2_val = _2_val * vec4<f32>(0.5);
    _1_ok = _1_ok && all(_2_val == vec4<f32>(_0_unknown));
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    if _1_ok {
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
