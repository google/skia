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
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const negativeVal: vec4<f32> = vec4<f32>(-1.0, -4.0, -16.0, -64.0);
    let _skTemp0 = -1.0;
    let _skTemp1 = vec2<f32>(-1.0, -4.0);
    let _skTemp2 = vec3<f32>(-1.0, -4.0, -16.0);
    let _skTemp3 = negativeVal;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((inverseSqrt(_globalUniforms.inputVal.x) == _globalUniforms.expected.x) && all(inverseSqrt(_globalUniforms.inputVal.xy) == _globalUniforms.expected.xy)) && all(inverseSqrt(_globalUniforms.inputVal.xyz) == _globalUniforms.expected.xyz)) && all(inverseSqrt(_globalUniforms.inputVal) == _globalUniforms.expected)) && (1.0 == _globalUniforms.expected.x)) && all(vec2<f32>(1.0, 0.5) == _globalUniforms.expected.xy)) && all(vec3<f32>(1.0, 0.5, 0.25) == _globalUniforms.expected.xyz)) && all(vec4<f32>(1.0, 0.5, 0.25, 0.125) == _globalUniforms.expected)) && (inverseSqrt(_skTemp0) == _globalUniforms.expected.x)) && all(inverseSqrt(_skTemp1) == _globalUniforms.expected.xy)) && all(inverseSqrt(_skTemp2) == _globalUniforms.expected.xyz)) && all(inverseSqrt(_skTemp3) == _globalUniforms.expected)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
