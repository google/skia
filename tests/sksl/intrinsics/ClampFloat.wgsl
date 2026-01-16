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
    const expectedA: vec4<f32> = vec4<f32>(-1.0, 0.0, 0.75, 1.0);
    const clampLow: vec4<f32> = vec4<f32>(-1.0, -2.0, -2.0, 1.0);
    const expectedB: vec4<f32> = vec4<f32>(-1.0, 0.0, 0.5, 2.25);
    const clampHigh: vec4<f32> = vec4<f32>(1.0, 2.0, 0.5, 3.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((clamp(_globalUniforms.testInputs.x, -1.0, 1.0) == expectedA.x) && all(clamp(_globalUniforms.testInputs.xy, vec2<f32>(-1.0), vec2<f32>(1.0)) == expectedA.xy)) && all(clamp(_globalUniforms.testInputs.xyz, vec3<f32>(-1.0), vec3<f32>(1.0)) == expectedA.xyz)) && all(clamp(_globalUniforms.testInputs, vec4<f32>(-1.0), vec4<f32>(1.0)) == expectedA)) && (clamp(_globalUniforms.testInputs.x, -1.0, 1.0) == expectedB.x)) && all(clamp(_globalUniforms.testInputs.xy, vec2<f32>(-1.0, -2.0), vec2<f32>(1.0, 2.0)) == expectedB.xy)) && all(clamp(_globalUniforms.testInputs.xyz, vec3<f32>(-1.0, -2.0, -2.0), vec3<f32>(1.0, 2.0, 0.5)) == expectedB.xyz)) && all(clamp(_globalUniforms.testInputs, clampLow, clampHigh) == expectedB)) && (-1.0 == expectedA.x)) && all(vec2<f32>(-1.0, 0.0) == expectedA.xy)) && all(vec3<f32>(-1.0, 0.0, 0.75) == expectedA.xyz)) && all(vec4<f32>(-1.0, 0.0, 0.75, 1.0) == expectedA)) && (-1.0 == expectedB.x)) && all(vec2<f32>(-1.0, 0.0) == expectedB.xy)) && all(vec3<f32>(-1.0, 0.0, 0.5) == expectedB.xyz)) && all(vec4<f32>(-1.0, 0.0, 0.5, 2.25) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
