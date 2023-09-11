diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  ah: vec2<f32>,
  bh: vec2<f32>,
  af: vec2<f32>,
  bf: vec2<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn cross_length_2d_ff2f2(a: vec2<f32>, b: vec2<f32>) -> f32 {
  {
    let _skTemp0 = determinant(mat2x2<f32>(a[0], a[1], b[0], b[1]));
    return _skTemp0;
  }
}
fn cross_length_2d_hh2h2(a: vec2<f32>, b: vec2<f32>) -> f32 {
  {
    let _skTemp1 = determinant(mat2x2<f32>(a[0], a[1], b[0], b[1]));
    return _skTemp1;
  }
}
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = cross_length_2d_hh2h2(_globalUniforms.ah, _globalUniforms.bh);
    (*_stageOut).sk_FragColor.x = _skTemp2;
    let _skTemp3 = cross_length_2d_ff2f2(_globalUniforms.af, _globalUniforms.bf);
    (*_stageOut).sk_FragColor.y = f32(_skTemp3);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
