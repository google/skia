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
    var intValues: vec4<i32> = vec4<i32>(_globalUniforms.testInputs * 100.0);
    var expectedA: vec4<i32> = vec4<i32>(-100, 0, 75, 100);
    const clampLow: vec4<i32> = vec4<i32>(-100, -200, -200, 100);
    var expectedB: vec4<i32> = vec4<i32>(-100, 0, 50, 225);
    const clampHigh: vec4<i32> = vec4<i32>(100, 200, 50, 300);
    let _skTemp0 = clamp(intValues.x, -100, 100);
    let _skTemp1 = clamp(intValues.xy, vec2<i32>(-100), vec2<i32>(100));
    let _skTemp2 = clamp(intValues.xyz, vec3<i32>(-100), vec3<i32>(100));
    let _skTemp3 = clamp(intValues, vec4<i32>(-100), vec4<i32>(100));
    let _skTemp4 = clamp(intValues.x, -100, 100);
    let _skTemp5 = clamp(intValues.xy, vec2<i32>(-100, -200), vec2<i32>(100, 200));
    let _skTemp6 = clamp(intValues.xyz, vec3<i32>(-100, -200, -200), vec3<i32>(100, 200, 50));
    let _skTemp7 = clamp(intValues, clampLow, clampHigh);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp0 == expectedA.x) && all(_skTemp1 == expectedA.xy)) && all(_skTemp2 == expectedA.xyz)) && all(_skTemp3 == expectedA)) && (-100 == expectedA.x)) && all(vec2<i32>(-100, 0) == expectedA.xy)) && all(vec3<i32>(-100, 0, 75) == expectedA.xyz)) && all(vec4<i32>(-100, 0, 75, 100) == expectedA)) && (_skTemp4 == expectedB.x)) && all(_skTemp5 == expectedB.xy)) && all(_skTemp6 == expectedB.xyz)) && all(_skTemp7 == expectedB)) && (-100 == expectedB.x)) && all(vec2<i32>(-100, 0) == expectedB.xy)) && all(vec3<i32>(-100, 0, 50) == expectedB.xyz)) && all(vec4<i32>(-100, 0, 50, 225) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
