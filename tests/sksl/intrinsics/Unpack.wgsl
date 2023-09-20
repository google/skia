diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: u32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = unpack2x16float(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(_skTemp0)), (*_stageOut).sk_FragColor.zw).xyzw;
    let _skTemp1 = unpack2x16unorm(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(_skTemp1)), (*_stageOut).sk_FragColor.zw).xyzw;
    let _skTemp2 = unpack2x16snorm(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(_skTemp2)), (*_stageOut).sk_FragColor.zw).xyzw;
    let _skTemp3 = unpack4x8unorm(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>(_skTemp3);
    let _skTemp4 = unpack4x8snorm(_globalUniforms.a);
    (*_stageOut).sk_FragColor = vec4<f32>(_skTemp4);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
