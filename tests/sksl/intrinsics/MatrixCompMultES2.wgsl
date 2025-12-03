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
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const smallM22: mat2x2<f32> = mat2x2<f32>(1000.0, 1000.0, 1000.0, 1000.0);
    let _skTemp0 = smallM22;
    let _skTemp1 = mat2x2<f32>(_skTemp0[0] * smallM22[0], _skTemp0[1] * smallM22[1]);
    var h22: mat2x2<f32> = _skTemp1;
    const hugeM22: mat2x2<f32> = mat2x2<f32>(1e+30, 1e+30, 1e+30, 1e+30);
    let _skTemp2 = hugeM22;
    let _skTemp3 = mat2x2<f32>(_skTemp2[0] * hugeM22[0], _skTemp2[1] * hugeM22[1]);
    h22 = mat2x2<f32>(_skTemp3);
    h22 = mat2x2<f32>(0.0, 5.0, 10.0, 15.0);
    const _skTemp4 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    let _skTemp5 = mat2x2<f32>(_globalUniforms.testMatrix2x2[0] * _skTemp4[0], _globalUniforms.testMatrix2x2[1] * _skTemp4[1]);
    let f22: mat2x2<f32> = _skTemp5;
    const _skTemp6 = mat3x3<f32>(2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0);
    let _skTemp7 = mat3x3<f32>(_globalUniforms.testMatrix3x3[0] * _skTemp6[0], _globalUniforms.testMatrix3x3[1] * _skTemp6[1], _globalUniforms.testMatrix3x3[2] * _skTemp6[2]);
    let h33: mat3x3<f32> = _skTemp7;
    const _skTemp8 = mat2x2<f32>(0.0, 5.0, 10.0, 15.0);
    const _skTemp9 = mat2x2<f32>(1.0, 0.0, 0.0, 4.0);
    const _skTemp10 = mat3x3<f32>(2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((all(h22[0] == _skTemp8[0]) && all(h22[1] == _skTemp8[1])) && (all(f22[0] == _skTemp9[0]) && all(f22[1] == _skTemp9[1]))) && (all(h33[0] == _skTemp10[0]) && all(h33[1] == _skTemp10[1]) && all(h33[2] == _skTemp10[2]))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
