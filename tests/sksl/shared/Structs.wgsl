diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct A {
  x: i32,
  y: i32,
};
var<private> a1: A;
struct B {
  x: f32,
  y: array<f32, 2>,
  z: A,
};
var<private> b1: B;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    a1.x = 0;
    b1.x = 0.0;
    (*_stageOut).sk_FragColor.x = f16(a1.x) + f16(b1.x);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
