diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  inputVal: vec4<f32>,
  expected: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = sin(_globalUniforms.inputVal.x);
    let _skTemp1 = sin(_globalUniforms.inputVal.xy);
    let _skTemp2 = sin(_globalUniforms.inputVal.xyz);
    let _skTemp3 = sin(_globalUniforms.inputVal);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp0 == _globalUniforms.expected.x) && all(_skTemp1 == _globalUniforms.expected.xy)) && all(_skTemp2 == _globalUniforms.expected.xyz)) && all(_skTemp3 == _globalUniforms.expected)) && (0.0 == _globalUniforms.expected.x)) && all(vec2<f32>(0.0) == _globalUniforms.expected.xy)) && all(vec3<f32>(0.0) == _globalUniforms.expected.xyz)) && all(vec4<f32>(0.0) == _globalUniforms.expected)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
