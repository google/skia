diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    loop {
      {
        (*_stageOut).sk_FragColor = vec4<f32>(1.0);
      }
      continuing {
        break if true;
      }
    }
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
