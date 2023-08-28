### Compilation failed:

error: :19:20 error: unresolved call target 'isinf'
    let _skTemp0 = isinf(infiniteValue.x);
                   ^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: mat2x2<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var infiniteValue: vec4<f32> = vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]) / _globalUniforms.colorGreen.x;
    var finiteValue: vec4<f32> = vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]) / _globalUniforms.colorGreen.y;
    let _skTemp0 = isinf(infiniteValue.x);
    let _skTemp1 = isinf(infiniteValue.xy);
    let _skTemp2 = all(_skTemp1);
    let _skTemp3 = isinf(infiniteValue.xyz);
    let _skTemp4 = all(_skTemp3);
    let _skTemp5 = isinf(infiniteValue);
    let _skTemp6 = all(_skTemp5);
    let _skTemp7 = isinf(finiteValue.x);
    let _skTemp8 = isinf(finiteValue.xy);
    let _skTemp9 = any(_skTemp8);
    let _skTemp10 = isinf(finiteValue.xyz);
    let _skTemp11 = any(_skTemp10);
    let _skTemp12 = isinf(finiteValue);
    let _skTemp13 = any(_skTemp12);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((_skTemp0 && _skTemp2) && _skTemp4) && _skTemp6) && (!_skTemp7)) && (!_skTemp9)) && (!_skTemp11)) && (!_skTemp13)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
