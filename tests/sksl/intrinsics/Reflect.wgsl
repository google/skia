struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  I: vec4<f32>,
  N: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let xy = _skParam0;
  {
    var expectedX: f32 = -49.0;
    var expectedXY: vec2<f32> = vec2<f32>(-169.0, 202.0);
    var expectedXYZ: vec3<f32> = vec3<f32>(-379.0, 454.0, -529.0);
    var expectedXYZW: vec4<f32> = vec4<f32>(-699.0, 838.0, -977.0, 1116.0);
    let _skTemp0 = _globalUniforms.I.x;
    let _skTemp1 = _globalUniforms.N.x;
    let _skTemp2 = _skTemp0 - 2 * _skTemp1 * _skTemp0 * _skTemp1;
    let _skTemp3 = reflect(_globalUniforms.I.xy, _globalUniforms.N.xy);
    let _skTemp4 = reflect(_globalUniforms.I.xyz, _globalUniforms.N.xyz);
    let _skTemp5 = reflect(_globalUniforms.I, _globalUniforms.N);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp2 == expectedX) && all(_skTemp3 == expectedXY)) && all(_skTemp4 == expectedXYZ)) && all(_skTemp5 == expectedXYZW)) && (-49.0 == expectedX)) && all(vec2<f32>(-169.0, 202.0) == expectedXY)) && all(vec3<f32>(-379.0, 454.0, -529.0) == expectedXYZ)) && all(vec4<f32>(-699.0, 838.0, -977.0, 1116.0) == expectedXYZW)));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
