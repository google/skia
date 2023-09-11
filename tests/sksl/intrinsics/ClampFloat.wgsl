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
    var expectedA: vec4<f32> = vec4<f32>(-1.0, 0.0, 0.75, 1.0);
    const clampLow: vec4<f32> = vec4<f32>(-1.0, -2.0, -2.0, 1.0);
    var expectedB: vec4<f32> = vec4<f32>(-1.0, 0.0, 0.5, 2.25);
    const clampHigh: vec4<f32> = vec4<f32>(1.0, 2.0, 0.5, 3.0);
    let _skTemp0 = clamp(_globalUniforms.testInputs.x, -1.0, 1.0);
    let _skTemp1 = clamp(_globalUniforms.testInputs.xy, vec2<f32>(-1.0), vec2<f32>(1.0));
    let _skTemp2 = clamp(_globalUniforms.testInputs.xyz, vec3<f32>(-1.0), vec3<f32>(1.0));
    let _skTemp3 = clamp(_globalUniforms.testInputs, vec4<f32>(-1.0), vec4<f32>(1.0));
    let _skTemp4 = clamp(_globalUniforms.testInputs.x, -1.0, 1.0);
    let _skTemp5 = clamp(_globalUniforms.testInputs.xy, vec2<f32>(-1.0, -2.0), vec2<f32>(1.0, 2.0));
    let _skTemp6 = clamp(_globalUniforms.testInputs.xyz, vec3<f32>(-1.0, -2.0, -2.0), vec3<f32>(1.0, 2.0, 0.5));
    let _skTemp7 = clamp(_globalUniforms.testInputs, clampLow, clampHigh);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp0 == expectedA.x) && all(_skTemp1 == expectedA.xy)) && all(_skTemp2 == expectedA.xyz)) && all(_skTemp3 == expectedA)) && (_skTemp4 == expectedB.x)) && all(_skTemp5 == expectedB.xy)) && all(_skTemp6 == expectedB.xyz)) && all(_skTemp7 == expectedB)) && (-1.0 == expectedA.x)) && all(vec2<f32>(-1.0, 0.0) == expectedA.xy)) && all(vec3<f32>(-1.0, 0.0, 0.75) == expectedA.xyz)) && all(vec4<f32>(-1.0, 0.0, 0.75, 1.0) == expectedA)) && (-1.0 == expectedB.x)) && all(vec2<f32>(-1.0, 0.0) == expectedB.xy)) && all(vec3<f32>(-1.0, 0.0, 0.5) == expectedB.xyz)) && all(vec4<f32>(-1.0, 0.0, 0.5, 2.25) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
