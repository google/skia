diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
    f2 = f2 + vec2<f32>(1, 1);
    var _skTemp12: bool;
    if ok {
      let _skTemp13 = f2;
      f2 = f2 + vec2<f32>(1, 1);
      _skTemp12 = all(_skTemp13 == vec2<f32>(1.5));
    } else {
      _skTemp12 = false;
    }
    ok = _skTemp12;
    ok = ok && all(f2 == vec2<f32>(2.5));
    var _skTemp14: bool;
    if ok {
      let _skTemp15 = f2;
      f2 = f2 - vec2<f32>(1, 1);
      _skTemp14 = all(_skTemp15 == vec2<f32>(2.5));
    } else {
      _skTemp14 = false;
    }
    ok = _skTemp14;
    ok = ok && all(f2 == vec2<f32>(1.5));
    f2 = f2 - vec2<f32>(1, 1);
    ok = ok && all(f2 == vec2<f32>(0.5));
    var i4: vec4<i32> = vec4<i32>(7, 8, 9, 10);
    i4 = i4 + vec4<i32>(1, 1, 1, 1);
    var _skTemp16: bool;
    if ok {
      let _skTemp17 = i4;
      i4 = i4 + vec4<i32>(1, 1, 1, 1);
      _skTemp16 = all(_skTemp17 == vec4<i32>(8, 9, 10, 11));
    } else {
      _skTemp16 = false;
    }
    ok = _skTemp16;
    ok = ok && all(i4 == vec4<i32>(9, 10, 11, 12));
    var _skTemp18: bool;
    if ok {
      let _skTemp19 = i4;
      i4 = i4 - vec4<i32>(1, 1, 1, 1);
      _skTemp18 = all(_skTemp19 == vec4<i32>(9, 10, 11, 12));
    } else {
      _skTemp18 = false;
    }
    ok = _skTemp18;
    ok = ok && all(i4 == vec4<i32>(8, 9, 10, 11));
    i4 = i4 - vec4<i32>(1, 1, 1, 1);
    ok = ok && all(i4 == vec4<i32>(7, 8, 9, 10));
    var m3x3: mat3x3<f32> = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    m3x3 = m3x3 + mat3x3<f32>(1, 1, 1, 1, 1, 1, 1, 1, 1);
    var _skTemp20: bool;
    if ok {
      let _skTemp21 = m3x3;
      m3x3 = m3x3 + mat3x3<f32>(1, 1, 1, 1, 1, 1, 1, 1, 1);
      let _skTemp22 = _skTemp21;
      let _skTemp23 = mat3x3<f32>(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0);
      _skTemp20 = (all(_skTemp22[0] == _skTemp23[0]) && all(_skTemp22[1] == _skTemp23[1]) && all(_skTemp22[2] == _skTemp23[2]));
    } else {
      _skTemp20 = false;
    }
    ok = _skTemp20;
    let _skTemp24 = mat3x3<f32>(3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0);
    ok = ok && (all(m3x3[0] == _skTemp24[0]) && all(m3x3[1] == _skTemp24[1]) && all(m3x3[2] == _skTemp24[2]));
    var _skTemp25: bool;
    if ok {
      let _skTemp26 = m3x3;
      m3x3 = m3x3 - mat3x3<f32>(1, 1, 1, 1, 1, 1, 1, 1, 1);
      let _skTemp27 = _skTemp26;
      let _skTemp28 = mat3x3<f32>(3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0);
      _skTemp25 = (all(_skTemp27[0] == _skTemp28[0]) && all(_skTemp27[1] == _skTemp28[1]) && all(_skTemp27[2] == _skTemp28[2]));
    } else {
      _skTemp25 = false;
    }
    ok = _skTemp25;
    let _skTemp29 = mat3x3<f32>(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0);
    ok = ok && (all(m3x3[0] == _skTemp29[0]) && all(m3x3[1] == _skTemp29[1]) && all(m3x3[2] == _skTemp29[2]));
    m3x3 = m3x3 - mat3x3<f32>(1, 1, 1, 1, 1, 1, 1, 1, 1);
    let _skTemp30 = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    ok = ok && (all(m3x3[0] == _skTemp30[0]) && all(m3x3[1] == _skTemp30[1]) && all(m3x3[2] == _skTemp30[2]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
