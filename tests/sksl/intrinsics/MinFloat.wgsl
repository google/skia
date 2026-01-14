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
    const expectedA: vec4<f32> = vec4<f32>(-1.25, 0.0, 0.5, 0.5);
    const expectedB: vec4<f32> = vec4<f32>(-1.25, 0.0, 0.0, 1.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((min(_globalUniforms.testInputs.x, 0.5) == expectedA.x) && all(min(_globalUniforms.testInputs.xy, vec2<f32>(0.5)) == expectedA.xy)) && all(min(_globalUniforms.testInputs.xyz, vec3<f32>(0.5)) == expectedA.xyz)) && all(min(_globalUniforms.testInputs, vec4<f32>(0.5)) == expectedA)) && (-1.25 == expectedA.x)) && all(vec2<f32>(-1.25, 0.0) == expectedA.xy)) && all(vec3<f32>(-1.25, 0.0, 0.5) == expectedA.xyz)) && all(vec4<f32>(-1.25, 0.0, 0.5, 0.5) == expectedA)) && (min(_globalUniforms.testInputs.x, f32(_globalUniforms.colorGreen.x)) == expectedB.x)) && all(min(_globalUniforms.testInputs.xy, vec2<f32>(_globalUniforms.colorGreen.xy)) == expectedB.xy)) && all(min(_globalUniforms.testInputs.xyz, vec3<f32>(_globalUniforms.colorGreen.xyz)) == expectedB.xyz)) && all(min(_globalUniforms.testInputs, vec4<f32>(_globalUniforms.colorGreen)) == expectedB)) && (-1.25 == expectedB.x)) && all(vec2<f32>(-1.25, 0.0) == expectedB.xy)) && all(vec3<f32>(-1.25, 0.0, 0.0) == expectedB.xyz)) && all(vec4<f32>(-1.25, 0.0, 0.0, 1.0) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
