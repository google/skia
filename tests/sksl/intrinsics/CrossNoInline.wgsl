diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  ah: vec2<f32>,
  bh: vec2<f32>,
  af: vec2<f32>,
  bf: vec2<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn cross_length_2d_ff2f2(a: vec2<f32>, b: vec2<f32>) -> f32 {
  {
    return determinant(mat2x2<f32>(a[0], a[1], b[0], b[1]));
  }
}
fn cross_length_2d_hh2h2(a: vec2<f32>, b: vec2<f32>) -> f32 {
  {
    return determinant(mat2x2<f32>(a[0], a[1], b[0], b[1]));
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor.x = cross_length_2d_hh2h2(_globalUniforms.ah, _globalUniforms.bh);
    (*_stageOut).sk_FragColor.y = f32(cross_length_2d_ff2f2(_globalUniforms.af, _globalUniforms.bf));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
