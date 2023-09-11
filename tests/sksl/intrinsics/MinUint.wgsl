diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = abs(_globalUniforms.testInputs);
    var uintValues: vec4<u32> = vec4<u32>(_skTemp0 * 100.0);
    var uintGreen: vec4<u32> = vec4<u32>(_globalUniforms.colorGreen * 100.0);
    var expectedA: vec4<u32> = vec4<u32>(50u, 0u, 50u, 50u);
    var expectedB: vec4<u32> = vec4<u32>(0u, 0u, 0u, 100u);
    let _skTemp1 = min(uintValues.x, 50u);
    let _skTemp2 = min(uintValues.xy, vec2<u32>(50u));
    let _skTemp3 = min(uintValues.xyz, vec3<u32>(50u));
    let _skTemp4 = min(uintValues, vec4<u32>(50u));
    let _skTemp5 = min(uintValues.x, uintGreen.x);
    let _skTemp6 = min(uintValues.xy, uintGreen.xy);
    let _skTemp7 = min(uintValues.xyz, uintGreen.xyz);
    let _skTemp8 = min(uintValues, uintGreen);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp1 == expectedA.x) && all(_skTemp2 == expectedA.xy)) && all(_skTemp3 == expectedA.xyz)) && all(_skTemp4 == expectedA)) && (50u == expectedA.x)) && all(vec2<u32>(50u, 0u) == expectedA.xy)) && all(vec3<u32>(50u, 0u, 50u) == expectedA.xyz)) && all(vec4<u32>(50u, 0u, 50u, 50u) == expectedA)) && (_skTemp5 == expectedB.x)) && all(_skTemp6 == expectedB.xy)) && all(_skTemp7 == expectedB.xyz)) && all(_skTemp8 == expectedB)) && (0u == expectedB.x)) && all(vec2<u32>(0u) == expectedB.xy)) && all(vec3<u32>(0u) == expectedB.xyz)) && all(vec4<u32>(0u, 0u, 0u, 100u) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
