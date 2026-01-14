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
    const expectedA: vec4<f32> = vec4<f32>(-1.0, 0.0, 0.0, 2.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((f32(trunc(_globalUniforms.testInputs.x)) == -1.0) && all(vec2<f32>(trunc(_globalUniforms.testInputs.xy)) == vec2<f32>(-1.0, 0.0))) && all(vec3<f32>(trunc(_globalUniforms.testInputs.xyz)) == vec3<f32>(-1.0, 0.0, 0.0))) && all(vec4<f32>(trunc(_globalUniforms.testInputs)) == expectedA)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
