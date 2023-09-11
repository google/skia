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
    var result: f32 = 0.0;
    let _skTemp0 = mat3x3<f32>(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    var a: mat2x2<f32> = mat2x2<f32>(_skTemp0[0][0], _skTemp0[0][1], _skTemp0[1][0], _skTemp0[1][1]);
    result = result + a[0].x;
    let _skTemp1 = mat4x4<f32>(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    var b: mat2x2<f32> = mat2x2<f32>(_skTemp1[0][0], _skTemp1[0][1], _skTemp1[1][0], _skTemp1[1][1]);
    result = result + b[0].x;
    let _skTemp2 = mat4x4<f32>(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    var c: mat3x3<f32> = mat3x3<f32>(_skTemp2[0][0], _skTemp2[0][1], _skTemp2[0][2], _skTemp2[1][0], _skTemp2[1][1], _skTemp2[1][2], _skTemp2[2][0], _skTemp2[2][1], _skTemp2[2][2]);
    result = result + c[0].x;
    let _skTemp3 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    var d: mat3x3<f32> = mat3x3<f32>(_skTemp3[0][0], _skTemp3[0][1], 0.0, _skTemp3[1][0], _skTemp3[1][1], 0.0, 0.0, 0.0, 1.0);
    result = result + d[0].x;
    let _skTemp4 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    let _skTemp5 = mat3x3<f32>(_skTemp4[0][0], _skTemp4[0][1], 0.0, _skTemp4[1][0], _skTemp4[1][1], 0.0, 0.0, 0.0, 1.0);
    var e: mat4x4<f32> = mat4x4<f32>(_skTemp5[0][0], _skTemp5[0][1], _skTemp5[0][2], 0.0, _skTemp5[1][0], _skTemp5[1][1], _skTemp5[1][2], 0.0, _skTemp5[2][0], _skTemp5[2][1], _skTemp5[2][2], 0.0, 0.0, 0.0, 0.0, 1.0);
    result = result + e[0].x;
    let _skTemp6 = mat4x4<f32>(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    let _skTemp7 = mat3x3<f32>(_skTemp6[0][0], _skTemp6[0][1], _skTemp6[0][2], _skTemp6[1][0], _skTemp6[1][1], _skTemp6[1][2], _skTemp6[2][0], _skTemp6[2][1], _skTemp6[2][2]);
    var f: mat2x2<f32> = mat2x2<f32>(_skTemp7[0][0], _skTemp7[0][1], _skTemp7[1][0], _skTemp7[1][1]);
    result = result + f[0].x;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(result == 6.0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
