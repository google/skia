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
    var h22: mat2x2<f32> = mat2x2<f32>(1000000.0, 1000000.0, 1000000.0, 1000000.0);
    const hugeM22: mat2x2<f32> = mat2x2<f32>(1e+30, 1e+30, 1e+30, 1e+30);
    let _skTemp0 = hugeM22;
    let _skTemp1 = mat2x2<f32>(_skTemp0[0] * hugeM22[0], _skTemp0[1] * hugeM22[1]);
    h22 = _skTemp1;
    h22 = mat2x2<f32>(0.0, 5.0, 10.0, 15.0);
    let _skTemp2 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    let _skTemp3 = mat2x2<f32>(_globalUniforms.testMatrix2x2[0] * _skTemp2[0], _globalUniforms.testMatrix2x2[1] * _skTemp2[1]);
    var f22: mat2x2<f32> = _skTemp3;
    let _skTemp4 = mat3x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
    let _skTemp5 = mat3x3<f32>(_globalUniforms.testMatrix3x3[0] * _skTemp4[0], _globalUniforms.testMatrix3x3[1] * _skTemp4[1], _globalUniforms.testMatrix3x3[2] * _skTemp4[2]);
    var h33: mat3x3<f32> = _skTemp5;
    let _skTemp6 = mat2x2<f32>(0.0, 5.0, 10.0, 15.0);
    let _skTemp7 = mat2x2<f32>(1.0, 0.0, 0.0, 4.0);
    let _skTemp8 = mat3x3<f32>(2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((all(h22[0] == _skTemp6[0]) && all(h22[1] == _skTemp6[1])) && (all(f22[0] == _skTemp7[0]) && all(f22[1] == _skTemp7[1]))) && (all(h33[0] == _skTemp8[0]) && all(h33[1] == _skTemp8[1]) && all(h33[2] == _skTemp8[2]))));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
