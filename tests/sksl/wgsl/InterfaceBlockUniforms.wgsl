diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _skRow2 {
    @size(16) r : vec2<f32>
};
struct _skMatrix22 {
    c : array<_skRow2, 2>
};
fn _skMatrixUnpack22(m : _skMatrix22) -> mat2x2<f32> {
    return mat2x2<f32>(m.c[0].r, m.c[1].r);
}
struct UniformBuffer {
  @size(32) m1: _skMatrix22,
  m2: _skMatrix22,
};
@group(12) @binding(34) var<uniform> _uniform0 : UniformBuffer;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(_skMatrixUnpack22(_uniform0.m1)[0].x, _skMatrixUnpack22(_uniform0.m1)[1].y, _skMatrixUnpack22(_uniform0.m2)[0].x, _skMatrixUnpack22(_uniform0.m2)[1].y);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}
