### Compilation failed:

error: :16:20 error: no matching call to frexp(f32, i32)

2 candidate functions:
  frexp(T) -> __frexp_result_T  where: T is abstract-float, f32 or f16
  frexp(vecN<T>) -> __frexp_result_vecN_T  where: T is abstract-float, f32 or f16

    let _skTemp0 = frexp(value.x, _0_exp.x);
                   ^^^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
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
    var value: vec4<f32> = vec4<f32>(_globalUniforms.colorGreen.yyyy * 6.0);
    var _0_exp: vec4<i32>;
    var result: vec4<f32>;
    var ok: vec4<bool>;
    let _skTemp0 = frexp(value.x, _0_exp.x);
    result.x = _skTemp0;
    ok.x = (result.x == 0.75) && (_0_exp.x == 3);
    let _skTemp1 = frexp(value.xy, _0_exp.xy);
    result = vec4<f32>((_skTemp1), result.zw).xyzw;
    ok.y = (result.y == 0.75) && (_0_exp.y == 3);
    let _skTemp2 = frexp(value.xyz, _0_exp.xyz);
    result = vec4<f32>((_skTemp2), result.w).xyzw;
    ok.z = (result.z == 0.75) && (_0_exp.z == 3);
    let _skTemp3 = frexp(value, _0_exp);
    result = _skTemp3;
    ok.w = (result.w == 0.75) && (_0_exp.w == 3);
    let _skTemp4 = all(ok);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_skTemp4));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
