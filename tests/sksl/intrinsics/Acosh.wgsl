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
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((acosh(_globalUniforms.inputVal.x) == _globalUniforms.expected.x) && all(acosh(_globalUniforms.inputVal.xy) == _globalUniforms.expected.xy)) && all(acosh(_globalUniforms.inputVal.xyz) == _globalUniforms.expected.xyz)) && all(acosh(_globalUniforms.inputVal) == _globalUniforms.expected)) && (0.0 == _globalUniforms.expected.x)) && all(vec2<f32>(0.0) == _globalUniforms.expected.xy)) && all(vec3<f32>(0.0, 0.0, 1.0) == _globalUniforms.expected.xyz)) && all(vec4<f32>(0.0, 0.0, 1.0, 2.0) == _globalUniforms.expected)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
