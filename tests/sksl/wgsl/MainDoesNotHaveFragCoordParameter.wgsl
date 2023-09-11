diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_stageIn: FSIn) -> vec4<f32> {
  {
    return vec4<f32>(f32(_stageIn.sk_FragCoord.x), 0.0, 0.0, 1.0);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn);
  return _stageOut;
}
