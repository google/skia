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
    const expectedA: vec4<f16> = vec4<f16>(-1.0h, 0.0h, 0.75h, 1.0h);
    const clampLow: vec4<f16> = vec4<f16>(-1.0h, -2.0h, -2.0h, 1.0h);
    const expectedB: vec4<f16> = vec4<f16>(-1.0h, 0.0h, 0.5h, 2.25h);
    const clampHigh: vec4<f16> = vec4<f16>(1.0h, 2.0h, 0.5h, 3.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((clamp(_globalUniforms.testInputs.x, -1.0h, 1.0h) == expectedA.x) && all(clamp(_globalUniforms.testInputs.xy, vec2<f16>(-1.0h), vec2<f16>(1.0h)) == expectedA.xy)) && all(clamp(_globalUniforms.testInputs.xyz, vec3<f16>(-1.0h), vec3<f16>(1.0h)) == expectedA.xyz)) && all(clamp(_globalUniforms.testInputs, vec4<f16>(-1.0h), vec4<f16>(1.0h)) == expectedA)) && (clamp(_globalUniforms.testInputs.x, -1.0h, 1.0h) == expectedB.x)) && all(clamp(_globalUniforms.testInputs.xy, vec2<f16>(-1.0h, -2.0h), vec2<f16>(1.0h, 2.0h)) == expectedB.xy)) && all(clamp(_globalUniforms.testInputs.xyz, vec3<f16>(-1.0h, -2.0h, -2.0h), vec3<f16>(1.0h, 2.0h, 0.5h)) == expectedB.xyz)) && all(clamp(_globalUniforms.testInputs, clampLow, clampHigh) == expectedB)) && (-1.0h == expectedA.x)) && all(vec2<f16>(-1.0h, 0.0h) == expectedA.xy)) && all(vec3<f16>(-1.0h, 0.0h, 0.75h) == expectedA.xyz)) && all(vec4<f16>(-1.0h, 0.0h, 0.75h, 1.0h) == expectedA)) && (-1.0h == expectedB.x)) && all(vec2<f16>(-1.0h, 0.0h) == expectedB.xy)) && all(vec3<f16>(-1.0h, 0.0h, 0.5h) == expectedB.xyz)) && all(vec4<f16>(-1.0h, 0.0h, 0.5h, 2.25h) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
