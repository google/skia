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
    const expectedA: vec4<f32> = vec4<f32>(0.5, 0.5, 0.75, 2.25);
    const expectedB: vec4<f32> = vec4<f32>(0.0, 1.0, 0.75, 2.25);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((max(_globalUniforms.testInputs.x, 0.5) == expectedA.x) && all(max(_globalUniforms.testInputs.xy, vec2<f32>(0.5)) == expectedA.xy)) && all(max(_globalUniforms.testInputs.xyz, vec3<f32>(0.5)) == expectedA.xyz)) && all(max(_globalUniforms.testInputs, vec4<f32>(0.5)) == expectedA)) && (0.5 == expectedA.x)) && all(vec2<f32>(0.5) == expectedA.xy)) && all(vec3<f32>(0.5, 0.5, 0.75) == expectedA.xyz)) && all(vec4<f32>(0.5, 0.5, 0.75, 2.25) == expectedA)) && (max(_globalUniforms.testInputs.x, _globalUniforms.colorGreen.x) == expectedB.x)) && all(max(_globalUniforms.testInputs.xy, _globalUniforms.colorGreen.xy) == expectedB.xy)) && all(max(_globalUniforms.testInputs.xyz, _globalUniforms.colorGreen.xyz) == expectedB.xyz)) && all(max(_globalUniforms.testInputs, _globalUniforms.colorGreen) == expectedB)) && (0.0 == expectedB.x)) && all(vec2<f32>(0.0, 1.0) == expectedB.xy)) && all(vec3<f32>(0.0, 1.0, 0.75) == expectedB.xyz)) && all(vec4<f32>(0.0, 1.0, 0.75, 2.25) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
