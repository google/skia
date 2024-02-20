diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct VSOut {
  @builtin(position) sk_Position: vec4<f32>,
};
fn _skslMain() {
  {
  }
}
@vertex fn main() -> VSOut {
  var _stageOut: VSOut;
  _skslMain();
  return _stageOut;
}
