diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
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
    var expectedA: vec4<i32> = vec4<i32>(50, 50, 75, 225);
    var expectedB: vec4<i32> = vec4<i32>(0, 100, 75, 225);
    let _skTemp0 = max(intValues.x, 50);
    let _skTemp1 = max(intValues.xy, vec2<i32>(50));
    let _skTemp2 = max(intValues.xyz, vec3<i32>(50));
    let _skTemp3 = max(intValues, vec4<i32>(50));
    let _skTemp4 = max(intValues.x, intGreen.x);
    let _skTemp5 = max(intValues.xy, intGreen.xy);
    let _skTemp6 = max(intValues.xyz, intGreen.xyz);
    let _skTemp7 = max(intValues, intGreen);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp0 == expectedA.x) && all(_skTemp1 == expectedA.xy)) && all(_skTemp2 == expectedA.xyz)) && all(_skTemp3 == expectedA)) && (50 == expectedA.x)) && all(vec2<i32>(50) == expectedA.xy)) && all(vec3<i32>(50, 50, 75) == expectedA.xyz)) && all(vec4<i32>(50, 50, 75, 225) == expectedA)) && (_skTemp4 == expectedB.x)) && all(_skTemp5 == expectedB.xy)) && all(_skTemp6 == expectedB.xyz)) && all(_skTemp7 == expectedB)) && (0 == expectedB.x)) && all(vec2<i32>(0, 100) == expectedB.xy)) && all(vec3<i32>(0, 100, 75) == expectedB.xyz)) && all(vec4<i32>(0, 100, 75, 225) == expectedB)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
