diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSIn {
  @builtin(sample_mask) sk_SampleMaskIn: u32,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
  @builtin(sample_mask) sk_SampleMask: u32,
};
fn samplemaskin_as_color_h4(_stageIn: FSIn) -> vec4<f16> {
  {
    return vec4<f16>(f16(_stageIn.sk_SampleMaskIn));
  }
}
fn clear_samplemask_v(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_SampleMask = 0u;
  }
}
fn reset_samplemask_v(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_SampleMask = _stageIn.sk_SampleMaskIn;
  }
}
fn _skslMain(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    clear_samplemask_v(_stageOut);
    reset_samplemask_v(_stageIn, _stageOut);
    (*_stageOut).sk_SampleMask = 4294967295u;
    (*_stageOut).sk_FragColor = samplemaskin_as_color_h4(_stageIn) * 0.00390625h;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
