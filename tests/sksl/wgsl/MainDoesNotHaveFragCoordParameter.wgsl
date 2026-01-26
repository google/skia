diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSIn {
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(_stageIn: FSIn) -> vec4<f16> {
  {
    return vec4<f16>(f16(_stageIn.sk_FragCoord.x), 0.0h, 0.0h, 1.0h);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn);
  return _stageOut;
}
