diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct VSIn {
  @location(0) pos: vec4<f32>,
};
struct VSOut {
  @builtin(position) sk_Position: vec4<f32>,
};
/* unsupported */ var<private> sk_PointSize: f32;
fn _skslMain(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    (*_stageOut).sk_Position = _stageIn.pos;
  }
}
@vertex fn main(_stageIn: VSIn) -> VSOut {
  var _stageOut: VSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
