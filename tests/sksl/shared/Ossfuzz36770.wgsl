### Compilation failed:

error: :5:1 error: structures must have at least one member
struct FSOut {
^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
};
fn _skslMain() {
  {
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain();
  return _stageOut;
}

1 error
