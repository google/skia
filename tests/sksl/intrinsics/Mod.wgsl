diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  colorWhite: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const expectedA: vec4<f32> = vec4<f32>(0.75, 0.0, 0.75, 0.25);
    const expectedB: vec4<f32> = vec4<f32>(0.25, 0.0, 0.75, 1.0);
    let _skTemp0 = _globalUniforms.testInputs.x;
    let _skTemp1 = _globalUniforms.testInputs.xy;
    let _skTemp2 = _globalUniforms.testInputs.xyz;
    let _skTemp3 = _globalUniforms.testInputs.x;
    let _skTemp4 = _globalUniforms.colorWhite.x;
    let _skTemp5 = _globalUniforms.testInputs.xy;
    let _skTemp6 = _globalUniforms.colorWhite.xy;
    let _skTemp7 = _globalUniforms.testInputs.xyz;
    let _skTemp8 = _globalUniforms.colorWhite.xyz;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((((((((((_skTemp0 - 1.0 * floor(_skTemp0 / 1.0)) == expectedA.x) && all((_skTemp1 - 1.0 * floor(_skTemp1 / 1.0)) == expectedA.xy)) && all((_skTemp2 - 1.0 * floor(_skTemp2 / 1.0)) == expectedA.xyz)) && all((_globalUniforms.testInputs - 1.0 * floor(_globalUniforms.testInputs / 1.0)) == expectedA)) && (0.75 == expectedA.x)) && all(vec2<f32>(0.75, 0.0) == expectedA.xy)) && all(vec3<f32>(0.75, 0.0, 0.75) == expectedA.xyz)) && all(vec4<f32>(0.75, 0.0, 0.75, 0.25) == expectedA)) && ((_skTemp3 - _skTemp4 * floor(_skTemp3 / _skTemp4)) == expectedA.x)) && all((_skTemp5 - _skTemp6 * floor(_skTemp5 / _skTemp6)) == expectedA.xy)) && all((_skTemp7 - _skTemp8 * floor(_skTemp7 / _skTemp8)) == expectedA.xyz)) && all((_globalUniforms.testInputs - _globalUniforms.colorWhite * floor(_globalUniforms.testInputs / _globalUniforms.colorWhite)) == expectedA)) && (0.25 == expectedB.x)) && all(vec2<f32>(0.25, 0.0) == expectedB.xy)) && all(vec3<f32>(0.25, 0.0, 0.75) == expectedB.xyz)) && all(vec4<f32>(0.25, 0.0, 0.75, 1.0) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
