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
    var result: f32 = 0.0;
    let _skTemp0 = mat2x3<f32>(1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    var g: mat3x3<f32> = mat3x3<f32>(_skTemp0[0][0], _skTemp0[0][1], _skTemp0[0][2], _skTemp0[1][0], _skTemp0[1][1], _skTemp0[1][2], 0.0, 0.0, 1.0);
    result = result + g[0].x;
    let _skTemp1 = mat3x2<f32>(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    var h: mat3x3<f32> = mat3x3<f32>(_skTemp1[0][0], _skTemp1[0][1], 0.0, _skTemp1[1][0], _skTemp1[1][1], 0.0, _skTemp1[2][0], _skTemp1[2][1], 1.0);
    result = result + h[0].x;
    let _skTemp2 = mat4x2<f32>(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0);
    let _skTemp3 = mat4x3<f32>(_skTemp2[0][0], _skTemp2[0][1], 0.0, _skTemp2[1][0], _skTemp2[1][1], 0.0, _skTemp2[2][0], _skTemp2[2][1], 1.0, _skTemp2[3][0], _skTemp2[3][1], 0.0);
    var i: mat4x4<f32> = mat4x4<f32>(_skTemp3[0][0], _skTemp3[0][1], _skTemp3[0][2], 0.0, _skTemp3[1][0], _skTemp3[1][1], _skTemp3[1][2], 0.0, _skTemp3[2][0], _skTemp3[2][1], _skTemp3[2][2], 0.0, _skTemp3[3][0], _skTemp3[3][1], _skTemp3[3][2], 1.0);
    result = result + i[0].x;
    let _skTemp4 = mat2x4<f32>(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    let _skTemp5 = mat3x4<f32>(_skTemp4[0][0], _skTemp4[0][1], _skTemp4[0][2], _skTemp4[0][3], _skTemp4[1][0], _skTemp4[1][1], _skTemp4[1][2], _skTemp4[1][3], 0.0, 0.0, 1.0, 0.0);
    var j: mat4x4<f32> = mat4x4<f32>(_skTemp5[0][0], _skTemp5[0][1], _skTemp5[0][2], _skTemp5[0][3], _skTemp5[1][0], _skTemp5[1][1], _skTemp5[1][2], _skTemp5[1][3], _skTemp5[2][0], _skTemp5[2][1], _skTemp5[2][2], _skTemp5[2][3], 0.0, 0.0, 0.0, 1.0);
    result = result + j[0].x;
    let _skTemp6 = mat4x2<f32>(1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0);
    var k: mat2x4<f32> = mat2x4<f32>(_skTemp6[0][0], _skTemp6[0][1], 0.0, 0.0, _skTemp6[1][0], _skTemp6[1][1], 0.0, 0.0);
    result = result + k[0].x;
    let _skTemp7 = mat2x4<f32>(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    var l: mat4x2<f32> = mat4x2<f32>(_skTemp7[0][0], _skTemp7[0][1], _skTemp7[1][0], _skTemp7[1][1], 0.0, 0.0, 0.0, 0.0);
    result = result + l[0].x;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(result == 6.0));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
