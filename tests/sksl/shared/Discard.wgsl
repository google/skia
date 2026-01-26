diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain() {
  {
    {
      discard;
    }
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain();
  return _stageOut;
}
