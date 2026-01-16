diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  colorBlack: vec4<f32>,
  colorWhite: vec4<f32>,
  testInputs: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const expectedBW: vec4<f32> = vec4<f32>(0.5, 0.5, 0.5, 1.0);
    const expectedWT: vec4<f32> = vec4<f32>(1.0, 0.5, 1.0, 2.25);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((((((((((((all(mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f32>(0.0)) == vec4<f32>(0.0, 1.0, 0.0, 1.0)) && all(mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f32>(0.25)) == vec4<f32>(0.25, 0.75, 0.0, 1.0))) && all(mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f32>(0.75)) == vec4<f32>(0.75, 0.25, 0.0, 1.0))) && all(mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f32>(1.0)) == vec4<f32>(1.0, 0.0, 0.0, 1.0))) && (mix(_globalUniforms.colorBlack.x, _globalUniforms.colorWhite.x, 0.5) == expectedBW.x)) && all(mix(_globalUniforms.colorBlack.xy, _globalUniforms.colorWhite.xy, vec2<f32>(0.5)) == expectedBW.xy)) && all(mix(_globalUniforms.colorBlack.xyz, _globalUniforms.colorWhite.xyz, vec3<f32>(0.5)) == expectedBW.xyz)) && all(mix(_globalUniforms.colorBlack, _globalUniforms.colorWhite, vec4<f32>(0.5)) == expectedBW)) && (0.5 == expectedBW.x)) && all(vec2<f32>(0.5) == expectedBW.xy)) && all(vec3<f32>(0.5) == expectedBW.xyz)) && all(vec4<f32>(0.5, 0.5, 0.5, 1.0) == expectedBW)) && (mix(_globalUniforms.colorWhite.x, _globalUniforms.testInputs.x, 0.0) == expectedWT.x)) && all(mix(_globalUniforms.colorWhite.xy, _globalUniforms.testInputs.xy, vec2<f32>(0.0, 0.5)) == expectedWT.xy)) && all(mix(_globalUniforms.colorWhite.xyz, _globalUniforms.testInputs.xyz, vec3<f32>(0.0, 0.5, 0.0)) == expectedWT.xyz)) && all(mix(_globalUniforms.colorWhite, _globalUniforms.testInputs, vec4<f32>(0.0, 0.5, 0.0, 1.0)) == expectedWT)) && (1.0 == expectedWT.x)) && all(vec2<f32>(1.0, 0.5) == expectedWT.xy)) && all(vec3<f32>(1.0, 0.5, 1.0) == expectedWT.xyz)) && all(vec4<f32>(1.0, 0.5, 1.0, 2.25) == expectedWT)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
