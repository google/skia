diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct VSOut {
  @location(0) defaultVarying: f32,
  @location(1) @interpolate(linear) linearVarying: f32,
  @location(2) @interpolate(flat, either) flatVarying: f32,
  @builtin(position) sk_Position: vec4<f32>,
};
fn _skslMain(_stageOut: ptr<function, VSOut>) {
  {
    (*_stageOut).defaultVarying = 1.0;
    (*_stageOut).linearVarying = 2.0;
    (*_stageOut).flatVarying = 3.0;
  }
}
@vertex fn main() -> VSOut {
  var _stageOut: VSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
