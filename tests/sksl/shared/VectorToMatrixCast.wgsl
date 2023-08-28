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
  testInputs: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var ok: bool = true;
    let _skTemp0 = mat2x2<f32>(_globalUniforms.testInputs[0], _globalUniforms.testInputs[1], _globalUniforms.testInputs[2], _globalUniforms.testInputs[3]);
    let _skTemp1 = mat2x2<f32>(-1.25, 0.0, 0.75, 2.25);
    ok = ok && (all(_skTemp0[0] == _skTemp1[0]) && all(_skTemp0[1] == _skTemp1[1]));
    let _skTemp2 = vec4<f32>(_globalUniforms.testInputs);
    let _skTemp3 = mat2x2<f32>(_skTemp2[0], _skTemp2[1], _skTemp2[2], _skTemp2[3]);
    let _skTemp4 = mat2x2<f32>(-1.25, 0.0, 0.75, 2.25);
    ok = ok && (all(_skTemp3[0] == _skTemp4[0]) && all(_skTemp3[1] == _skTemp4[1]));
    let _skTemp5 = mat2x2<f32>(_globalUniforms.colorGreen[0], _globalUniforms.colorGreen[1], _globalUniforms.colorGreen[2], _globalUniforms.colorGreen[3]);
    let _skTemp6 = mat2x2<f32>(0.0, 1.0, 0.0, 1.0);
    ok = ok && (all(_skTemp5[0] == _skTemp6[0]) && all(_skTemp5[1] == _skTemp6[1]));
    let _skTemp7 = mat2x2<f32>(_globalUniforms.colorGreen[0], _globalUniforms.colorGreen[1], _globalUniforms.colorGreen[2], _globalUniforms.colorGreen[3]);
    let _skTemp8 = mat2x2<f32>(0.0, 1.0, 0.0, 1.0);
    ok = ok && (all(_skTemp7[0] == _skTemp8[0]) && all(_skTemp7[1] == _skTemp8[1]));
    let _skTemp9 = vec4<f32>(vec4<i32>(_globalUniforms.colorGreen));
    let _skTemp10 = mat2x2<f32>(_skTemp9[0], _skTemp9[1], _skTemp9[2], _skTemp9[3]);
    let _skTemp11 = mat2x2<f32>(0.0, 1.0, 0.0, 1.0);
    ok = ok && (all(_skTemp10[0] == _skTemp11[0]) && all(_skTemp10[1] == _skTemp11[1]));
    let _skTemp12 = mat2x2<f32>(_globalUniforms.colorGreen[0], _globalUniforms.colorGreen[1], _globalUniforms.colorGreen[2], _globalUniforms.colorGreen[3]);
    let _skTemp13 = mat2x2<f32>(0.0, 1.0, 0.0, 1.0);
    ok = ok && (all(_skTemp12[0] == _skTemp13[0]) && all(_skTemp12[1] == _skTemp13[1]));
    let _skTemp14 = mat2x2<f32>(_globalUniforms.colorGreen[0], _globalUniforms.colorGreen[1], _globalUniforms.colorGreen[2], _globalUniforms.colorGreen[3]);
    let _skTemp15 = mat2x2<f32>(0.0, 1.0, 0.0, 1.0);
    ok = ok && (all(_skTemp14[0] == _skTemp15[0]) && all(_skTemp14[1] == _skTemp15[1]));
    let _skTemp16 = vec4<f32>(vec4<bool>(_globalUniforms.colorGreen));
    let _skTemp17 = mat2x2<f32>(_skTemp16[0], _skTemp16[1], _skTemp16[2], _skTemp16[3]);
    let _skTemp18 = mat2x2<f32>(0.0, 1.0, 0.0, 1.0);
    ok = ok && (all(_skTemp17[0] == _skTemp18[0]) && all(_skTemp17[1] == _skTemp18[1]));
    let _skTemp19 = _globalUniforms.colorGreen - _globalUniforms.colorRed;
    let _skTemp20 = mat2x2<f32>(_skTemp19[0], _skTemp19[1], _skTemp19[2], _skTemp19[3]);
    let _skTemp21 = mat2x2<f32>(-1.0, 1.0, 0.0, 0.0);
    ok = ok && (all(_skTemp20[0] == _skTemp21[0]) && all(_skTemp20[1] == _skTemp21[1]));
    let _skTemp22 = _globalUniforms.colorGreen + 5.0;
    let _skTemp23 = mat2x2<f32>(_skTemp22[0], _skTemp22[1], _skTemp22[2], _skTemp22[3]);
    let _skTemp24 = mat2x2<f32>(5.0, 6.0, 5.0, 6.0);
    ok = ok && (all(_skTemp23[0] == _skTemp24[0]) && all(_skTemp23[1] == _skTemp24[1]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
