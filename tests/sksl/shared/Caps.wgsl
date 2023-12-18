diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
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
    (*_stageOut).sk_FragColor = vec4<f32>((vec3<f32>(f32(x), f32(y), f32(z))), (*_stageOut).sk_FragColor.w).xyzw;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
