diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
    return mat2x2<f32>(_globalUniforms.testMatrix3x3[0][0], _globalUniforms.testMatrix3x3[0][1], _globalUniforms.testMatrix3x3[1][0], _globalUniforms.testMatrix3x3[1][1]);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    let _skTemp2 = resizeMatrix_f22();
    let _skTemp3 = _skTemp2;
    const _skTemp4 = mat2x2<f32>(1.0, 2.0, 4.0, 5.0);
    if (all(_skTemp3[0] == _skTemp4[0]) && all(_skTemp3[1] == _skTemp4[1])) {
      let _skTemp5 = resizeMatrix_f22();
      let _skTemp6 = _skTemp5;
      let _skTemp7 = mat3x3<f32>(_skTemp6[0][0], _skTemp6[0][1], 0.0, _skTemp6[1][0], _skTemp6[1][1], 0.0, 0.0, 0.0, 1.0);
      const _skTemp8 = mat3x3<f32>(1.0, 2.0, 0.0, 4.0, 5.0, 0.0, 0.0, 0.0, 1.0);
      _skTemp1 = (all(_skTemp7[0] == _skTemp8[0]) && all(_skTemp7[1] == _skTemp8[1]) && all(_skTemp7[2] == _skTemp8[2]));
    } else {
      _skTemp1 = false;
    }
    if _skTemp1 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
