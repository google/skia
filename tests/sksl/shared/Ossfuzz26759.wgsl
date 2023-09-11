diagnostic(off, derivative_uniformity);
fn _skslMain() {
  {
    var i: i32;
    let _skTemp0 = i;
    i = i - i32(1);
  }
}
@fragment fn main() {
  _skslMain();
}
