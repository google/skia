diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: mat2x2<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  var coords = _skParam0;
  {
    const negativeVal: vec4<f32> = vec4<f32>(-1.0, -4.0, -16.0, -64.0);
    let _skTemp0 = negativeVal;
    let _skTemp1 = sqrt(_skTemp0);
    coords = _skTemp1.xy;
    var inputVal: vec4<f32> = vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]) + vec4<f32>(0.0, 2.0, 6.0, 12.0);
    const expected: vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    const allowedDelta: vec4<f32> = vec4<f32>(0.05);
    let _skTemp2 = sqrt(inputVal.x);
    let _skTemp3 = abs(_skTemp2 - 1.0);
    let _skTemp4 = sqrt(inputVal.xy);
    let _skTemp5 = abs(_skTemp4 - vec2<f32>(1.0, 2.0));
    let _skTemp6 = all((_skTemp5 < vec2<f32>(0.05)));
    let _skTemp7 = sqrt(inputVal.xyz);
    let _skTemp8 = abs(_skTemp7 - vec3<f32>(1.0, 2.0, 3.0));
    let _skTemp9 = all((_skTemp8 < vec3<f32>(0.05)));
    let _skTemp10 = sqrt(inputVal);
    let _skTemp11 = abs(_skTemp10 - expected);
    let _skTemp12 = all((_skTemp11 < allowedDelta));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((_skTemp3 < 0.05) && _skTemp6) && _skTemp9) && _skTemp12));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
