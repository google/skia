diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = 1e+30;
    let huge: f32 = select(-1.0, 1.0, _skTemp0 * 1e+30 < 0);
    let _skTemp1 = vec2<f32>(1.0);
    let huge2: vec2<f32> = faceForward(_skTemp1, vec2<f32>(1e+30), vec2<f32>(1e+30));
    let _skTemp2 = vec3<f32>(1.0);
    let huge3: vec3<f32> = faceForward(_skTemp2, vec3<f32>(1e+30), vec3<f32>(1e+30));
    let _skTemp3 = vec4<f32>(1.0);
    let huge4: vec4<f32> = faceForward(_skTemp3, vec4<f32>(1e+30), vec4<f32>(1e+30));
    var expectedPos: vec4<f32> = vec4<f32>(vec4<f32>(huge) + huge2.xxxx);
    var expectedNeg: vec4<f32> = vec4<f32>(huge3.xxxx + huge4.xxxx);
    expectedPos = vec4<f32>(1.0, 2.0, 3.0, 4.0);
    expectedNeg = vec4<f32>(-1.0, -2.0, -3.0, -4.0);
    let _skTemp4 = _globalUniforms.N.x;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((select(-_skTemp4, _skTemp4, _globalUniforms.I.x * _globalUniforms.NRef.x < 0) == expectedNeg.x) && all(faceForward(_globalUniforms.N.xy, _globalUniforms.I.xy, _globalUniforms.NRef.xy) == expectedNeg.xy)) && all(faceForward(_globalUniforms.N.xyz, _globalUniforms.I.xyz, _globalUniforms.NRef.xyz) == expectedPos.xyz)) && all(faceForward(_globalUniforms.N, _globalUniforms.I, _globalUniforms.NRef) == expectedPos)) && (-1.0 == expectedNeg.x)) && all(vec2<f32>(-1.0, -2.0) == expectedNeg.xy)) && all(vec3<f32>(1.0, 2.0, 3.0) == expectedPos.xyz)) && all(vec4<f32>(1.0, 2.0, 3.0, 4.0) == expectedPos)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
