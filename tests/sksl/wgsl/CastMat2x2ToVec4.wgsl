diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  f: mat2x2<f32>,
  h: mat2x2<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = (vec4<f32>(_globalUniforms.h[0], _globalUniforms.h[1]) + vec4<f32>(vec4<f32>(_globalUniforms.f[0], _globalUniforms.f[1]))) + vec4<f32>(vec4<f32>(vec4<f32>(_globalUniforms.h[0], _globalUniforms.h[1])) + vec4<f32>(_globalUniforms.f[0], _globalUniforms.f[1]));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
