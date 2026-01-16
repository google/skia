diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: vec2<f32>,
  b: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor.x = f32(pack2x16float(vec2<f32>(_globalUniforms.a)));
    (*_stageOut).sk_FragColor.x = f32(pack2x16unorm(vec2<f32>(_globalUniforms.a)));
    (*_stageOut).sk_FragColor.x = f32(pack2x16snorm(vec2<f32>(_globalUniforms.a)));
    (*_stageOut).sk_FragColor.x = f32(pack4x8unorm(vec4<f32>(_globalUniforms.b)));
    (*_stageOut).sk_FragColor.x = f32(pack4x8snorm(vec4<f32>(_globalUniforms.b)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
