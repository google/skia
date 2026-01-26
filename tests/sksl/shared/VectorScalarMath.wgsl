diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  colorGreen: vec4<f16>,
  unknownInput: f16,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn test_int_b() -> bool {
  {
    var ok: bool = true;
    let inputRed: vec4<i32> = vec4<i32>(_globalUniforms.colorRed);
    let inputGreen: vec4<i32> = vec4<i32>(_globalUniforms.colorGreen);
    var x: vec4<i32> = inputRed + 2;
    ok = ok && all(x == vec4<i32>(3, 2, 2, 3));
    x = inputGreen.ywxz - 2;
    ok = ok && all(x == vec4<i32>(-1, -1, -2, -2));
    x = inputRed + inputGreen.y;
    ok = ok && all(x == vec4<i32>(2, 1, 1, 2));
    x = vec4<i32>((inputGreen.wyw * 9), x.w);
    ok = ok && all(x == vec4<i32>(9, 9, 9, 2));
    x = vec4<i32>((x.zw / 4), x.zw);
    ok = ok && all(x == vec4<i32>(2, 0, 9, 2));
    x = (inputRed * 5).yxwz;
    ok = ok && all(x == vec4<i32>(0, 5, 5, 0));
    x = 2 + inputRed;
    ok = ok && all(x == vec4<i32>(3, 2, 2, 3));
    x = 10 - inputGreen.ywxz;
    ok = ok && all(x == vec4<i32>(9, 9, 10, 10));
    x = inputRed.x + inputGreen;
    ok = ok && all(x == vec4<i32>(1, 2, 1, 2));
    x = vec4<i32>((8 * inputGreen.wyw), x.w);
    ok = ok && all(x == vec4<i32>(8, 8, 8, 2));
    x = vec4<i32>((36 / x.zw), x.zw);
    ok = ok && all(x == vec4<i32>(4, 18, 8, 2));
    x = (37 / x).yxwz;
    ok = ok && all(x == vec4<i32>(2, 9, 18, 4));
    x = x + 2;
    x = x * 2;
    x = x - 4;
    x = x / 2;
    ok = ok && all(x == vec4<i32>(2, 9, 18, 4));
    x = x + 2;
    x = x * 2;
    x = x - 4;
    x = x / 2;
    ok = ok && all(x == vec4<i32>(2, 9, 18, 4));
    return ok;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var _0_ok: bool = true;
    let _1_inputRed: vec4<f16> = _globalUniforms.colorRed;
    let _2_inputGreen: vec4<f16> = _globalUniforms.colorGreen;
    var _3_x: vec4<f16> = _1_inputRed + 2.0h;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(3.0h, 2.0h, 2.0h, 3.0h));
    _3_x = _2_inputGreen.ywxz - 2.0h;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(-1.0h, -1.0h, -2.0h, -2.0h));
    _3_x = _1_inputRed + _2_inputGreen.y;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(2.0h, 1.0h, 1.0h, 2.0h));
    _3_x = vec4<f16>((_2_inputGreen.wyw * 9.0h), _3_x.w);
    _0_ok = _0_ok && all(_3_x == vec4<f16>(9.0h, 9.0h, 9.0h, 2.0h));
    _3_x = vec4<f16>((_3_x.zw * 2.0h), _3_x.zw);
    _0_ok = _0_ok && all(_3_x == vec4<f16>(18.0h, 4.0h, 9.0h, 2.0h));
    _3_x = (_1_inputRed * 5.0h).yxwz;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(0.0h, 5.0h, 5.0h, 0.0h));
    _3_x = 2.0h + _1_inputRed;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(3.0h, 2.0h, 2.0h, 3.0h));
    _3_x = 10.0h - _2_inputGreen.ywxz;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(9.0h, 9.0h, 10.0h, 10.0h));
    _3_x = _1_inputRed.x + _2_inputGreen;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(1.0h, 2.0h, 1.0h, 2.0h));
    _3_x = vec4<f16>((8.0h * _2_inputGreen.wyw), _3_x.w);
    _0_ok = _0_ok && all(_3_x == vec4<f16>(8.0h, 8.0h, 8.0h, 2.0h));
    _3_x = vec4<f16>((32.0h / _3_x.zw), _3_x.zw);
    _0_ok = _0_ok && all(_3_x == vec4<f16>(4.0h, 16.0h, 8.0h, 2.0h));
    _3_x = (32.0h / _3_x).yxwz;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(2.0h, 8.0h, 16.0h, 4.0h));
    _3_x = _3_x + 2.0h;
    _3_x = _3_x * 2.0h;
    _3_x = _3_x - 4.0h;
    _3_x = _3_x * 0.5h;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(2.0h, 8.0h, 16.0h, 4.0h));
    _3_x = _3_x + 2.0h;
    _3_x = _3_x * 2.0h;
    _3_x = _3_x - 4.0h;
    _3_x = _3_x * 0.5h;
    _0_ok = _0_ok && all(_3_x == vec4<f16>(2.0h, 8.0h, 16.0h, 4.0h));
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
