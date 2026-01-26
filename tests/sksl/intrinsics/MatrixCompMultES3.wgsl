diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const _skTemp0 = mat2x4<f16>(9.0h, 9.0h, 9.0h, 9.0h, 9.0h, 9.0h, 9.0h, 9.0h);
    let _skTemp1 = mat2x4<f16>(_globalUniforms.colorRed[0], _globalUniforms.colorRed[1], _globalUniforms.colorRed[2], _globalUniforms.colorRed[3], _globalUniforms.colorGreen[0], _globalUniforms.colorGreen[1], _globalUniforms.colorGreen[2], _globalUniforms.colorGreen[3]);
    let h24: mat2x4<f16> = mat2x4<f16>(_skTemp0[0] * _skTemp1[0], _skTemp0[1] * _skTemp1[1]);
    const _skTemp2 = mat4x2<f16>(1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h, 7.0h, 8.0h);
    let _skTemp3 = mat4x2<f16>(_globalUniforms.colorRed[0], _globalUniforms.colorRed[1], _globalUniforms.colorRed[2], _globalUniforms.colorRed[3], _globalUniforms.colorGreen[0], _globalUniforms.colorGreen[1], _globalUniforms.colorGreen[2], _globalUniforms.colorGreen[3]);
    let h42: mat4x2<f16> = mat4x2<f16>(_skTemp2[0] * _skTemp3[0], _skTemp2[1] * _skTemp3[1], _skTemp2[2] * _skTemp3[2], _skTemp2[3] * _skTemp3[3]);
    const f43: mat4x3<f32> = mat4x3<f32>(12.0, 22.0, 30.0, 36.0, 40.0, 42.0, 42.0, 40.0, 36.0, 30.0, 22.0, 12.0);
    const _skTemp4 = mat2x4<f16>(9.0h, 0.0h, 0.0h, 9.0h, 0.0h, 9.0h, 0.0h, 9.0h);
    const _skTemp5 = mat4x2<f16>(1.0h, 0.0h, 0.0h, 4.0h, 0.0h, 6.0h, 0.0h, 8.0h);
    const _skTemp6 = mat4x3<f32>(12.0, 22.0, 30.0, 36.0, 40.0, 42.0, 42.0, 40.0, 36.0, 30.0, 22.0, 12.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((all(h24[0] == _skTemp4[0]) && all(h24[1] == _skTemp4[1])) && (all(h42[0] == _skTemp5[0]) && all(h42[1] == _skTemp5[1]) && all(h42[2] == _skTemp5[2]) && all(h42[3] == _skTemp5[3]))) && (all(f43[0] == _skTemp6[0]) && all(f43[1] == _skTemp6[1]) && all(f43[2] == _skTemp6[2]) && all(f43[3] == _skTemp6[3]))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
