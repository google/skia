### Compilation failed:

error: :15:20 error: unresolved call target 'isnan'
    let _skTemp0 = isnan(valueIsNaN.x);
                   ^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var valueIsNaN: vec4<f32> = 0.0 / _globalUniforms.testInputs.yyyy;
    var valueIsNumber: vec4<f32> = 1.0 / _globalUniforms.testInputs;
    let _skTemp0 = isnan(valueIsNaN.x);
    let _skTemp1 = isnan(valueIsNaN.xy);
    let _skTemp2 = all(_skTemp1);
    let _skTemp3 = isnan(valueIsNaN.xyz);
    let _skTemp4 = all(_skTemp3);
    let _skTemp5 = isnan(valueIsNaN);
    let _skTemp6 = all(_skTemp5);
    let _skTemp7 = isnan(valueIsNumber.x);
    let _skTemp8 = isnan(valueIsNumber.xy);
    let _skTemp9 = any(_skTemp8);
    let _skTemp10 = isnan(valueIsNumber.xyz);
    let _skTemp11 = any(_skTemp10);
    let _skTemp12 = isnan(valueIsNumber);
    let _skTemp13 = any(_skTemp12);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((_skTemp0 && _skTemp2) && _skTemp4) && _skTemp6) && (!_skTemp7)) && (!_skTemp9)) && (!_skTemp11)) && (!_skTemp13)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
