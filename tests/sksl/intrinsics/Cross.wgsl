diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix3x3: mat3x3<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    const expected1: vec3<f32> = vec3<f32>(-3.0, 6.0, -3.0);
    const expected2: vec3<f32> = vec3<f32>(6.0, -12.0, 6.0);
    let _skTemp0 = cross(_globalUniforms.testMatrix3x3[0], _globalUniforms.testMatrix3x3[1]);
    let _skTemp1 = cross(_globalUniforms.testMatrix3x3[2], _globalUniforms.testMatrix3x3[0]);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(_skTemp0 == expected1) && all(_skTemp1 == expected2)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
