diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
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
    var uintValues: vec4<u32> = vec4<u32>(_globalUniforms.testInputs * 100.0 + 200.0);
    var expectedA: vec4<u32> = vec4<u32>(100u, 200u, 275u, 300u);
    const clampLow: vec4<u32> = vec4<u32>(100u, 0u, 0u, 300u);
    var expectedB: vec4<u32> = vec4<u32>(100u, 200u, 250u, 425u);
    const clampHigh: vec4<u32> = vec4<u32>(300u, 400u, 250u, 500u);
    let _skTemp0 = clamp(uintValues.x, 100u, 300u);
    let _skTemp1 = clamp(uintValues.xy, vec2<u32>(100u), vec2<u32>(300u));
    let _skTemp2 = clamp(uintValues.xyz, vec3<u32>(100u), vec3<u32>(300u));
    let _skTemp3 = clamp(uintValues, vec4<u32>(100u), vec4<u32>(300u));
    let _skTemp4 = clamp(uintValues.x, 100u, 300u);
    let _skTemp5 = clamp(uintValues.xy, vec2<u32>(100u, 0u), vec2<u32>(300u, 400u));
    let _skTemp6 = clamp(uintValues.xyz, vec3<u32>(100u, 0u, 0u), vec3<u32>(300u, 400u, 250u));
    let _skTemp7 = clamp(uintValues, clampLow, clampHigh);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((_skTemp0 == expectedA.x) && all(_skTemp1 == expectedA.xy)) && all(_skTemp2 == expectedA.xyz)) && all(_skTemp3 == expectedA)) && (100u == expectedA.x)) && all(vec2<u32>(100u, 200u) == expectedA.xy)) && all(vec3<u32>(100u, 200u, 275u) == expectedA.xyz)) && all(vec4<u32>(100u, 200u, 275u, 300u) == expectedA)) && (_skTemp4 == expectedB.x)) && all(_skTemp5 == expectedB.xy)) && all(_skTemp6 == expectedB.xyz)) && all(_skTemp7 == expectedB)) && (100u == expectedB.x)) && all(vec2<u32>(100u, 200u) == expectedB.xy)) && all(vec3<u32>(100u, 200u, 250u) == expectedB.xyz)) && all(vec4<u32>(100u, 200u, 250u, 425u) == expectedB)));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
