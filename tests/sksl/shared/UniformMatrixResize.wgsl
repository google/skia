diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix3x3: mat3x3<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn resizeMatrix_f22() -> mat2x2<f32> {
  {
    let _skTemp0 = _globalUniforms.testMatrix3x3;
    return mat2x2<f32>(_skTemp0[0][0], _skTemp0[0][1], _skTemp0[1][0], _skTemp0[1][1]);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp1: vec4<f32>;
    var _skTemp2: bool;
    let _skTemp3 = resizeMatrix_f22();
    let _skTemp4 = _skTemp3;
    let _skTemp5 = mat2x2<f32>(1.0, 2.0, 4.0, 5.0);
    if (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1])) {
      let _skTemp6 = resizeMatrix_f22();
      let _skTemp7 = _skTemp6;
      let _skTemp8 = mat3x3<f32>(_skTemp7[0][0], _skTemp7[0][1], 0.0, _skTemp7[1][0], _skTemp7[1][1], 0.0, 0.0, 0.0, 1.0);
      let _skTemp9 = mat3x3<f32>(1.0, 2.0, 0.0, 4.0, 5.0, 0.0, 0.0, 0.0, 1.0);
      _skTemp2 = (all(_skTemp8[0] == _skTemp9[0]) && all(_skTemp8[1] == _skTemp9[1]) && all(_skTemp8[2] == _skTemp9[2]));
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = _globalUniforms.colorGreen;
    } else {
      _skTemp1 = _globalUniforms.colorRed;
    }
    return _skTemp1;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
