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
    var intGreen: vec4<i32> = vec4<i32>(_globalUniforms.colorGreen * 100.0);
    var expectedA: vec4<i32> = vec4<i32>(-125, 0, 50, 50);
    var expectedB: vec4<i32> = vec4<i32>(-125, 0, 0, 100);
    let _skTemp0 = min(intValues.x, 50);
    let _skTemp1 = min(intValues.xy, vec2<i32>(50));
    let _skTemp2 = min(intValues.xyz, vec3<i32>(50));
    let _skTemp3 = min(intValues, vec4<i32>(50));
    let _skTemp4 = min(intValues.x, intGreen.x);
    let _skTemp5 = min(intValues.xy, intGreen.xy);
    let _skTemp6 = min(intValues.xyz, intGreen.xyz);
    let _skTemp7 = min(intValues, intGreen);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp0 == expectedA.x) && all(_skTemp1 == expectedA.xy)) && all(_skTemp2 == expectedA.xyz)) && all(_skTemp3 == expectedA)) && (-125 == expectedA.x)) && all(vec2<i32>(-125, 0) == expectedA.xy)) && all(vec3<i32>(-125, 0, 50) == expectedA.xyz)) && all(vec4<i32>(-125, 0, 50, 50) == expectedA)) && (_skTemp4 == expectedB.x)) && all(_skTemp5 == expectedB.xy)) && all(_skTemp6 == expectedB.xyz)) && all(_skTemp7 == expectedB)) && (-125 == expectedB.x)) && all(vec2<i32>(-125, 0) == expectedB.xy)) && all(vec3<i32>(-125, 0, 0) == expectedB.xyz)) && all(vec4<i32>(-125, 0, 0, 100) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
