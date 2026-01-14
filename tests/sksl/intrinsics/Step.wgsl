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
    const constGreen: vec4<f32> = vec4<f32>(0.0, 1.0, 0.0, 1.0);
    const expectedA: vec4<f32> = vec4<f32>(0.0, 0.0, 1.0, 1.0);
    const expectedB: vec4<f32> = vec4<f32>(1.0, 1.0, 0.0, 0.0);
    const expectedC: vec4<f32> = vec4<f32>(0.0, 1.0, 1.0, 1.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((((((((((step(0.5, _globalUniforms.testInputs.x) == expectedA.x) && all(step(vec2<f32>(0.5), _globalUniforms.testInputs.xy) == expectedA.xy)) && all(step(vec3<f32>(0.5), _globalUniforms.testInputs.xyz) == expectedA.xyz)) && all(step(vec4<f32>(0.5), _globalUniforms.testInputs) == expectedA)) && (0.0 == expectedA.x)) && all(vec2<f32>(0.0) == expectedA.xy)) && all(vec3<f32>(0.0, 0.0, 1.0) == expectedA.xyz)) && all(vec4<f32>(0.0, 0.0, 1.0, 1.0) == expectedA)) && (step(_globalUniforms.testInputs.x, 0.0) == expectedB.x)) && all(step(_globalUniforms.testInputs.xy, vec2<f32>(0.0, 1.0)) == expectedB.xy)) && all(step(_globalUniforms.testInputs.xyz, vec3<f32>(0.0, 1.0, 0.0)) == expectedB.xyz)) && all(step(_globalUniforms.testInputs, constGreen) == expectedB)) && (1.0 == expectedB.x)) && all(vec2<f32>(1.0) == expectedB.xy)) && all(vec3<f32>(1.0, 1.0, 0.0) == expectedB.xyz)) && all(vec4<f32>(1.0, 1.0, 0.0, 0.0) == expectedB)) && (step(_globalUniforms.colorRed.x, _globalUniforms.colorGreen.x) == expectedC.x)) && all(step(_globalUniforms.colorRed.xy, _globalUniforms.colorGreen.xy) == expectedC.xy)) && all(step(_globalUniforms.colorRed.xyz, _globalUniforms.colorGreen.xyz) == expectedC.xyz)) && all(step(_globalUniforms.colorRed, _globalUniforms.colorGreen) == expectedC)) && (0.0 == expectedC.x)) && all(vec2<f32>(0.0, 1.0) == expectedC.xy)) && all(vec3<f32>(0.0, 1.0, 1.0) == expectedC.xyz)) && all(vec4<f32>(0.0, 1.0, 1.0, 1.0) == expectedC)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
