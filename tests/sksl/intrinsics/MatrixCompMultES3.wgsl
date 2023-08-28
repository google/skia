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
    let _skTemp0 = mat2x4<f32>(9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0, 9.0);
    let _skTemp1 = mat2x4<f32>(_globalUniforms.colorRed[0], _globalUniforms.colorRed[1], _globalUniforms.colorRed[2], _globalUniforms.colorRed[3], _globalUniforms.colorGreen[0], _globalUniforms.colorGreen[1], _globalUniforms.colorGreen[2], _globalUniforms.colorGreen[3]);
    let _skTemp2 = mat2x4<f32>(_skTemp0[0] * _skTemp1[0], _skTemp0[1] * _skTemp1[1]);
    var h24: mat2x4<f32> = _skTemp2;
    let _skTemp3 = mat4x2<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    let _skTemp4 = mat4x2<f32>(_globalUniforms.colorRed[0], _globalUniforms.colorRed[1], _globalUniforms.colorRed[2], _globalUniforms.colorRed[3], _globalUniforms.colorGreen[0], _globalUniforms.colorGreen[1], _globalUniforms.colorGreen[2], _globalUniforms.colorGreen[3]);
    let _skTemp5 = mat4x2<f32>(_skTemp3[0] * _skTemp4[0], _skTemp3[1] * _skTemp4[1], _skTemp3[2] * _skTemp4[2], _skTemp3[3] * _skTemp4[3]);
    var h42: mat4x2<f32> = _skTemp5;
    var f43: mat4x3<f32> = mat4x3<f32>(12.0, 22.0, 30.0, 36.0, 40.0, 42.0, 42.0, 40.0, 36.0, 30.0, 22.0, 12.0);
    let _skTemp6 = mat2x4<f32>(9.0, 0.0, 0.0, 9.0, 0.0, 9.0, 0.0, 9.0);
    let _skTemp7 = mat4x2<f32>(1.0, 0.0, 0.0, 4.0, 0.0, 6.0, 0.0, 8.0);
    let _skTemp8 = mat4x3<f32>(12.0, 22.0, 30.0, 36.0, 40.0, 42.0, 42.0, 40.0, 36.0, 30.0, 22.0, 12.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((all(h24[0] == _skTemp6[0]) && all(h24[1] == _skTemp6[1])) && (all(h42[0] == _skTemp7[0]) && all(h42[1] == _skTemp7[1]) && all(h42[2] == _skTemp7[2]) && all(h42[3] == _skTemp7[3]))) && (all(f43[0] == _skTemp8[0]) && all(f43[1] == _skTemp8[1]) && all(f43[2] == _skTemp8[2]) && all(f43[3] == _skTemp8[3]))));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
