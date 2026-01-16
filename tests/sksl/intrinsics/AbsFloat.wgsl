diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const expected: vec4<f32> = vec4<f32>(1.25, 0.0, 0.75, 2.25);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((abs(_globalUniforms.testInputs.x) == expected.x) && all(abs(_globalUniforms.testInputs.xy) == expected.xy)) && all(abs(_globalUniforms.testInputs.xyz) == expected.xyz)) && all(abs(_globalUniforms.testInputs) == expected)) && (1.25 == expected.x)) && all(vec2<f32>(1.25, 0.0) == expected.xy)) && all(vec3<f32>(1.25, 0.0, 0.75) == expected.xyz)) && all(vec4<f32>(1.25, 0.0, 0.75, 2.25) == expected)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
