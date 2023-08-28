diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  pos1: vec4<f32>,
  pos2: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var expected: vec4<f32> = vec4<f32>(3.0, 3.0, 5.0, 13.0);
    let _skTemp0 = distance(_globalUniforms.pos1.x, _globalUniforms.pos2.x);
    let _skTemp1 = distance(_globalUniforms.pos1.xy, _globalUniforms.pos2.xy);
    let _skTemp2 = distance(_globalUniforms.pos1.xyz, _globalUniforms.pos2.xyz);
    let _skTemp3 = distance(_globalUniforms.pos1, _globalUniforms.pos2);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp0 == expected.x) && (_skTemp1 == expected.y)) && (_skTemp2 == expected.z)) && (_skTemp3 == expected.w)) && (3.0 == expected.x)) && (3.0 == expected.y)) && (5.0 == expected.z)) && (13.0 == expected.w)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
