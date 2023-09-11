diagnostic(off, derivative_uniformity);
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
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var expectedBW: vec4<f32> = vec4<f32>(0.5, 0.5, 0.5, 1.0);
    var expectedWT: vec4<f32> = vec4<f32>(1.0, 0.5, 1.0, 2.25);
    let _skTemp0 = mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f32>(0.0));
    let _skTemp1 = mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f32>(0.25));
    let _skTemp2 = mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f32>(0.75));
    let _skTemp3 = mix(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<f32>(1.0));
    let _skTemp4 = mix(_globalUniforms.colorBlack.x, _globalUniforms.colorWhite.x, 0.5);
    let _skTemp5 = mix(_globalUniforms.colorBlack.xy, _globalUniforms.colorWhite.xy, vec2<f32>(0.5));
    let _skTemp6 = mix(_globalUniforms.colorBlack.xyz, _globalUniforms.colorWhite.xyz, vec3<f32>(0.5));
    let _skTemp7 = mix(_globalUniforms.colorBlack, _globalUniforms.colorWhite, vec4<f32>(0.5));
    let _skTemp8 = mix(_globalUniforms.colorWhite.x, _globalUniforms.testInputs.x, 0.0);
    let _skTemp9 = mix(_globalUniforms.colorWhite.xy, _globalUniforms.testInputs.xy, vec2<f32>(0.0, 0.5));
    let _skTemp10 = mix(_globalUniforms.colorWhite.xyz, _globalUniforms.testInputs.xyz, vec3<f32>(0.0, 0.5, 0.0));
    let _skTemp11 = mix(_globalUniforms.colorWhite, _globalUniforms.testInputs, vec4<f32>(0.0, 0.5, 0.0, 1.0));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((((((((((((all(_skTemp0 == vec4<f32>(0.0, 1.0, 0.0, 1.0)) && all(_skTemp1 == vec4<f32>(0.25, 0.75, 0.0, 1.0))) && all(_skTemp2 == vec4<f32>(0.75, 0.25, 0.0, 1.0))) && all(_skTemp3 == vec4<f32>(1.0, 0.0, 0.0, 1.0))) && (_skTemp4 == expectedBW.x)) && all(_skTemp5 == expectedBW.xy)) && all(_skTemp6 == expectedBW.xyz)) && all(_skTemp7 == expectedBW)) && (0.5 == expectedBW.x)) && all(vec2<f32>(0.5) == expectedBW.xy)) && all(vec3<f32>(0.5) == expectedBW.xyz)) && all(vec4<f32>(0.5, 0.5, 0.5, 1.0) == expectedBW)) && (_skTemp8 == expectedWT.x)) && all(_skTemp9 == expectedWT.xy)) && all(_skTemp10 == expectedWT.xyz)) && all(_skTemp11 == expectedWT)) && (1.0 == expectedWT.x)) && all(vec2<f32>(1.0, 0.5) == expectedWT.xy)) && all(vec3<f32>(1.0, 0.5, 1.0) == expectedWT.xyz)) && all(vec4<f32>(1.0, 0.5, 1.0, 2.25) == expectedWT)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
