diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSIn {
  @location(0) defaultVarying: f32,
  @location(1) @interpolate(linear) linearVarying: f32,
  @location(2) @interpolate(flat, either) flatVarying: f32,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f16>(f16(_stageIn.defaultVarying), f16(_stageIn.linearVarying), f16(_stageIn.flatVarying), 1.0h);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
