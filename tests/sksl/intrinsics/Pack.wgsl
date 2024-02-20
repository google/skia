diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: vec2<f32>,
  b: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = pack2x16float(vec2<f32>(_globalUniforms.a));
    (*_stageOut).sk_FragColor.x = f32(_skTemp0);
    let _skTemp1 = pack2x16unorm(vec2<f32>(_globalUniforms.a));
    (*_stageOut).sk_FragColor.x = f32(_skTemp1);
    let _skTemp2 = pack2x16snorm(vec2<f32>(_globalUniforms.a));
    (*_stageOut).sk_FragColor.x = f32(_skTemp2);
    let _skTemp3 = pack4x8unorm(vec4<f32>(_globalUniforms.b));
    (*_stageOut).sk_FragColor.x = f32(_skTemp3);
    let _skTemp4 = pack4x8snorm(vec4<f32>(_globalUniforms.b));
    (*_stageOut).sk_FragColor.x = f32(_skTemp4);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
