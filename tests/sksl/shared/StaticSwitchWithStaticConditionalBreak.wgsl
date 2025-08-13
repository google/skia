diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var x: f32 = 0.0;
    switch 0 {
      case 0, 1 {
        var _skTemp0: bool = false;
        if 0 == 0 {
          x = 0.0;
          if x < 1.0 {
            break;
          }
          // fallthrough
        }
        x = 1.0;
      }
      case default {}
    }
    (*_stageOut).sk_FragColor = vec4<f32>(f32(x));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
