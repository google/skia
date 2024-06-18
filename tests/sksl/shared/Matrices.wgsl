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
fn test_half_b() -> bool {
  {
    var ok: bool = true;
    var m1: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    let _skTemp0 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    ok = ok && (all(m1[0] == _skTemp0[0]) && all(m1[1] == _skTemp0[1]));
    var m3: mat2x2<f32> = m1;
    let _skTemp1 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    ok = ok && (all(m3[0] == _skTemp1[0]) && all(m3[1] == _skTemp1[1]));
    var m4: mat2x2<f32> = mat2x2<f32>(6.0, 0.0, 0.0, 6.0);
    let _skTemp2 = mat2x2<f32>(6.0, 0.0, 0.0, 6.0);
    ok = ok && (all(m4[0] == _skTemp2[0]) && all(m4[1] == _skTemp2[1]));
    m3 = m3 * m4;
    let _skTemp3 = mat2x2<f32>(6.0, 12.0, 18.0, 24.0);
    ok = ok && (all(m3[0] == _skTemp3[0]) && all(m3[1] == _skTemp3[1]));
    let _skTemp4 = m1[1].y;
    var m5: mat2x2<f32> = mat2x2<f32>(_skTemp4, 0.0, 0.0, _skTemp4);
    let _skTemp5 = mat2x2<f32>(4.0, 0.0, 0.0, 4.0);
    ok = ok && (all(m5[0] == _skTemp5[0]) && all(m5[1] == _skTemp5[1]));
    m1 = m1 + m5;
    let _skTemp6 = mat2x2<f32>(5.0, 2.0, 3.0, 8.0);
    ok = ok && (all(m1[0] == _skTemp6[0]) && all(m1[1] == _skTemp6[1]));
    var m7: mat2x2<f32> = mat2x2<f32>(5.0, 6.0, 7.0, 8.0);
    let _skTemp7 = mat2x2<f32>(5.0, 6.0, 7.0, 8.0);
    ok = ok && (all(m7[0] == _skTemp7[0]) && all(m7[1] == _skTemp7[1]));
    var m9: mat3x3<f32> = mat3x3<f32>(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, 9.0);
    let _skTemp8 = mat3x3<f32>(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, 9.0);
    ok = ok && (all(m9[0] == _skTemp8[0]) && all(m9[1] == _skTemp8[1]) && all(m9[2] == _skTemp8[2]));
    var m10: mat4x4<f32> = mat4x4<f32>(11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0);
    let _skTemp9 = mat4x4<f32>(11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0);
    ok = ok && (all(m10[0] == _skTemp9[0]) && all(m10[1] == _skTemp9[1]) && all(m10[2] == _skTemp9[2]) && all(m10[3] == _skTemp9[3]));
    var m11: mat4x4<f32> = mat4x4<f32>(20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0);
    m11 = m11 - m10;
    let _skTemp10 = mat4x4<f32>(9.0, 20.0, 20.0, 20.0, 20.0, 9.0, 20.0, 20.0, 20.0, 20.0, 9.0, 20.0, 20.0, 20.0, 20.0, 9.0);
    ok = ok && (all(m11[0] == _skTemp10[0]) && all(m11[1] == _skTemp10[1]) && all(m11[2] == _skTemp10[2]) && all(m11[3] == _skTemp10[3]));
    return ok;
  }
}
fn test_comma_b() -> bool {
  {
    var x: mat2x2<f32>;
    var y: mat2x2<f32>;
    x = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    y = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    return (all(x[0] == y[0]) && all(x[1] == y[1]));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_ok: bool = true;
    var _1_m1: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    let _skTemp11 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && (all(_1_m1[0] == _skTemp11[0]) && all(_1_m1[1] == _skTemp11[1]));
    var _2_m3: mat2x2<f32> = _1_m1;
    let _skTemp12 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && (all(_2_m3[0] == _skTemp12[0]) && all(_2_m3[1] == _skTemp12[1]));
    const _3_m4: mat2x2<f32> = mat2x2<f32>(6.0, 0.0, 0.0, 6.0);
    _2_m3 = _2_m3 * _3_m4;
    let _skTemp13 = mat2x2<f32>(6.0, 12.0, 18.0, 24.0);
    _0_ok = _0_ok && (all(_2_m3[0] == _skTemp13[0]) && all(_2_m3[1] == _skTemp13[1]));
    let _skTemp14 = _1_m1[1].y;
    var _4_m5: mat2x2<f32> = mat2x2<f32>(_skTemp14, 0.0, 0.0, _skTemp14);
    let _skTemp15 = mat2x2<f32>(4.0, 0.0, 0.0, 4.0);
    _0_ok = _0_ok && (all(_4_m5[0] == _skTemp15[0]) && all(_4_m5[1] == _skTemp15[1]));
    _1_m1 = _1_m1 + _4_m5;
    let _skTemp16 = mat2x2<f32>(5.0, 2.0, 3.0, 8.0);
    _0_ok = _0_ok && (all(_1_m1[0] == _skTemp16[0]) && all(_1_m1[1] == _skTemp16[1]));
    const _7_m10: mat4x4<f32> = mat4x4<f32>(11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0);
    var _8_m11: mat4x4<f32> = mat4x4<f32>(20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0);
    _8_m11 = _8_m11 - _7_m10;
    let _skTemp17 = mat4x4<f32>(9.0, 20.0, 20.0, 20.0, 20.0, 9.0, 20.0, 20.0, 20.0, 20.0, 9.0, 20.0, 20.0, 20.0, 20.0, 9.0);
    _0_ok = _0_ok && (all(_8_m11[0] == _skTemp17[0]) && all(_8_m11[1] == _skTemp17[1]) && all(_8_m11[2] == _skTemp17[2]) && all(_8_m11[3] == _skTemp17[3]));
    var _skTemp18: vec4<f32>;
    var _skTemp19: bool;
    var _skTemp20: bool;
    if _0_ok {
      let _skTemp21 = test_half_b();
      _skTemp20 = _skTemp21;
    } else {
      _skTemp20 = false;
    }
    if _skTemp20 {
      let _skTemp22 = test_comma_b();
      _skTemp19 = _skTemp22;
    } else {
      _skTemp19 = false;
    }
    if _skTemp19 {
      _skTemp18 = _globalUniforms.colorGreen;
    } else {
      _skTemp18 = _globalUniforms.colorRed;
    }
    return _skTemp18;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
