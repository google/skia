diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
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
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
