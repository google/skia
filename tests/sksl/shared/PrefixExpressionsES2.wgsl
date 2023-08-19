diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testMatrix2x2: mat2x2<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    var ok: bool = true;
    var i: i32 = 5;
    i = i + i32(1);
    ok = ok && (i == 6);
    var _skTemp0: bool;
    if ok {
      i = i + i32(1);
      _skTemp0 = (i == 7);
    } else {
      _skTemp0 = false;
    }
    ok = _skTemp0;
    var _skTemp1: bool;
    if ok {
      i = i - i32(1);
      _skTemp1 = (i == 6);
    } else {
      _skTemp1 = false;
    }
    ok = _skTemp1;
    i = i - i32(1);
    ok = ok && (i == 5);
    var f: f32 = 0.5;
    f = f + f32(1);
    ok = ok && (f == 1.5);
    var _skTemp2: bool;
    if ok {
      f = f + f32(1);
      _skTemp2 = (f == 2.5);
    } else {
      _skTemp2 = false;
    }
    ok = _skTemp2;
    var _skTemp3: bool;
    if ok {
      f = f - f32(1);
      _skTemp3 = (f == 1.5);
    } else {
      _skTemp3 = false;
    }
    ok = _skTemp3;
    f = f - f32(1);
    ok = ok && (f == 0.5);
    var f2: vec2<f32> = vec2<f32>(0.5);
    f2.x = f2.x + f32(1);
    ok = ok && (f2.x == 1.5);
    var _skTemp4: bool;
    if ok {
      f2.x = f2.x + f32(1);
      _skTemp4 = (f2.x == 2.5);
    } else {
      _skTemp4 = false;
    }
    ok = _skTemp4;
    var _skTemp5: bool;
    if ok {
      f2.x = f2.x - f32(1);
      _skTemp5 = (f2.x == 1.5);
    } else {
      _skTemp5 = false;
    }
    ok = _skTemp5;
    f2.x = f2.x - f32(1);
    ok = ok && (f2.x == 0.5);
    ok = ok && (_globalUniforms.colorGreen.x != 1.0);
    ok = ok && (-1.0 == (-_globalUniforms.colorGreen.y));
    ok = ok && all(vec4<f32>(0.0, -1.0, 0.0, -1.0) == (-_globalUniforms.colorGreen));
    let _skTemp6 = mat2x2<f32>(-1.0, -2.0, -3.0, -4.0);
    let _skTemp7 = (-1.0 * _globalUniforms.testMatrix2x2);
    ok = ok && (all(_skTemp6[0] == _skTemp7[0]) && all(_skTemp6[1] == _skTemp7[1]));
    var iv: vec2<i32> = vec2<i32>(i, -i);
    ok = ok && ((-i) == -5);
    ok = ok && all((-iv) == vec2<i32>(-5, 5));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
