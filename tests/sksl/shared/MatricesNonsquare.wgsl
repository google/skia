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
    var m23: mat2x3<f32> = mat2x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
    let _skTemp0 = mat2x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
    ok = ok && (all(m23[0] == _skTemp0[0]) && all(m23[1] == _skTemp0[1]));
    var m24: mat2x4<f32> = mat2x4<f32>(3.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0);
    let _skTemp1 = mat2x4<f32>(3.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0);
    ok = ok && (all(m24[0] == _skTemp1[0]) && all(m24[1] == _skTemp1[1]));
    var m32: mat3x2<f32> = mat3x2<f32>(4.0, 0.0, 0.0, 4.0, 0.0, 0.0);
    let _skTemp2 = mat3x2<f32>(4.0, 0.0, 0.0, 4.0, 0.0, 0.0);
    ok = ok && (all(m32[0] == _skTemp2[0]) && all(m32[1] == _skTemp2[1]) && all(m32[2] == _skTemp2[2]));
    var m34: mat3x4<f32> = mat3x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0);
    let _skTemp3 = mat3x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0);
    ok = ok && (all(m34[0] == _skTemp3[0]) && all(m34[1] == _skTemp3[1]) && all(m34[2] == _skTemp3[2]));
    var m42: mat4x2<f32> = mat4x2<f32>(6.0, 0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 0.0);
    let _skTemp4 = mat4x2<f32>(6.0, 0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 0.0);
    ok = ok && (all(m42[0] == _skTemp4[0]) && all(m42[1] == _skTemp4[1]) && all(m42[2] == _skTemp4[2]) && all(m42[3] == _skTemp4[3]));
    var m43: mat4x3<f32> = mat4x3<f32>(7.0, 0.0, 0.0, 0.0, 7.0, 0.0, 0.0, 0.0, 7.0, 0.0, 0.0, 0.0);
    let _skTemp5 = mat4x3<f32>(7.0, 0.0, 0.0, 0.0, 7.0, 0.0, 0.0, 0.0, 7.0, 0.0, 0.0, 0.0);
    ok = ok && (all(m43[0] == _skTemp5[0]) && all(m43[1] == _skTemp5[1]) && all(m43[2] == _skTemp5[2]) && all(m43[3] == _skTemp5[3]));
    var m22: mat2x2<f32> = m32 * m23;
    let _skTemp6 = mat2x2<f32>(8.0, 0.0, 0.0, 8.0);
    ok = ok && (all(m22[0] == _skTemp6[0]) && all(m22[1] == _skTemp6[1]));
    var m33: mat3x3<f32> = m43 * m34;
    let _skTemp7 = mat3x3<f32>(35.0, 0.0, 0.0, 0.0, 35.0, 0.0, 0.0, 0.0, 35.0);
    ok = ok && (all(m33[0] == _skTemp7[0]) && all(m33[1] == _skTemp7[1]) && all(m33[2] == _skTemp7[2]));
    m23 = mat2x3<f32>(m23[0] + 1.0, m23[1] + 1.0);
    let _skTemp8 = mat2x3<f32>(3.0, 1.0, 1.0, 1.0, 3.0, 1.0);
    ok = ok && (all(m23[0] == _skTemp8[0]) && all(m23[1] == _skTemp8[1]));
    m32 = mat3x2<f32>(m32[0] - 2.0, m32[1] - 2.0, m32[2] - 2.0);
    let _skTemp9 = mat3x2<f32>(2.0, -2.0, -2.0, 2.0, -2.0, -2.0);
    ok = ok && (all(m32[0] == _skTemp9[0]) && all(m32[1] == _skTemp9[1]) && all(m32[2] == _skTemp9[2]));
    m24 = m24 * 0.25;
    let _skTemp10 = mat2x4<f32>(0.75, 0.0, 0.0, 0.0, 0.0, 0.75, 0.0, 0.0);
    ok = ok && (all(m24[0] == _skTemp10[0]) && all(m24[1] == _skTemp10[1]));
    return ok;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_ok: bool = true;
    var _1_m23: mat2x3<f32> = mat2x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
    let _skTemp11 = mat2x3<f32>(2.0, 0.0, 0.0, 0.0, 2.0, 0.0);
    _0_ok = _0_ok && (all(_1_m23[0] == _skTemp11[0]) && all(_1_m23[1] == _skTemp11[1]));
    var _2_m24: mat2x4<f32> = mat2x4<f32>(3.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0);
    let _skTemp12 = mat2x4<f32>(3.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0);
    _0_ok = _0_ok && (all(_2_m24[0] == _skTemp12[0]) && all(_2_m24[1] == _skTemp12[1]));
    var _3_m32: mat3x2<f32> = mat3x2<f32>(4.0, 0.0, 0.0, 4.0, 0.0, 0.0);
    let _skTemp13 = mat3x2<f32>(4.0, 0.0, 0.0, 4.0, 0.0, 0.0);
    _0_ok = _0_ok && (all(_3_m32[0] == _skTemp13[0]) && all(_3_m32[1] == _skTemp13[1]) && all(_3_m32[2] == _skTemp13[2]));
    var _7_m22: mat2x2<f32> = _3_m32 * _1_m23;
    let _skTemp14 = mat2x2<f32>(8.0, 0.0, 0.0, 8.0);
    _0_ok = _0_ok && (all(_7_m22[0] == _skTemp14[0]) && all(_7_m22[1] == _skTemp14[1]));
    _1_m23 = mat2x3<f32>(_1_m23[0] + 1.0, _1_m23[1] + 1.0);
    let _skTemp15 = mat2x3<f32>(3.0, 1.0, 1.0, 1.0, 3.0, 1.0);
    _0_ok = _0_ok && (all(_1_m23[0] == _skTemp15[0]) && all(_1_m23[1] == _skTemp15[1]));
    _3_m32 = mat3x2<f32>(_3_m32[0] - 2.0, _3_m32[1] - 2.0, _3_m32[2] - 2.0);
    let _skTemp16 = mat3x2<f32>(2.0, -2.0, -2.0, 2.0, -2.0, -2.0);
    _0_ok = _0_ok && (all(_3_m32[0] == _skTemp16[0]) && all(_3_m32[1] == _skTemp16[1]) && all(_3_m32[2] == _skTemp16[2]));
    _2_m24 = _2_m24 * 0.25;
    let _skTemp17 = mat2x4<f32>(0.75, 0.0, 0.0, 0.0, 0.0, 0.75, 0.0, 0.0);
    _0_ok = _0_ok && (all(_2_m24[0] == _skTemp17[0]) && all(_2_m24[1] == _skTemp17[1]));
    var _skTemp18: vec4<f32>;
    var _skTemp19: bool;
    if _0_ok {
      let _skTemp20 = test_half_b();
      _skTemp19 = _skTemp20;
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
