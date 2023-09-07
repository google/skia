diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
fn _skslMain() {
  {
    var i: i32;
    let _skTemp0 = i;
    i = i - i32(1);
  }
}
@fragment fn main(_stageIn: FSIn) {
  _skslMain();
}
