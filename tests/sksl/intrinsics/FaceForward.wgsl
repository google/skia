diagnostic(off, derivative_uniformity);
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
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = 1e+30;
    let _skTemp1 = select(-1.0, 1.0, _skTemp0 * 1e+30 < 0);
    var huge: f32 = f32(_skTemp1);
    let _skTemp2 = vec2<f32>(1.0);
    let _skTemp3 = faceForward(_skTemp2, vec2<f32>(1e+30), vec2<f32>(1e+30));
    var huge2: vec2<f32> = _skTemp3;
    let _skTemp4 = vec3<f32>(1.0);
    let _skTemp5 = faceForward(_skTemp4, vec3<f32>(1e+30), vec3<f32>(1e+30));
    var huge3: vec3<f32> = _skTemp5;
    let _skTemp6 = vec4<f32>(1.0);
    let _skTemp7 = faceForward(_skTemp6, vec4<f32>(1e+30), vec4<f32>(1e+30));
    var huge4: vec4<f32> = _skTemp7;
    var expectedPos: vec4<f32> = vec4<f32>(vec4<f32>(huge) + huge2.xxxx);
    var expectedNeg: vec4<f32> = vec4<f32>(huge3.xxxx + huge4.xxxx);
    expectedPos = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    expectedNeg = vec4<f32>(-1.0, -2.0, -3.0, -4.0);
    let _skTemp8 = _globalUniforms.N.x;
    let _skTemp9 = select(-_skTemp8, _skTemp8, _globalUniforms.I.x * _globalUniforms.NRef.x < 0);
    let _skTemp10 = faceForward(_globalUniforms.N.xy, _globalUniforms.I.xy, _globalUniforms.NRef.xy);
    let _skTemp11 = faceForward(_globalUniforms.N.xyz, _globalUniforms.I.xyz, _globalUniforms.NRef.xyz);
    let _skTemp12 = faceForward(_globalUniforms.N, _globalUniforms.I, _globalUniforms.NRef);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp9 == expectedNeg.x) && all(_skTemp10 == expectedNeg.xy)) && all(_skTemp11 == expectedPos.xyz)) && all(_skTemp12 == expectedPos)) && (-1.0 == expectedNeg.x)) && all(vec2<f32>(-1.0, -2.0) == expectedNeg.xy)) && all(vec3<f32>(1.0, 2.0, 3.0) == expectedPos.xyz)) && all(vec4<f32>(1.0, 2.0, 3.0, 4.0) == expectedPos)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
