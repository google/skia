diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
var<private> b: i32;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let _skTemp0 = ldexp(_globalUniforms.a, b);
    (*_stageOut).sk_FragColor.x = f32(_skTemp0);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
