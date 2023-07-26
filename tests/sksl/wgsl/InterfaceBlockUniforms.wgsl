diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct UniformBuffer {
  @size(32) m1: mat2x2<f32>,
  m2: mat2x2<f32>,
};
@group(12) @binding(34) var<uniform> _uniform0 : UniformBuffer;
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(_uniform0.m1[0].x, _uniform0.m1[1].y, _uniform0.m2[0].x, _uniform0.m2[1].y);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}
