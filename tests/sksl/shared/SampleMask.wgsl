diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(sample_mask) sk_SampleMaskIn: u32,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
  @builtin(sample_mask) sk_SampleMask: u32,
};
fn _skslMain(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_SampleMask = 4294967295u;
    (*_stageOut).sk_FragColor = vec4<f32>(f32(_stageIn.sk_SampleMaskIn)) * 0.00390625;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
