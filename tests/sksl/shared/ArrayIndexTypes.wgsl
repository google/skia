diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn main(_stageOut: ptr<function, FSOut>) {
  {
    var _array: array<f32, 4> = array<f32, 4>(1.0, 2.0, 3.0, 4.0);
    var x: i32 = 0;
    var y: u32 = 1u;
    var z: i32 = 2;
    var w: u32 = 3u;
    (*_stageOut).sk_FragColor = vec4<f32>(f32(_array[x]), f32(_array[y]), f32(_array[z]), f32(_array[w]));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}
