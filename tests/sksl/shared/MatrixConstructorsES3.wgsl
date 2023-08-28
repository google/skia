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
  testMatrix2x2: mat2x2<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var f4: vec4<f32> = vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]);
    let _skTemp0 = f4.xy;
    let _skTemp1 = mat2x3<f32>(f4[0], f4[1], f4[2], f4[3], _skTemp0[0], _skTemp0[1]);
    let _skTemp2 = mat2x3<f32>(1.0, 2.0, 3.0, 4.0, 1.0, 2.0);
    var ok: bool = (all(_skTemp1[0] == _skTemp2[0]) && all(_skTemp1[1] == _skTemp2[1]));
    let _skTemp3 = f4.xyz;
    let _skTemp4 = f4.wxyz;
    let _skTemp5 = mat2x4<f32>(_skTemp3[0], _skTemp3[1], _skTemp3[2], _skTemp4[0], _skTemp4[1], _skTemp4[2], _skTemp4[3], f4.w);
    let _skTemp6 = mat2x4<f32>(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
    ok = ok && (all(_skTemp5[0] == _skTemp6[0]) && all(_skTemp5[1] == _skTemp6[1]));
    let _skTemp7 = f4.xy;
    let _skTemp8 = f4.zw;
    let _skTemp9 = mat3x3<f32>(_skTemp7[0], _skTemp7[1], _skTemp8[0], _skTemp8[1], f4[0], f4[1], f4[2], f4[3], f4.x);
    let _skTemp10 = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0);
    ok = ok && (all(_skTemp9[0] == _skTemp10[0]) && all(_skTemp9[1] == _skTemp10[1]) && all(_skTemp9[2] == _skTemp10[2]));
    let _skTemp11 = f4.xyz;
    let _skTemp12 = f4.wxyz;
    let _skTemp13 = mat4x2<f32>(_skTemp11[0], _skTemp11[1], _skTemp11[2], _skTemp12[0], _skTemp12[1], _skTemp12[2], _skTemp12[3], f4.w);
    let _skTemp14 = mat4x2<f32>(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
    ok = ok && (all(_skTemp13[0] == _skTemp14[0]) && all(_skTemp13[1] == _skTemp14[1]) && all(_skTemp13[2] == _skTemp14[2]) && all(_skTemp13[3] == _skTemp14[3]));
    let _skTemp15 = f4.yzwx;
    let _skTemp16 = f4.yzwx;
    let _skTemp17 = f4.yzw;
    let _skTemp18 = mat4x3<f32>(f4.x, _skTemp15[0], _skTemp15[1], _skTemp15[2], _skTemp15[3], _skTemp16[0], _skTemp16[1], _skTemp16[2], _skTemp16[3], _skTemp17[0], _skTemp17[1], _skTemp17[2]);
    let _skTemp19 = mat4x3<f32>(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
    ok = ok && (all(_skTemp18[0] == _skTemp19[0]) && all(_skTemp18[1] == _skTemp19[1]) && all(_skTemp18[2] == _skTemp19[2]) && all(_skTemp18[3] == _skTemp19[3]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
