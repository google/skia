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
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_half_b() -> bool {
  {
    var ok: bool = true;
    var m1: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    let _skTemp0 = m1;
    let _skTemp1 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    ok = ok && (all(_skTemp0[0] == _skTemp1[0]) && all(_skTemp0[1] == _skTemp1[1]));
    var m3: mat2x2<f32> = m1;
    let _skTemp2 = m3;
    let _skTemp3 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    ok = ok && (all(_skTemp2[0] == _skTemp3[0]) && all(_skTemp2[1] == _skTemp3[1]));
    var m4: mat2x2<f32> = mat2x2<f32>(6.0, 0.0, 0.0, 6.0);
    let _skTemp4 = m4;
    let _skTemp5 = mat2x2<f32>(6.0, 0.0, 0.0, 6.0);
    ok = ok && (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1]));
    m3 = m3 * m4;
    let _skTemp6 = m3;
    let _skTemp7 = mat2x2<f32>(6.0, 12.0, 18.0, 24.0);
    ok = ok && (all(_skTemp6[0] == _skTemp7[0]) && all(_skTemp6[1] == _skTemp7[1]));
    let _skTemp8 = m1[1].y;
    var m5: mat2x2<f32> = mat2x2<f32>(_skTemp8, 0.0, 0.0, _skTemp8);
    let _skTemp9 = m5;
    let _skTemp10 = mat2x2<f32>(4.0, 0.0, 0.0, 4.0);
    ok = ok && (all(_skTemp9[0] == _skTemp10[0]) && all(_skTemp9[1] == _skTemp10[1]));
    m1 = m1 + m5;
    let _skTemp11 = m1;
    let _skTemp12 = mat2x2<f32>(5.0, 2.0, 3.0, 8.0);
    ok = ok && (all(_skTemp11[0] == _skTemp12[0]) && all(_skTemp11[1] == _skTemp12[1]));
    var m7: mat2x2<f32> = mat2x2<f32>(5.0, 6.0, 7.0, 8.0);
    let _skTemp13 = m7;
    let _skTemp14 = mat2x2<f32>(5.0, 6.0, 7.0, 8.0);
    ok = ok && (all(_skTemp13[0] == _skTemp14[0]) && all(_skTemp13[1] == _skTemp14[1]));
    var m9: mat3x3<f32> = mat3x3<f32>(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, 9.0);
    let _skTemp15 = m9;
    let _skTemp16 = mat3x3<f32>(9.0, 0.0, 0.0, 0.0, 9.0, 0.0, 0.0, 0.0, 9.0);
    ok = ok && (all(_skTemp15[0] == _skTemp16[0]) && all(_skTemp15[1] == _skTemp16[1]) && all(_skTemp15[2] == _skTemp16[2]));
    var m10: mat4x4<f32> = mat4x4<f32>(11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0);
    let _skTemp17 = m10;
    let _skTemp18 = mat4x4<f32>(11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0);
    ok = ok && (all(_skTemp17[0] == _skTemp18[0]) && all(_skTemp17[1] == _skTemp18[1]) && all(_skTemp17[2] == _skTemp18[2]) && all(_skTemp17[3] == _skTemp18[3]));
    var m11: mat4x4<f32> = mat4x4<f32>(20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0);
    m11 = m11 - m10;
    let _skTemp19 = m11;
    let _skTemp20 = mat4x4<f32>(9.0, 20.0, 20.0, 20.0, 20.0, 9.0, 20.0, 20.0, 20.0, 20.0, 9.0, 20.0, 20.0, 20.0, 20.0, 9.0);
    ok = ok && (all(_skTemp19[0] == _skTemp20[0]) && all(_skTemp19[1] == _skTemp20[1]) && all(_skTemp19[2] == _skTemp20[2]) && all(_skTemp19[3] == _skTemp20[3]));
    return ok;
  }
}
fn test_comma_b() -> bool {
  {
    var x: mat2x2<f32>;
    var y: mat2x2<f32>;
    x = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    y = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    let _skTemp21 = x;
    let _skTemp22 = y;
    return (all(_skTemp21[0] == _skTemp22[0]) && all(_skTemp21[1] == _skTemp22[1]));
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var _0_ok: bool = true;
    var _1_m1: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    let _skTemp23 = _1_m1;
    let _skTemp24 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && (all(_skTemp23[0] == _skTemp24[0]) && all(_skTemp23[1] == _skTemp24[1]));
    var _2_m3: mat2x2<f32> = _1_m1;
    let _skTemp25 = _2_m3;
    let _skTemp26 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && (all(_skTemp25[0] == _skTemp26[0]) && all(_skTemp25[1] == _skTemp26[1]));
    let _3_m4: mat2x2<f32> = mat2x2<f32>(6.0, 0.0, 0.0, 6.0);
    _2_m3 = _2_m3 * _3_m4;
    let _skTemp27 = _2_m3;
    let _skTemp28 = mat2x2<f32>(6.0, 12.0, 18.0, 24.0);
    _0_ok = _0_ok && (all(_skTemp27[0] == _skTemp28[0]) && all(_skTemp27[1] == _skTemp28[1]));
    let _skTemp29 = _1_m1[1].y;
    var _4_m5: mat2x2<f32> = mat2x2<f32>(_skTemp29, 0.0, 0.0, _skTemp29);
    let _skTemp30 = _4_m5;
    let _skTemp31 = mat2x2<f32>(4.0, 0.0, 0.0, 4.0);
    _0_ok = _0_ok && (all(_skTemp30[0] == _skTemp31[0]) && all(_skTemp30[1] == _skTemp31[1]));
    _1_m1 = _1_m1 + _4_m5;
    let _skTemp32 = _1_m1;
    let _skTemp33 = mat2x2<f32>(5.0, 2.0, 3.0, 8.0);
    _0_ok = _0_ok && (all(_skTemp32[0] == _skTemp33[0]) && all(_skTemp32[1] == _skTemp33[1]));
    let _7_m10: mat4x4<f32> = mat4x4<f32>(11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0, 0.0, 0.0, 0.0, 0.0, 11.0);
    var _8_m11: mat4x4<f32> = mat4x4<f32>(20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0);
    _8_m11 = _8_m11 - _7_m10;
    let _skTemp34 = _8_m11;
    let _skTemp35 = mat4x4<f32>(9.0, 20.0, 20.0, 20.0, 20.0, 9.0, 20.0, 20.0, 20.0, 20.0, 9.0, 20.0, 20.0, 20.0, 20.0, 9.0);
    _0_ok = _0_ok && (all(_skTemp34[0] == _skTemp35[0]) && all(_skTemp34[1] == _skTemp35[1]) && all(_skTemp34[2] == _skTemp35[2]) && all(_skTemp34[3] == _skTemp35[3]));
    var _skTemp36: vec4<f32>;
    var _skTemp37: bool;
    var _skTemp38: bool;
    if _0_ok {
      let _skTemp39 = test_half_b();
      _skTemp38 = _skTemp39;
    } else {
      _skTemp38 = false;
    }
    if _skTemp38 {
      let _skTemp40 = test_comma_b();
      _skTemp37 = _skTemp40;
    } else {
      _skTemp37 = false;
    }
    if _skTemp37 {
      _skTemp36 = _globalUniforms.colorGreen;
    } else {
      _skTemp36 = _globalUniforms.colorRed;
    }
    return _skTemp36;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
