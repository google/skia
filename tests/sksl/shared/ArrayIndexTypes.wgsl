diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    const R_array: array<f32, 4> = array<f32, 4>(1.0, 2.0, 3.0, 4.0);
    const x: i32 = 0;
    const y: u32 = 1u;
    const z: i32 = 2;
    const w: u32 = 3u;
    (*_stageOut).sk_FragColor = vec4<f32>(f32(R_array[x]), f32(R_array[y]), f32(R_array[z]), f32(R_array[w]));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
