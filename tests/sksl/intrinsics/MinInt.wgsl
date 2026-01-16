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
    const expectedA: vec4<i32> = vec4<i32>(-125, 0, 50, 50);
    const expectedB: vec4<i32> = vec4<i32>(-125, 0, 0, 100);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((min(intValues.x, 50) == expectedA.x) && all(min(intValues.xy, vec2<i32>(50)) == expectedA.xy)) && all(min(intValues.xyz, vec3<i32>(50)) == expectedA.xyz)) && all(min(intValues, vec4<i32>(50)) == expectedA)) && (-125 == expectedA.x)) && all(vec2<i32>(-125, 0) == expectedA.xy)) && all(vec3<i32>(-125, 0, 50) == expectedA.xyz)) && all(vec4<i32>(-125, 0, 50, 50) == expectedA)) && (min(intValues.x, intGreen.x) == expectedB.x)) && all(min(intValues.xy, intGreen.xy) == expectedB.xy)) && all(min(intValues.xyz, intGreen.xyz) == expectedB.xyz)) && all(min(intValues, intGreen) == expectedB)) && (-125 == expectedB.x)) && all(vec2<i32>(-125, 0) == expectedB.xy)) && all(vec3<i32>(-125, 0, 0) == expectedB.xyz)) && all(vec4<i32>(-125, 0, 0, 100) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
