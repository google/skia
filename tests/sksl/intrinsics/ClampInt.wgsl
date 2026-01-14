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
    let intValues: vec4<i32> = vec4<i32>(_globalUniforms.testInputs * 100.0);
    const expectedA: vec4<i32> = vec4<i32>(-100, 0, 75, 100);
    const clampLow: vec4<i32> = vec4<i32>(-100, -200, -200, 100);
    const expectedB: vec4<i32> = vec4<i32>(-100, 0, 50, 225);
    const clampHigh: vec4<i32> = vec4<i32>(100, 200, 50, 300);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((clamp(intValues.x, -100, 100) == expectedA.x) && all(clamp(intValues.xy, vec2<i32>(-100), vec2<i32>(100)) == expectedA.xy)) && all(clamp(intValues.xyz, vec3<i32>(-100), vec3<i32>(100)) == expectedA.xyz)) && all(clamp(intValues, vec4<i32>(-100), vec4<i32>(100)) == expectedA)) && (-100 == expectedA.x)) && all(vec2<i32>(-100, 0) == expectedA.xy)) && all(vec3<i32>(-100, 0, 75) == expectedA.xyz)) && all(vec4<i32>(-100, 0, 75, 100) == expectedA)) && (clamp(intValues.x, -100, 100) == expectedB.x)) && all(clamp(intValues.xy, vec2<i32>(-100, -200), vec2<i32>(100, 200)) == expectedB.xy)) && all(clamp(intValues.xyz, vec3<i32>(-100, -200, -200), vec3<i32>(100, 200, 50)) == expectedB.xyz)) && all(clamp(intValues, clampLow, clampHigh) == expectedB)) && (-100 == expectedB.x)) && all(vec2<i32>(-100, 0) == expectedB.xy)) && all(vec3<i32>(-100, 0, 50) == expectedB.xyz)) && all(vec4<i32>(-100, 0, 50, 225) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
