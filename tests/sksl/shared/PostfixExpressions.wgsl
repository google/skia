diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(c: vec2<f32>) -> vec4<f32> {
  {
    var ok: bool = true;
    var i: i32 = 5;
    i = i + i32(1);
    var _skTemp0: bool;
    if ok {
      let _skTemp1 = i;
      i = i + i32(1);
      _skTemp0 = (_skTemp1 == 6);
    } else {
      _skTemp0 = false;
    }
    ok = _skTemp0;
    ok = ok && (i == 7);
    var _skTemp2: bool;
    if ok {
      let _skTemp3 = i;
      i = i - i32(1);
      _skTemp2 = (_skTemp3 == 7);
    } else {
      _skTemp2 = false;
    }
    ok = _skTemp2;
    ok = ok && (i == 6);
    i = i - i32(1);
    ok = ok && (i == 5);
    var f: f32 = 0.5;
    f = f + f32(1);
    var _skTemp4: bool;
    if ok {
      let _skTemp5 = f;
      f = f + f32(1);
      _skTemp4 = (_skTemp5 == 1.5);
    } else {
      _skTemp4 = false;
    }
    ok = _skTemp4;
    ok = ok && (f == 2.5);
    var _skTemp6: bool;
    if ok {
      let _skTemp7 = f;
      f = f - f32(1);
      _skTemp6 = (_skTemp7 == 2.5);
    } else {
      _skTemp6 = false;
    }
    ok = _skTemp6;
    ok = ok && (f == 1.5);
    f = f - f32(1);
    ok = ok && (f == 0.5);
    var f2: vec2<f32> = vec2<f32>(0.5);
    f2.x = f2.x + f32(1);
    var _skTemp8: bool;
    if ok {
      let _skTemp9 = f2.x;
      f2.x = f2.x + f32(1);
      _skTemp8 = (_skTemp9 == 1.5);
    } else {
      _skTemp8 = false;
    }
    ok = _skTemp8;
    ok = ok && (f2.x == 2.5);
    var _skTemp10: bool;
    if ok {
      let _skTemp11 = f2.x;
      f2.x = f2.x - f32(1);
      _skTemp10 = (_skTemp11 == 2.5);
    } else {
      _skTemp10 = false;
    }
    ok = _skTemp10;
    ok = ok && (f2.x == 1.5);
    f2.x = f2.x - f32(1);
    ok = ok && (f2.x == 0.5);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
