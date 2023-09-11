diagnostic(off, derivative_uniformity);
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
    let _skTemp0 = f4.xyz;
    let _skTemp1 = mat2x2<f32>(_skTemp0[0], _skTemp0[1], _skTemp0[2], 4.0);
    let _skTemp2 = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    var ok: bool = (all(_skTemp1[0] == _skTemp2[0]) && all(_skTemp1[1] == _skTemp2[1]));
    let _skTemp3 = f4.xy;
    let _skTemp4 = f4.zw;
    let _skTemp5 = mat3x3<f32>(_skTemp3[0], _skTemp3[1], _skTemp4[0], _skTemp4[1], f4[0], f4[1], f4[2], f4[3], f4.x);
    let _skTemp6 = mat3x3<f32>(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0);
    ok = ok && (all(_skTemp5[0] == _skTemp6[0]) && all(_skTemp5[1] == _skTemp6[1]) && all(_skTemp5[2] == _skTemp6[2]));
    let _skTemp7 = f4.xyz;
    let _skTemp8 = f4.wxy;
    let _skTemp9 = f4.zwxy;
    let _skTemp10 = f4.zw;
    let _skTemp11 = mat4x4<f32>(_skTemp7[0], _skTemp7[1], _skTemp7[2], _skTemp8[0], _skTemp8[1], _skTemp8[2], _skTemp9[0], _skTemp9[1], _skTemp9[2], _skTemp9[3], _skTemp10[0], _skTemp10[1], f4[0], f4[1], f4[2], f4[3]);
    let _skTemp12 = mat4x4<f32>(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0);
    ok = ok && (all(_skTemp11[0] == _skTemp12[0]) && all(_skTemp11[1] == _skTemp12[1]) && all(_skTemp11[2] == _skTemp12[2]) && all(_skTemp11[3] == _skTemp12[3]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
