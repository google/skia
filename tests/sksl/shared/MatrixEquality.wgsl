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
  testMatrix3x3: mat3x3<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_ok: bool = true;
    let _skTemp0 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    _0_ok = _0_ok && (all(_globalUniforms.testMatrix2x2[0] == _skTemp0[0]) && all(_globalUniforms.testMatrix2x2[1] == _skTemp0[1]));
    let _skTemp1 = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    _0_ok = _0_ok && (all(_globalUniforms.testMatrix3x3[0] == _skTemp1[0]) && all(_globalUniforms.testMatrix3x3[1] == _skTemp1[1]) && all(_globalUniforms.testMatrix3x3[2] == _skTemp1[2]));
    let _skTemp2 = mat2x2<f32>(100.0, 0.0, 0.0, 100.0);
    _0_ok = _0_ok && (any(_globalUniforms.testMatrix2x2[0] != _skTemp2[0]) || any(_globalUniforms.testMatrix2x2[1] != _skTemp2[1]));
    let _skTemp3 = mat3x3<f32>(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    _0_ok = _0_ok && (any(_globalUniforms.testMatrix3x3[0] != _skTemp3[0]) || any(_globalUniforms.testMatrix3x3[1] != _skTemp3[1]) || any(_globalUniforms.testMatrix3x3[2] != _skTemp3[2]));
    var _1_zero: f32 = f32(_globalUniforms.colorGreen.x);
    var _2_one: f32 = f32(_globalUniforms.colorGreen.y);
    var _3_two: f32 = 2.0 * _2_one;
    var _4_nine: f32 = 9.0 * _2_one;
    let _skTemp4 = mat2x2<f32>(_2_one, _1_zero, _1_zero, _2_one);
    let _skTemp5 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1]));
    let _skTemp6 = vec2<f32>(_2_one);
    let _skTemp7 = mat2x2<f32>(_2_one, _1_zero, _skTemp6[0], _skTemp6[1]);
    let _skTemp8 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (any(_skTemp7[0] != _skTemp8[0]) || any(_skTemp7[1] != _skTemp8[1]));
    let _skTemp9 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    let _skTemp10 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp9[0] == _skTemp10[0]) && all(_skTemp9[1] == _skTemp10[1]));
    let _skTemp11 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    let _skTemp12 = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    _0_ok = _0_ok && (any(_skTemp11[0] != _skTemp12[0]) || any(_skTemp11[1] != _skTemp12[1]));
    let _skTemp13 = -_2_one;
    let _skTemp14 = mat2x2<f32>(_skTemp13, 0.0, 0.0, _skTemp13);
    let _skTemp15 = mat2x2<f32>(-1.0, 0.0, 0.0, -1.0);
    _0_ok = _0_ok && (all(_skTemp14[0] == _skTemp15[0]) && all(_skTemp14[1] == _skTemp15[1]));
    let _skTemp16 = mat2x2<f32>(_1_zero, 0.0, 0.0, _1_zero);
    let _skTemp17 = mat2x2<f32>(-0.0, 0.0, 0.0, -0.0);
    _0_ok = _0_ok && (all(_skTemp16[0] == _skTemp17[0]) && all(_skTemp16[1] == _skTemp17[1]));
    let _skTemp18 = -_2_one;
    let _skTemp19 = (-1.0 * mat2x2<f32>(_skTemp18, 0.0, 0.0, _skTemp18));
    let _skTemp20 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp19[0] == _skTemp20[0]) && all(_skTemp19[1] == _skTemp20[1]));
    let _skTemp21 = (-1.0 * mat2x2<f32>(_1_zero, 0.0, 0.0, _1_zero));
    let _skTemp22 = mat2x2<f32>(-0.0, 0.0, 0.0, -0.0);
    _0_ok = _0_ok && (all(_skTemp21[0] == _skTemp22[0]) && all(_skTemp21[1] == _skTemp22[1]));
    let _skTemp23 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    let _skTemp24 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp23[0] == _skTemp24[0]) && all(_skTemp23[1] == _skTemp24[1]));
    let _skTemp25 = mat2x2<f32>(_3_two, 0.0, 0.0, _3_two);
    let _skTemp26 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (any(_skTemp25[0] != _skTemp26[0]) || any(_skTemp25[1] != _skTemp26[1]));
    let _skTemp27 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    let _skTemp28 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp27[0] == _skTemp28[0]) && all(_skTemp27[1] == _skTemp28[1]));
    let _skTemp29 = mat2x2<f32>(_2_one, 0.0, 0.0, _2_one);
    let _skTemp30 = mat2x2<f32>(0.0, 0.0, 0.0, 0.0);
    _0_ok = _0_ok && (any(_skTemp29[0] != _skTemp30[0]) || any(_skTemp29[1] != _skTemp30[1]));
    let _skTemp31 = mat3x3<f32>(_2_one, _1_zero, _1_zero, _1_zero, _2_one, _1_zero, _1_zero, _1_zero, _2_one);
    let _skTemp32 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    let _skTemp33 = mat3x3<f32>(_skTemp32[0][0], _skTemp32[0][1], 0.0, _skTemp32[1][0], _skTemp32[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp31[0] == _skTemp33[0]) && all(_skTemp31[1] == _skTemp33[1]) && all(_skTemp31[2] == _skTemp33[2]));
    let _skTemp34 = mat3x3<f32>(_4_nine, _1_zero, _1_zero, _1_zero, _4_nine, _1_zero, _1_zero, _1_zero, _2_one);
    let _skTemp35 = mat2x2<f32>(9.0, 0.0, 0.0, 9.0);
    let _skTemp36 = mat3x3<f32>(_skTemp35[0][0], _skTemp35[0][1], 0.0, _skTemp35[1][0], _skTemp35[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp34[0] == _skTemp36[0]) && all(_skTemp34[1] == _skTemp36[1]) && all(_skTemp34[2] == _skTemp36[2]));
    let _skTemp37 = mat3x3<f32>(_2_one, 0.0, 0.0, 0.0, _2_one, 0.0, 0.0, 0.0, _2_one);
    let _skTemp38 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    let _skTemp39 = mat3x3<f32>(_skTemp38[0][0], _skTemp38[0][1], 0.0, _skTemp38[1][0], _skTemp38[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp37[0] == _skTemp39[0]) && all(_skTemp37[1] == _skTemp39[1]) && all(_skTemp37[2] == _skTemp39[2]));
    let _skTemp40 = mat3x3<f32>(_4_nine, 0.0, 0.0, 0.0, _4_nine, 0.0, 0.0, 0.0, _2_one);
    let _skTemp41 = mat2x2<f32>(9.0, 0.0, 0.0, 9.0);
    let _skTemp42 = mat3x3<f32>(_skTemp41[0][0], _skTemp41[0][1], 0.0, _skTemp41[1][0], _skTemp41[1][1], 0.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp40[0] == _skTemp42[0]) && all(_skTemp40[1] == _skTemp42[1]) && all(_skTemp40[2] == _skTemp42[2]));
    let _skTemp43 = mat3x3<f32>(_2_one, 0.0, 0.0, 0.0, _2_one, 0.0, 0.0, 0.0, _2_one);
    let _skTemp44 = mat2x2<f32>(_skTemp43[0][0], _skTemp43[0][1], _skTemp43[1][0], _skTemp43[1][1]);
    let _skTemp45 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp44[0] == _skTemp45[0]) && all(_skTemp44[1] == _skTemp45[1]));
    let _skTemp46 = mat3x3<f32>(_2_one, 0.0, 0.0, 0.0, _2_one, 0.0, 0.0, 0.0, _2_one);
    let _skTemp47 = mat2x2<f32>(_skTemp46[0][0], _skTemp46[0][1], _skTemp46[1][0], _skTemp46[1][1]);
    let _skTemp48 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp47[0] == _skTemp48[0]) && all(_skTemp47[1] == _skTemp48[1]));
    let _skTemp49 = mat2x2<f32>(_2_one, _1_zero, _1_zero, _2_one);
    let _skTemp50 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp49[0] == _skTemp50[0]) && all(_skTemp49[1] == _skTemp50[1]));
    let _skTemp51 = mat2x2<f32>(_2_one, _1_zero, _1_zero, _2_one);
    let _skTemp52 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp51[0] == _skTemp52[0]) && all(_skTemp51[1] == _skTemp52[1]));
    let _skTemp53 = mat2x2<f32>(_2_one, _1_zero, _1_zero, _2_one);
    let _skTemp54 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    _0_ok = _0_ok && (all(_skTemp53[0] == _skTemp54[0]) && all(_skTemp53[1] == _skTemp54[1]));
    _0_ok = _0_ok && all((vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) * vec4<f32>(_2_one)) == vec4<f32>(1.0, 2.0, 3.0, 4.0));
    _0_ok = _0_ok && all((vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) * vec4<f32>(_2_one)) == vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])));
    _0_ok = _0_ok && all((vec4<f32>(vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1])) * vec4<f32>(_1_zero)) == vec4<f32>(0.0));
    var _5_m: mat3x3<f32> = mat3x3<f32>(_2_one, _3_two, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, _4_nine);
    _0_ok = _0_ok && all(_5_m[0] == vec3<f32>(1.0, 2.0, 3.0));
    _0_ok = _0_ok && all(_5_m[1] == vec3<f32>(4.0, 5.0, 6.0));
    _0_ok = _0_ok && all(_5_m[2] == vec3<f32>(7.0, 8.0, 9.0));
    _0_ok = _0_ok && (_5_m[0].x == 1.0);
    _0_ok = _0_ok && (_5_m[0].y == 2.0);
    _0_ok = _0_ok && (_5_m[0].z == 3.0);
    _0_ok = _0_ok && (_5_m[1].x == 4.0);
    _0_ok = _0_ok && (_5_m[1].y == 5.0);
    _0_ok = _0_ok && (_5_m[1].z == 6.0);
    _0_ok = _0_ok && (_5_m[2].x == 7.0);
    _0_ok = _0_ok && (_5_m[2].y == 8.0);
    _0_ok = _0_ok && (_5_m[2].z == 9.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_0_ok));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
