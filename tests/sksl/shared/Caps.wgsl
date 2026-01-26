diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var x: i32 = 0;
    var y: i32 = 0;
    var z: i32 = 0;
    if true {
      x = 1;
    }
    if false {
      y = 1;
    }
    if true {
      z = 1;
    }
    (*_stageOut).sk_FragColor = vec4<f16>((vec3<f16>(f16(x), f16(y), f16(z))), (*_stageOut).sk_FragColor.w);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
