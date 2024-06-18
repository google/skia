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
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var expectedA: vec4<f32> = vec4<f32>(0.5, 0.5, 0.75, 2.25);
    var expectedB: vec4<f32> = vec4<f32>(0.0, 1.0, 0.75, 2.25);
    let _skTemp0 = max(_globalUniforms.testInputs.x, 0.5);
    let _skTemp1 = max(_globalUniforms.testInputs.xy, vec2<f32>(0.5));
    let _skTemp2 = max(_globalUniforms.testInputs.xyz, vec3<f32>(0.5));
    let _skTemp3 = max(_globalUniforms.testInputs, vec4<f32>(0.5));
    let _skTemp4 = max(_globalUniforms.testInputs.x, _globalUniforms.colorGreen.x);
    let _skTemp5 = max(_globalUniforms.testInputs.xy, _globalUniforms.colorGreen.xy);
    let _skTemp6 = max(_globalUniforms.testInputs.xyz, _globalUniforms.colorGreen.xyz);
    let _skTemp7 = max(_globalUniforms.testInputs, _globalUniforms.colorGreen);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp0 == expectedA.x) && all(_skTemp1 == expectedA.xy)) && all(_skTemp2 == expectedA.xyz)) && all(_skTemp3 == expectedA)) && (0.5 == expectedA.x)) && all(vec2<f32>(0.5) == expectedA.xy)) && all(vec3<f32>(0.5, 0.5, 0.75) == expectedA.xyz)) && all(vec4<f32>(0.5, 0.5, 0.75, 2.25) == expectedA)) && (_skTemp4 == expectedB.x)) && all(_skTemp5 == expectedB.xy)) && all(_skTemp6 == expectedB.xyz)) && all(_skTemp7 == expectedB)) && (0.0 == expectedB.x)) && all(vec2<f32>(0.0, 1.0) == expectedB.xy)) && all(vec3<f32>(0.0, 1.0, 0.75) == expectedB.xyz)) && all(vec4<f32>(0.0, 1.0, 0.75, 2.25) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
