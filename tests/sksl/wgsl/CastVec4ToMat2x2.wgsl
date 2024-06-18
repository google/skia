diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var m1: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    var m2: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    var m3: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    var m4: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    (*_stageOut).sk_FragColor = ((m1 * m2) * mat2x2<f32>(m3 * m4)[0]).xyxy;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
