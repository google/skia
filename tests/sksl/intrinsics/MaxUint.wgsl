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
    let _skTemp0 = abs(_globalUniforms.testInputs);
    var uintValues: vec4<u32> = vec4<u32>(_skTemp0 * 100.0);
    var uintGreen: vec4<u32> = vec4<u32>(_globalUniforms.colorGreen * 100.0);
    var expectedA: vec4<u32> = vec4<u32>(125u, 80u, 80u, 225u);
    var expectedB: vec4<u32> = vec4<u32>(125u, 100u, 75u, 225u);
    let _skTemp1 = max(uintValues.x, 80u);
    let _skTemp2 = max(uintValues.xy, vec2<u32>(80u));
    let _skTemp3 = max(uintValues.xyz, vec3<u32>(80u));
    let _skTemp4 = max(uintValues, vec4<u32>(80u));
    let _skTemp5 = max(uintValues.x, uintGreen.x);
    let _skTemp6 = max(uintValues.xy, uintGreen.xy);
    let _skTemp7 = max(uintValues.xyz, uintGreen.xyz);
    let _skTemp8 = max(uintValues, uintGreen);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp1 == expectedA.x) && all(_skTemp2 == expectedA.xy)) && all(_skTemp3 == expectedA.xyz)) && all(_skTemp4 == expectedA)) && (125u == expectedA.x)) && all(vec2<u32>(125u, 80u) == expectedA.xy)) && all(vec3<u32>(125u, 80u, 80u) == expectedA.xyz)) && all(vec4<u32>(125u, 80u, 80u, 225u) == expectedA)) && (_skTemp5 == expectedB.x)) && all(_skTemp6 == expectedB.xy)) && all(_skTemp7 == expectedB.xyz)) && all(_skTemp8 == expectedB)) && (125u == expectedB.x)) && all(vec2<u32>(125u, 100u) == expectedB.xy)) && all(vec3<u32>(125u, 100u, 75u) == expectedB.xyz)) && all(vec4<u32>(125u, 100u, 75u, 225u) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
