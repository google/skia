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
    const expectedA: vec4<f32> = vec4<f32>(-1.0, 0.0, 0.0, 2.0);
    let _skTemp0 = trunc(_globalUniforms.testInputs.x);
    let _skTemp1 = trunc(_globalUniforms.testInputs.xy);
    let _skTemp2 = trunc(_globalUniforms.testInputs.xyz);
    let _skTemp3 = trunc(_globalUniforms.testInputs);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((f32(_skTemp0) == -1.0) && all(vec2<f32>(_skTemp1) == vec2<f32>(-1.0, 0.0))) && all(vec3<f32>(_skTemp2) == vec3<f32>(-1.0, 0.0, 0.0))) && all(vec4<f32>(_skTemp3) == expectedA)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
