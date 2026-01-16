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
    let uintValues: vec4<u32> = vec4<u32>(abs(_globalUniforms.testInputs) * 100.0);
    let uintGreen: vec4<u32> = vec4<u32>(_globalUniforms.colorGreen * 100.0);
    const expectedA: vec4<u32> = vec4<u32>(50u, 0u, 50u, 50u);
    const expectedB: vec4<u32> = vec4<u32>(0u, 0u, 0u, 100u);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((min(uintValues.x, 50u) == expectedA.x) && all(min(uintValues.xy, vec2<u32>(50u)) == expectedA.xy)) && all(min(uintValues.xyz, vec3<u32>(50u)) == expectedA.xyz)) && all(min(uintValues, vec4<u32>(50u)) == expectedA)) && (50u == expectedA.x)) && all(vec2<u32>(50u, 0u) == expectedA.xy)) && all(vec3<u32>(50u, 0u, 50u) == expectedA.xyz)) && all(vec4<u32>(50u, 0u, 50u, 50u) == expectedA)) && (min(uintValues.x, uintGreen.x) == expectedB.x)) && all(min(uintValues.xy, uintGreen.xy) == expectedB.xy)) && all(min(uintValues.xyz, uintGreen.xyz) == expectedB.xyz)) && all(min(uintValues, uintGreen) == expectedB)) && (0u == expectedB.x)) && all(vec2<u32>(0u) == expectedB.xy)) && all(vec3<u32>(0u) == expectedB.xyz)) && all(vec4<u32>(0u, 0u, 0u, 100u) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
