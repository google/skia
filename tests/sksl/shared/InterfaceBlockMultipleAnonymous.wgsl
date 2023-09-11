diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct testBlockA {
  x: vec2<f32>,
};
@group(0) @binding(1) var<uniform> _uniform0 : testBlockA;
struct testBlockB {
  y: vec2<f32>,
};
@group(0) @binding(2) var<uniform> _uniform1 : testBlockB;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(vec2<f32>(_uniform0.x), vec2<f32>(_uniform1.y));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
