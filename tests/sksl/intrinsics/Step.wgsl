diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  testInputs: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const constGreen: vec4<f16> = vec4<f16>(0.0h, 1.0h, 0.0h, 1.0h);
    const expectedA: vec4<f16> = vec4<f16>(0.0h, 0.0h, 1.0h, 1.0h);
    const expectedB: vec4<f16> = vec4<f16>(1.0h, 1.0h, 0.0h, 0.0h);
    const expectedC: vec4<f16> = vec4<f16>(0.0h, 1.0h, 1.0h, 1.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((((((((((step(0.5h, _globalUniforms.testInputs.x) == expectedA.x) && all(step(vec2<f16>(0.5h), _globalUniforms.testInputs.xy) == expectedA.xy)) && all(step(vec3<f16>(0.5h), _globalUniforms.testInputs.xyz) == expectedA.xyz)) && all(step(vec4<f16>(0.5h), _globalUniforms.testInputs) == expectedA)) && (0.0h == expectedA.x)) && all(vec2<f16>(0.0h) == expectedA.xy)) && all(vec3<f16>(0.0h, 0.0h, 1.0h) == expectedA.xyz)) && all(vec4<f16>(0.0h, 0.0h, 1.0h, 1.0h) == expectedA)) && (step(_globalUniforms.testInputs.x, 0.0h) == expectedB.x)) && all(step(_globalUniforms.testInputs.xy, vec2<f16>(0.0h, 1.0h)) == expectedB.xy)) && all(step(_globalUniforms.testInputs.xyz, vec3<f16>(0.0h, 1.0h, 0.0h)) == expectedB.xyz)) && all(step(_globalUniforms.testInputs, constGreen) == expectedB)) && (1.0h == expectedB.x)) && all(vec2<f16>(1.0h) == expectedB.xy)) && all(vec3<f16>(1.0h, 1.0h, 0.0h) == expectedB.xyz)) && all(vec4<f16>(1.0h, 1.0h, 0.0h, 0.0h) == expectedB)) && (step(_globalUniforms.colorRed.x, _globalUniforms.colorGreen.x) == expectedC.x)) && all(step(_globalUniforms.colorRed.xy, _globalUniforms.colorGreen.xy) == expectedC.xy)) && all(step(_globalUniforms.colorRed.xyz, _globalUniforms.colorGreen.xyz) == expectedC.xyz)) && all(step(_globalUniforms.colorRed, _globalUniforms.colorGreen) == expectedC)) && (0.0h == expectedC.x)) && all(vec2<f16>(0.0h, 1.0h) == expectedC.xy)) && all(vec3<f16>(0.0h, 1.0h, 1.0h) == expectedC.xyz)) && all(vec4<f16>(0.0h, 1.0h, 1.0h, 1.0h) == expectedC)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
