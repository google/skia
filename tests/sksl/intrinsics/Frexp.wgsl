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
fn checkIntrinsicAsFunctionArg_bf3i3(f3: vec3<f32>, e3: vec3<i32>) -> bool {
  {
    return all(f3 == vec3<f32>(0.75)) && all(e3 == vec3<i32>(3));
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var value: vec4<f32> = vec4<f32>(_globalUniforms.colorGreen.yyyy * 6.0);
    var _0_exp: vec4<i32>;
    var result: vec4<f32>;
    var ok: vec4<bool>;
    let _skTemp0 = frexp(value.x);
    _0_exp.x = _skTemp0.exp;
    result.x = _skTemp0.fract;
    ok.x = (result.x == 0.75) && (_0_exp.x == 3);
    let _skTemp1 = frexp(value.xy);
    _0_exp = vec4<i32>((_skTemp1.exp), _0_exp.zw);
    result = vec4<f32>((_skTemp1.fract), result.zw);
    ok.y = (result.y == 0.75) && (_0_exp.y == 3);
    let _skTemp2 = frexp(value.xyz);
    _0_exp = vec4<i32>((_skTemp2.exp), _0_exp.w);
    result = vec4<f32>((_skTemp2.fract), result.w);
    ok.z = (result.z == 0.75) && (_0_exp.z == 3);
    let _skTemp3 = frexp(value);
    _0_exp = _skTemp3.exp;
    result = _skTemp3.fract;
    ok.w = (result.w == 0.75) && (_0_exp.w == 3);
    let _skTemp4 = frexp(value.wzy);
    _0_exp = vec4<i32>((_skTemp4.exp), _0_exp.y).ywxz;
    let _skTemp5 = checkIntrinsicAsFunctionArg_bf3i3(_skTemp4.fract.yxz, _0_exp.yxz);
    var funcOk: bool = _skTemp5;
    let _skTemp6 = all(ok);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_skTemp6 && funcOk));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
