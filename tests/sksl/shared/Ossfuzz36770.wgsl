### Compilation failed:

error: :2:1 error: structures must have at least one member
struct FSIn {
^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
};
fn _skslMain() {
  {
  }
}
@fragment fn main(_stageIn: FSIn) {
  _skslMain();
}

1 error
