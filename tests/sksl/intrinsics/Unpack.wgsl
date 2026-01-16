diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: u32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(unpack2x16float(_globalUniforms.a))), (*_stageOut).sk_FragColor.zw);
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(unpack2x16unorm(_globalUniforms.a))), (*_stageOut).sk_FragColor.zw);
    (*_stageOut).sk_FragColor = vec4<f32>((vec2<f32>(unpack2x16snorm(_globalUniforms.a))), (*_stageOut).sk_FragColor.zw);
    (*_stageOut).sk_FragColor = vec4<f32>(unpack4x8unorm(_globalUniforms.a));
    (*_stageOut).sk_FragColor = vec4<f32>(unpack4x8snorm(_globalUniforms.a));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
