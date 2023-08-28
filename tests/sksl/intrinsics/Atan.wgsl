diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  inputVal: vec4<f32>,
  expected: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const constVal2: vec4<f32> = vec4<f32>(1.0);
    let _skTemp0 = atan(_globalUniforms.inputVal.x);
    let _skTemp1 = atan(_globalUniforms.inputVal.xy);
    let _skTemp2 = atan(_globalUniforms.inputVal.xyz);
    let _skTemp3 = atan(_globalUniforms.inputVal);
    let _skTemp4 = atan2(_globalUniforms.inputVal.x, 1.0);
    let _skTemp5 = atan2(_globalUniforms.inputVal.xy, vec2<f32>(1.0));
    let _skTemp6 = atan2(_globalUniforms.inputVal.xyz, vec3<f32>(1.0));
    let _skTemp7 = atan2(_globalUniforms.inputVal, constVal2);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp0 == _globalUniforms.expected.x) && all(_skTemp1 == _globalUniforms.expected.xy)) && all(_skTemp2 == _globalUniforms.expected.xyz)) && all(_skTemp3 == _globalUniforms.expected)) && (0.0 == _globalUniforms.expected.x)) && all(vec2<f32>(0.0) == _globalUniforms.expected.xy)) && all(vec3<f32>(0.0) == _globalUniforms.expected.xyz)) && all(vec4<f32>(0.0) == _globalUniforms.expected)) && (_skTemp4 == _globalUniforms.expected.x)) && all(_skTemp5 == _globalUniforms.expected.xy)) && all(_skTemp6 == _globalUniforms.expected.xyz)) && all(_skTemp7 == _globalUniforms.expected)) && (0.0 == _globalUniforms.expected.x)) && all(vec2<f32>(0.0) == _globalUniforms.expected.xy)) && all(vec3<f32>(0.0) == _globalUniforms.expected.xyz)) && all(vec4<f32>(0.0) == _globalUniforms.expected)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
