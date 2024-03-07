diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var value: vec4<f32> = vec4<f32>(2.5, -2.5, 8.0, -0.125);
    const expectedWhole: vec4<f32> = vec4<f32>(2.0, -2.0, 8.0, 0.0);
    const expectedFraction: vec4<f32> = vec4<f32>(0.5, -0.5, 0.0, -0.125);
    var ok: vec4<bool> = vec4<bool>(false);
    var whole: vec4<f32>;
    var fraction: vec4<f32>;
    let _skTemp0 = modf(value.x);
    whole.x = _skTemp0.whole;
    fraction.x = _skTemp0.fract;
    ok.x = (whole.x == 2.0) && (fraction.x == 0.5);
    let _skTemp1 = modf(value.xy);
    whole = vec4<f32>((_skTemp1.whole), whole.zw).xyzw;
    fraction = vec4<f32>((_skTemp1.fract), fraction.zw).xyzw;
    ok.y = all(whole.xy == vec2<f32>(2.0, -2.0)) && all(fraction.xy == vec2<f32>(0.5, -0.5));
    let _skTemp2 = modf(value.xyz);
    whole = vec4<f32>((_skTemp2.whole), whole.w).xyzw;
    fraction = vec4<f32>((_skTemp2.fract), fraction.w).xyzw;
    ok.z = all(whole.xyz == vec3<f32>(2.0, -2.0, 8.0)) && all(fraction.xyz == vec3<f32>(0.5, -0.5, 0.0));
    let _skTemp3 = modf(value);
    whole = _skTemp3.whole;
    fraction = _skTemp3.fract;
    ok.w = all(whole == expectedWhole) && all(fraction == expectedFraction);
    let _skTemp4 = all(ok);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_skTemp4));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
