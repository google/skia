### Compilation failed:

error: Tint compilation failed.

diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testMatrix2x2: mat2x2<f32>,
  testMatrix3x3: mat3x3<f32>,
  testInputs: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const c12: vec2<f32> = vec2<f32>(1.0, 2.0);
    let _skTemp0 = outerProduct(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]);
    let _skTemp1 = _skTemp0;
    let _skTemp2 = mat2x2<f32>(3.0, 6.0, 4.0, 8.0);
    let _skTemp3 = outerProduct(_globalUniforms.testMatrix3x3[0], _globalUniforms.testMatrix3x3[1]);
    let _skTemp4 = _skTemp3;
    let _skTemp5 = mat3x3<f32>(4.0, 8.0, 12.0, 5.0, 10.0, 15.0, 6.0, 12.0, 18.0);
    let _skTemp6 = outerProduct(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix3x3[1]);
    let _skTemp7 = _skTemp6;
    let _skTemp8 = mat3x2<f32>(4.0, 8.0, 5.0, 10.0, 6.0, 12.0);
    let _skTemp9 = outerProduct(_globalUniforms.testInputs, vec4<f32>(1.0, 0.0, 0.0, 2.0));
    let _skTemp10 = mat4x4<f32>(_skTemp9);
    let _skTemp11 = mat4x4<f32>(-1.25, 0.0, 0.75, 2.25, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -2.5, 0.0, 1.5, 4.5);
    let _skTemp12 = outerProduct(vec4<f32>(_globalUniforms.testInputs), c12);
    let _skTemp13 = _skTemp12;
    let _skTemp14 = mat2x4<f32>(-1.25, 0.0, 0.75, 2.25, -2.5, 0.0, 1.5, 4.5);
    let _skTemp15 = outerProduct(c12, vec4<f32>(_globalUniforms.testInputs));
    let _skTemp16 = _skTemp15;
    let _skTemp17 = mat4x2<f32>(-1.25, -2.5, 0.0, 0.0, 0.75, 1.5, 2.25, 4.5);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((all(_skTemp1[0] == _skTemp2[0]) && all(_skTemp1[1] == _skTemp2[1])) && (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1]) && all(_skTemp4[2] == _skTemp5[2]))) && (all(_skTemp7[0] == _skTemp8[0]) && all(_skTemp7[1] == _skTemp8[1]) && all(_skTemp7[2] == _skTemp8[2]))) && (all(_skTemp10[0] == _skTemp11[0]) && all(_skTemp10[1] == _skTemp11[1]) && all(_skTemp10[2] == _skTemp11[2]) && all(_skTemp10[3] == _skTemp11[3]))) && (all(_skTemp13[0] == _skTemp14[0]) && all(_skTemp13[1] == _skTemp14[1]))) && (all(_skTemp16[0] == _skTemp17[0]) && all(_skTemp16[1] == _skTemp17[1]) && all(_skTemp16[2] == _skTemp17[2]) && all(_skTemp16[3] == _skTemp17[3]))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
