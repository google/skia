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
    let intGreen: vec4<i32> = vec4<i32>(_globalUniforms.colorGreen * 100.0);
    const expectedA: vec4<i32> = vec4<i32>(50, 50, 75, 225);
    const expectedB: vec4<i32> = vec4<i32>(0, 100, 75, 225);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((max(intValues.x, 50) == expectedA.x) && all(max(intValues.xy, vec2<i32>(50)) == expectedA.xy)) && all(max(intValues.xyz, vec3<i32>(50)) == expectedA.xyz)) && all(max(intValues, vec4<i32>(50)) == expectedA)) && (50 == expectedA.x)) && all(vec2<i32>(50) == expectedA.xy)) && all(vec3<i32>(50, 50, 75) == expectedA.xyz)) && all(vec4<i32>(50, 50, 75, 225) == expectedA)) && (max(intValues.x, intGreen.x) == expectedB.x)) && all(max(intValues.xy, intGreen.xy) == expectedB.xy)) && all(max(intValues.xyz, intGreen.xyz) == expectedB.xyz)) && all(max(intValues, intGreen) == expectedB)) && (0 == expectedB.x)) && all(vec2<i32>(0, 100) == expectedB.xy)) && all(vec3<i32>(0, 100, 75) == expectedB.xyz)) && all(vec4<i32>(0, 100, 75, 225) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
