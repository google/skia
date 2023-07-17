diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
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
fn cross_length_2d_ff2f2(_skParam0: vec2<f32>, _skParam1: vec2<f32>) -> f32 {
  let a = _skParam0;
  let b = _skParam1;
  {
    let _skTemp0 = determinant(mat2x2<f32>(a[0], a[1], b[0], b[1]));
    return _skTemp0;
  }
}
fn cross_length_2d_hh2h2(_skParam0: vec2<f32>, _skParam1: vec2<f32>) -> f32 {
  let a = _skParam0;
  let b = _skParam1;
  {
    let _skTemp1 = determinant(mat2x2<f32>(a[0], a[1], b[0], b[1]));
    return _skTemp1;
  }
}
fn main(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = cross_length_2d_hh2h2(_globalUniforms.ah, _globalUniforms.bh);
    (*_stageOut).sk_FragColor.x = _skTemp2;
    let _skTemp3 = cross_length_2d_ff2f2(_globalUniforms.af, _globalUniforms.bf);
    (*_stageOut).sk_FragColor.y = f32(_skTemp3);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}
