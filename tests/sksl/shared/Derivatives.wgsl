diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = 1.0;
    (*_stageOut).sk_FragColor.x = f16(dpdx(_skTemp0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
