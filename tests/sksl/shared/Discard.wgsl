diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
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
