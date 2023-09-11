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
    const expected: vec4<f32> = vec4<f32>(-0.021816615, 0.0, 0.01308997, 0.03926991);
    const allowedDelta: vec4<f32> = vec4<f32>(0.0005);
    let _skTemp0 = radians(_globalUniforms.testInputs.x);
    let _skTemp1 = abs(_skTemp0 - -0.021816615);
    let _skTemp2 = radians(_globalUniforms.testInputs.xy);
    let _skTemp3 = abs(_skTemp2 - vec2<f32>(-0.021816615, 0.0));
    let _skTemp4 = all((_skTemp3 < vec2<f32>(0.0005)));
    let _skTemp5 = radians(_globalUniforms.testInputs.xyz);
    let _skTemp6 = abs(_skTemp5 - vec3<f32>(-0.021816615, 0.0, 0.01308997));
    let _skTemp7 = all((_skTemp6 < vec3<f32>(0.0005)));
    let _skTemp8 = radians(_globalUniforms.testInputs);
    let _skTemp9 = abs(_skTemp8 - expected);
    let _skTemp10 = all((_skTemp9 < allowedDelta));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((_skTemp1 < 0.0005) && _skTemp4) && _skTemp7) && _skTemp10));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
