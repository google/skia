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
    const expectedA: vec4<u32> = vec4<u32>(125u, 80u, 80u, 225u);
    const expectedB: vec4<u32> = vec4<u32>(125u, 100u, 75u, 225u);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((max(uintValues.x, 80u) == expectedA.x) && all(max(uintValues.xy, vec2<u32>(80u)) == expectedA.xy)) && all(max(uintValues.xyz, vec3<u32>(80u)) == expectedA.xyz)) && all(max(uintValues, vec4<u32>(80u)) == expectedA)) && (125u == expectedA.x)) && all(vec2<u32>(125u, 80u) == expectedA.xy)) && all(vec3<u32>(125u, 80u, 80u) == expectedA.xyz)) && all(vec4<u32>(125u, 80u, 80u, 225u) == expectedA)) && (max(uintValues.x, uintGreen.x) == expectedB.x)) && all(max(uintValues.xy, uintGreen.xy) == expectedB.xy)) && all(max(uintValues.xyz, uintGreen.xyz) == expectedB.xyz)) && all(max(uintValues, uintGreen) == expectedB)) && (125u == expectedB.x)) && all(vec2<u32>(125u, 100u) == expectedB.xy)) && all(vec3<u32>(125u, 100u, 75u) == expectedB.xyz)) && all(vec4<u32>(125u, 100u, 75u, 225u) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
