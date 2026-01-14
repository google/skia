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
    const expected: vec4<f32> = vec4<f32>(-71.61973, 0.0, 42.9718361, 128.915512);
    const allowedDelta: vec4<f32> = vec4<f32>(0.05);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((abs(degrees(_globalUniforms.testInputs.x) - -71.61973) < 0.05) && all((abs(degrees(_globalUniforms.testInputs.xy) - vec2<f32>(-71.61973, 0.0)) < vec2<f32>(0.05)))) && all((abs(degrees(_globalUniforms.testInputs.xyz) - vec3<f32>(-71.61973, 0.0, 42.9718361)) < vec3<f32>(0.05)))) && all((abs(degrees(_globalUniforms.testInputs) - expected) < allowedDelta))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
