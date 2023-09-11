diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var value: f32 = 0.0;
    switch 0 {
      case 0, 1 {
        var _skTemp0: bool = false;
        if 0 == 0 {
          value = 0.0;
          if _globalUniforms.unknownInput == 2.0 {
            break;
          }
          // fallthrough
        }
        value = 1.0;
      }
      case default {}
    }
    (*_stageOut).sk_FragColor = vec4<f32>(value);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
