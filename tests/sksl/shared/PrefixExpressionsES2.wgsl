diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
    f2 = f2 + vec2<f32>(1, 1);
    ok = ok && all(f2 == vec2<f32>(1.5));
    var _skTemp6: bool;
    if ok {
      f2 = f2 + vec2<f32>(1, 1);
      _skTemp6 = all(f2 == vec2<f32>(2.5));
    } else {
      _skTemp6 = false;
    }
    ok = _skTemp6;
    var _skTemp7: bool;
    if ok {
      f2 = f2 - vec2<f32>(1, 1);
      _skTemp7 = all(f2 == vec2<f32>(1.5));
    } else {
      _skTemp7 = false;
    }
    ok = _skTemp7;
    f2 = f2 - vec2<f32>(1, 1);
    ok = ok && all(f2 == vec2<f32>(0.5));
    var i4: vec4<i32> = vec4<i32>(7, 8, 9, 10);
    i4 = i4 + vec4<i32>(1, 1, 1, 1);
    ok = ok && all(i4 == vec4<i32>(8, 9, 10, 11));
    var _skTemp8: bool;
    if ok {
      i4 = i4 + vec4<i32>(1, 1, 1, 1);
      _skTemp8 = all(i4 == vec4<i32>(9, 10, 11, 12));
    } else {
      _skTemp8 = false;
    }
    ok = _skTemp8;
    var _skTemp9: bool;
    if ok {
      i4 = i4 - vec4<i32>(1, 1, 1, 1);
      _skTemp9 = all(i4 == vec4<i32>(8, 9, 10, 11));
    } else {
      _skTemp9 = false;
    }
    ok = _skTemp9;
    i4 = i4 - vec4<i32>(1, 1, 1, 1);
    ok = ok && all(i4 == vec4<i32>(7, 8, 9, 10));
    var m3x3: mat3x3<f32> = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    m3x3 = m3x3 + mat3x3<f32>(1, 1, 1, 1, 1, 1, 1, 1, 1);
    let _skTemp10 = mat3x3<f32>(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0);
    ok = ok && (all(m3x3[0] == _skTemp10[0]) && all(m3x3[1] == _skTemp10[1]) && all(m3x3[2] == _skTemp10[2]));
    var _skTemp11: bool;
    if ok {
      m3x3 = m3x3 + mat3x3<f32>(1, 1, 1, 1, 1, 1, 1, 1, 1);
      let _skTemp12 = m3x3;
      let _skTemp13 = mat3x3<f32>(3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0);
      _skTemp11 = (all(_skTemp12[0] == _skTemp13[0]) && all(_skTemp12[1] == _skTemp13[1]) && all(_skTemp12[2] == _skTemp13[2]));
    } else {
      _skTemp11 = false;
    }
    ok = _skTemp11;
    var _skTemp14: bool;
    if ok {
      m3x3 = m3x3 - mat3x3<f32>(1, 1, 1, 1, 1, 1, 1, 1, 1);
      let _skTemp15 = m3x3;
      let _skTemp16 = mat3x3<f32>(2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0);
      _skTemp14 = (all(_skTemp15[0] == _skTemp16[0]) && all(_skTemp15[1] == _skTemp16[1]) && all(_skTemp15[2] == _skTemp16[2]));
    } else {
      _skTemp14 = false;
    }
    ok = _skTemp14;
    m3x3 = m3x3 - mat3x3<f32>(1, 1, 1, 1, 1, 1, 1, 1, 1);
    let _skTemp17 = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    ok = ok && (all(m3x3[0] == _skTemp17[0]) && all(m3x3[1] == _skTemp17[1]) && all(m3x3[2] == _skTemp17[2]));
    ok = ok && (_globalUniforms.colorGreen.x != 1.0);
    ok = ok && (-1.0 == (-_globalUniforms.colorGreen.y));
    ok = ok && all(vec4<f32>(0.0, -1.0, 0.0, -1.0) == (-_globalUniforms.colorGreen));
    let _skTemp18 = mat2x2<f32>(-1.0, -2.0, -3.0, -4.0);
    let _skTemp19 = (-1.0 * _globalUniforms.testMatrix2x2);
    ok = ok && (all(_skTemp18[0] == _skTemp19[0]) && all(_skTemp18[1] == _skTemp19[1]));
    var iv: vec2<i32> = vec2<i32>(i, -i);
    ok = ok && ((-i) == -5);
    ok = ok && all((-iv) == vec2<i32>(-5, 5));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
