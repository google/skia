diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: mat2x2<f32>,
  testMatrix3x3: mat3x3<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var testMatrix2x3: mat2x3<f32> = mat2x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    let _skTemp0 = transpose(_globalUniforms.testMatrix2x2);
    let _skTemp1 = _skTemp0;
    let _skTemp2 = mat2x2<f32>(1.0, 3.0, 2.0, 4.0);
    let _skTemp3 = transpose(testMatrix2x3);
    let _skTemp4 = _skTemp3;
    let _skTemp5 = mat3x2<f32>(1.0, 4.0, 2.0, 5.0, 3.0, 6.0);
    let _skTemp6 = transpose(_globalUniforms.testMatrix3x3);
    let _skTemp7 = _skTemp6;
    let _skTemp8 = mat3x3<f32>(1.0, 4.0, 7.0, 2.0, 5.0, 8.0, 3.0, 6.0, 9.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((all(_skTemp1[0] == _skTemp2[0]) && all(_skTemp1[1] == _skTemp2[1])) && (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1]) && all(_skTemp4[2] == _skTemp5[2]))) && (all(_skTemp7[0] == _skTemp8[0]) && all(_skTemp7[1] == _skTemp8[1]) && all(_skTemp7[2] == _skTemp8[2]))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
