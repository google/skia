diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  inputVal: vec4<f16>,
  expected: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const negativeVal: vec4<f16> = vec4<f16>(-1.0h, -4.0h, -16.0h, -64.0h);
    let _skTemp0 = -1.0h;
    let _skTemp1 = vec2<f16>(-1.0h, -4.0h);
    let _skTemp2 = vec3<f16>(-1.0h, -4.0h, -16.0h);
    let _skTemp3 = negativeVal;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((inverseSqrt(_globalUniforms.inputVal.x) == _globalUniforms.expected.x) && all(inverseSqrt(_globalUniforms.inputVal.xy) == _globalUniforms.expected.xy)) && all(inverseSqrt(_globalUniforms.inputVal.xyz) == _globalUniforms.expected.xyz)) && all(inverseSqrt(_globalUniforms.inputVal) == _globalUniforms.expected)) && (1.0h == _globalUniforms.expected.x)) && all(vec2<f16>(1.0h, 0.5h) == _globalUniforms.expected.xy)) && all(vec3<f16>(1.0h, 0.5h, 0.25h) == _globalUniforms.expected.xyz)) && all(vec4<f16>(1.0h, 0.5h, 0.25h, 0.125h) == _globalUniforms.expected)) && (inverseSqrt(_skTemp0) == _globalUniforms.expected.x)) && all(inverseSqrt(_skTemp1) == _globalUniforms.expected.xy)) && all(inverseSqrt(_skTemp2) == _globalUniforms.expected.xyz)) && all(inverseSqrt(_skTemp3) == _globalUniforms.expected)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
