diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    const m1: mat2x2<f16> = mat2x2<f16>(1.0h, 2.0h, 3.0h, 4.0h);
    const m2: mat2x2<f16> = mat2x2<f16>(1.0h, 2.0h, 3.0h, 4.0h);
    const m3: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    const m4: mat2x2<f32> = mat2x2<f32>(1.0, 2.0, 3.0, 4.0);
    (*_stageOut).sk_FragColor = ((m1 * m2) * mat2x2<f16>(m3 * m4)[0]).xyxy;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
