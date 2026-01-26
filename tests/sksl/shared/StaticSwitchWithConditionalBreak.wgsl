diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  unknownInput: f32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var value: f16 = 0.0h;
    switch 0 {
      case 0, 1 {
        var _skTemp0: bool = false;
        if 0 == 0 {
          value = 0.0h;
          if _globalUniforms.unknownInput == 2.0 {
            break;
          }
        }
        value = 1.0h;
      }
      case default {}
    }
    (*_stageOut).sk_FragColor = vec4<f16>(value);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
