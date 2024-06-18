diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  colorBlack: vec4<f32>,
  colorWhite: vec4<f32>,
  testInputs: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var FTFT: vec4<bool> = vec4<bool>(_globalUniforms.colorGreen);
    var TFTF: vec4<bool> = FTFT.wzyx;
    let _skTemp0 = select(_globalUniforms.colorBlack.x, _globalUniforms.colorWhite.x, FTFT.x);
    let _skTemp1 = select(_globalUniforms.colorBlack.xy, _globalUniforms.colorWhite.xy, FTFT.xy);
    let _skTemp2 = select(_globalUniforms.colorBlack.xyz, _globalUniforms.colorWhite.xyz, FTFT.xyz);
    let _skTemp3 = select(_globalUniforms.colorBlack, _globalUniforms.colorWhite, FTFT);
    let _skTemp4 = select(_globalUniforms.colorWhite.x, _globalUniforms.testInputs.x, TFTF.x);
    let _skTemp5 = select(_globalUniforms.colorWhite.xy, _globalUniforms.testInputs.xy, TFTF.xy);
    let _skTemp6 = select(_globalUniforms.colorWhite.xyz, _globalUniforms.testInputs.xyz, TFTF.xyz);
    let _skTemp7 = select(_globalUniforms.colorWhite, _globalUniforms.testInputs, TFTF);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp0 == _globalUniforms.colorBlack.x) && all(_skTemp1 == vec2<f32>(_globalUniforms.colorBlack.x, 1.0))) && all(_skTemp2 == vec3<f32>(_globalUniforms.colorBlack.x, 1.0, _globalUniforms.colorBlack.z))) && all(_skTemp3 == vec4<f32>(_globalUniforms.colorBlack.x, 1.0, _globalUniforms.colorBlack.z, 1.0))) && (_skTemp4 == _globalUniforms.testInputs.x)) && all(_skTemp5 == vec2<f32>(_globalUniforms.testInputs.x, 1.0))) && all(_skTemp6 == vec3<f32>(_globalUniforms.testInputs.x, 1.0, _globalUniforms.testInputs.z))) && all(_skTemp7 == vec4<f32>(_globalUniforms.testInputs.x, 1.0, _globalUniforms.testInputs.z, 1.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
