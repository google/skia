diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
  colorBlack: vec4<f16>,
  colorWhite: vec4<f16>,
  testInputs: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const expectedBW: vec4<f16> = vec4<f16>(0.5h, 0.5h, 0.5h, 1.0h);
    const expectedWT: vec4<f16> = vec4<f16>(1.0h, 0.5h, 1.0h, 2.25h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((((((((((((all(mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f16>(0.0h)) == vec4<f16>(0.0h, 1.0h, 0.0h, 1.0h)) && all(mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f16>(0.25h)) == vec4<f16>(0.25h, 0.75h, 0.0h, 1.0h))) && all(mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f16>(0.75h)) == vec4<f16>(0.75h, 0.25h, 0.0h, 1.0h))) && all(mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f16>(1.0h)) == vec4<f16>(1.0h, 0.0h, 0.0h, 1.0h))) && (mix(_globalUniforms.colorBlack.x, _globalUniforms.colorWhite.x, 0.5h) == expectedBW.x)) && all(mix(_globalUniforms.colorBlack.xy, _globalUniforms.colorWhite.xy, vec2<f16>(0.5h)) == expectedBW.xy)) && all(mix(_globalUniforms.colorBlack.xyz, _globalUniforms.colorWhite.xyz, vec3<f16>(0.5h)) == expectedBW.xyz)) && all(mix(_globalUniforms.colorBlack, _globalUniforms.colorWhite, vec4<f16>(0.5h)) == expectedBW)) && (0.5h == expectedBW.x)) && all(vec2<f16>(0.5h) == expectedBW.xy)) && all(vec3<f16>(0.5h) == expectedBW.xyz)) && all(vec4<f16>(0.5h, 0.5h, 0.5h, 1.0h) == expectedBW)) && (mix(_globalUniforms.colorWhite.x, _globalUniforms.testInputs.x, 0.0h) == expectedWT.x)) && all(mix(_globalUniforms.colorWhite.xy, _globalUniforms.testInputs.xy, vec2<f16>(0.0h, 0.5h)) == expectedWT.xy)) && all(mix(_globalUniforms.colorWhite.xyz, _globalUniforms.testInputs.xyz, vec3<f16>(0.0h, 0.5h, 0.0h)) == expectedWT.xyz)) && all(mix(_globalUniforms.colorWhite, _globalUniforms.testInputs, vec4<f16>(0.0h, 0.5h, 0.0h, 1.0h)) == expectedWT)) && (1.0h == expectedWT.x)) && all(vec2<f16>(1.0h, 0.5h) == expectedWT.xy)) && all(vec3<f16>(1.0h, 0.5h, 1.0h) == expectedWT.xyz)) && all(vec4<f16>(1.0h, 0.5h, 1.0h, 2.25h) == expectedWT)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
