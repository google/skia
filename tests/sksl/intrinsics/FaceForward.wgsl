struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  N: vec4<f32>,
  I: vec4<f32>,
  NRef: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let xy = _skParam0;
  {
    var expectedPos: vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    var expectedNeg: vec4<f32> = vec4<f32>(-1.0, -2.0, -3.0, -4.0);
    let _skTemp0 = (select(-1.0, 1.0, (_globalUniforms.I.x * _globalUniforms.NRef.x) < 0) * _globalUniforms.N.x);
    let _skTemp1 = faceForward(_globalUniforms.N.xy, _globalUniforms.I.xy, _globalUniforms.NRef.xy);
    let _skTemp2 = faceForward(_globalUniforms.N.xyz, _globalUniforms.I.xyz, _globalUniforms.NRef.xyz);
    let _skTemp3 = faceForward(_globalUniforms.N, _globalUniforms.I, _globalUniforms.NRef);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((_skTemp0 == expectedNeg.x && all(_skTemp1 == expectedNeg.xy)) && all(_skTemp2 == expectedPos.xyz)) && all(_skTemp3 == expectedPos)) && -1.0 == expectedNeg.x) && all(vec2<f32>(-1.0, -2.0) == expectedNeg.xy)) && all(vec3<f32>(1.0, 2.0, 3.0) == expectedPos.xyz)) && all(vec4<f32>(1.0, 2.0, 3.0, 4.0) == expectedPos)));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
