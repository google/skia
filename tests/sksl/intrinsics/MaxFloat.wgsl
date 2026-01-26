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
    const expectedA: vec4<f16> = vec4<f16>(0.5h, 0.5h, 0.75h, 2.25h);
    const expectedB: vec4<f16> = vec4<f16>(0.0h, 1.0h, 0.75h, 2.25h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((max(_globalUniforms.testInputs.x, 0.5h) == expectedA.x) && all(max(_globalUniforms.testInputs.xy, vec2<f16>(0.5h)) == expectedA.xy)) && all(max(_globalUniforms.testInputs.xyz, vec3<f16>(0.5h)) == expectedA.xyz)) && all(max(_globalUniforms.testInputs, vec4<f16>(0.5h)) == expectedA)) && (0.5h == expectedA.x)) && all(vec2<f16>(0.5h) == expectedA.xy)) && all(vec3<f16>(0.5h, 0.5h, 0.75h) == expectedA.xyz)) && all(vec4<f16>(0.5h, 0.5h, 0.75h, 2.25h) == expectedA)) && (max(_globalUniforms.testInputs.x, _globalUniforms.colorGreen.x) == expectedB.x)) && all(max(_globalUniforms.testInputs.xy, _globalUniforms.colorGreen.xy) == expectedB.xy)) && all(max(_globalUniforms.testInputs.xyz, _globalUniforms.colorGreen.xyz) == expectedB.xyz)) && all(max(_globalUniforms.testInputs, _globalUniforms.colorGreen) == expectedB)) && (0.0h == expectedB.x)) && all(vec2<f16>(0.0h, 1.0h) == expectedB.xy)) && all(vec3<f16>(0.0h, 1.0h, 0.75h) == expectedB.xyz)) && all(vec4<f16>(0.0h, 1.0h, 0.75h, 2.25h) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
