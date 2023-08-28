### Compilation failed:

error: :22:20 error: no matching call to modf(f32, f32)

2 candidate functions:
  modf(T) -> __modf_result_T  where: T is abstract-float, f32 or f16
  modf(vecN<T>) -> __modf_result_vecN_T  where: T is abstract-float, f32 or f16

    let _skTemp0 = modf(value.x, whole.x);
                   ^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
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
    let _skTemp0 = modf(value.x, whole.x);
    fraction.x = _skTemp0;
    ok.x = (whole.x == 2.0) && (fraction.x == 0.5);
    let _skTemp1 = modf(value.xy, whole.xy);
    fraction = vec4<f32>((_skTemp1), fraction.zw).xyzw;
    ok.y = all(whole.xy == vec2<f32>(2.0, -2.0)) && all(fraction.xy == vec2<f32>(0.5, -0.5));
    let _skTemp2 = modf(value.xyz, whole.xyz);
    fraction = vec4<f32>((_skTemp2), fraction.w).xyzw;
    ok.z = all(whole.xyz == vec3<f32>(2.0, -2.0, 8.0)) && all(fraction.xyz == vec3<f32>(0.5, -0.5, 0.0));
    let _skTemp3 = modf(value, whole);
    fraction = _skTemp3;
    ok.w = all(whole == expectedWhole) && all(fraction == expectedFraction);
    let _skTemp4 = all(ok);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_skTemp4));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
