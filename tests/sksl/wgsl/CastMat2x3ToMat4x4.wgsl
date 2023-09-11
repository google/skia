diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var a: mat4x4<f32> = mat4x4<f32>(6.0, 0.0, 0.0, 0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 0.0, 6.0);
    let _skTemp0 = mat2x3<f32>(7.0, 0.0, 0.0, 0.0, 7.0, 0.0);
    var b: mat4x4<f32> = mat4x4<f32>(_skTemp0[0][0], _skTemp0[0][1], _skTemp0[0][2], 0.0, _skTemp0[1][0], _skTemp0[1][1], _skTemp0[1][2], 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    (*_stageOut).sk_FragColor.x = f32(select(1, 0, all(a[1] == b[1])));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
