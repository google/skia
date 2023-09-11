diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const constVal: vec4<f32> = vec4<f32>(-1.25, 0.0, 0.75, 2.25);
    var expectedA: vec4<f32> = vec4<f32>(0.0, 0.0, 0.84375, 1.0);
    var expectedB: vec4<f32> = vec4<f32>(1.0, 0.0, 1.0, 1.0);
    let _skTemp0 = smoothstep(_globalUniforms.colorRed.y, _globalUniforms.colorGreen.y, -1.25);
    let _skTemp1 = smoothstep(vec2<f32>(_globalUniforms.colorRed.y), vec2<f32>(_globalUniforms.colorGreen.y), vec2<f32>(-1.25, 0.0));
    let _skTemp2 = smoothstep(vec3<f32>(_globalUniforms.colorRed.y), vec3<f32>(_globalUniforms.colorGreen.y), vec3<f32>(-1.25, 0.0, 0.75));
    let _skTemp3 = smoothstep(vec4<f32>(_globalUniforms.colorRed.y), vec4<f32>(_globalUniforms.colorGreen.y), constVal);
    let _skTemp4 = smoothstep(_globalUniforms.colorRed.x, _globalUniforms.colorGreen.x, -1.25);
    let _skTemp5 = smoothstep(_globalUniforms.colorRed.xy, _globalUniforms.colorGreen.xy, vec2<f32>(-1.25, 0.0));
    let _skTemp6 = smoothstep(_globalUniforms.colorRed.xyz, _globalUniforms.colorGreen.xyz, vec3<f32>(-1.25, 0.0, 0.75));
    let _skTemp7 = smoothstep(_globalUniforms.colorRed, _globalUniforms.colorGreen, constVal);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((((((0.0 == expectedA.x) && all(vec2<f32>(0.0) == expectedA.xy)) && all(vec3<f32>(0.0, 0.0, 0.84375) == expectedA.xyz)) && all(vec4<f32>(0.0, 0.0, 0.84375, 1.0) == expectedA)) && (0.0 == expectedA.x)) && all(vec2<f32>(0.0) == expectedA.xy)) && all(vec3<f32>(0.0, 0.0, 0.84375) == expectedA.xyz)) && all(vec4<f32>(0.0, 0.0, 0.84375, 1.0) == expectedA)) && (_skTemp0 == expectedA.x)) && all(_skTemp1 == expectedA.xy)) && all(_skTemp2 == expectedA.xyz)) && all(_skTemp3 == expectedA)) && (1.0 == expectedB.x)) && all(vec2<f32>(1.0, 0.0) == expectedB.xy)) && all(vec3<f32>(1.0, 0.0, 1.0) == expectedB.xyz)) && all(vec4<f32>(1.0, 0.0, 1.0, 1.0) == expectedB)) && (_skTemp4 == expectedB.x)) && all(_skTemp5 == expectedB.xy)) && all(_skTemp6 == expectedB.xyz)) && all(_skTemp7 == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
