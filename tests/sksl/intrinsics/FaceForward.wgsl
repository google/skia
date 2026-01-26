diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  N: vec4<f16>,
  I: vec4<f16>,
  NRef: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(xy: vec2<f32>) -> vec4<f16> {
  {
    let _skTemp0 = 1e+30;
    let huge: f32 = select(-1.0, 1.0, _skTemp0 * 1e+30 < 0);
    let _skTemp1 = vec2<f32>(1.0);
    let huge2: vec2<f32> = faceForward(_skTemp1, vec2<f32>(1e+30), vec2<f32>(1e+30));
    let _skTemp2 = vec3<f32>(1.0);
    let huge3: vec3<f32> = faceForward(_skTemp2, vec3<f32>(1e+30), vec3<f32>(1e+30));
    let _skTemp3 = vec4<f32>(1.0);
    let huge4: vec4<f32> = faceForward(_skTemp3, vec4<f32>(1e+30), vec4<f32>(1e+30));
    var expectedPos: vec4<f16> = vec4<f16>(vec4<f32>(huge) + huge2.xxxx);
    var expectedNeg: vec4<f16> = vec4<f16>(huge3.xxxx + huge4.xxxx);
    expectedPos = vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h);
    expectedNeg = vec4<f16>(-1.0h, -2.0h, -3.0h, -4.0h);
    let _skTemp4 = _globalUniforms.N.x;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((select(-_skTemp4, _skTemp4, _globalUniforms.I.x * _globalUniforms.NRef.x < 0) == expectedNeg.x) && all(faceForward(_globalUniforms.N.xy, _globalUniforms.I.xy, _globalUniforms.NRef.xy) == expectedNeg.xy)) && all(faceForward(_globalUniforms.N.xyz, _globalUniforms.I.xyz, _globalUniforms.NRef.xyz) == expectedPos.xyz)) && all(faceForward(_globalUniforms.N, _globalUniforms.I, _globalUniforms.NRef) == expectedPos)) && (-1.0h == expectedNeg.x)) && all(vec2<f16>(-1.0h, -2.0h) == expectedNeg.xy)) && all(vec3<f16>(1.0h, 2.0h, 3.0h) == expectedPos.xyz)) && all(vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h) == expectedPos)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
