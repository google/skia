diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
fn foo_ff(_skParam0: array<f32, 2>) -> f32 {
  var v = _skParam0;
  {
    v[0] = v[1];
    return v[0];
  }
}
fn _skslMain() {
  {
    var y: array<f32, 2>;
    let _skTemp0 = foo_ff(y);
  }
}
@fragment fn main() {
  _skslMain();
}
