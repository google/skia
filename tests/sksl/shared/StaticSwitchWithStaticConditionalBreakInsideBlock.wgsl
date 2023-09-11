diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
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
            {
              (*_stageOut).sk_FragColor = vec4<f32>(f32(x));
              break;
            }
          }
          // fallthrough
        }
        x = 1.0;
      }
      case default {}
    }
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
