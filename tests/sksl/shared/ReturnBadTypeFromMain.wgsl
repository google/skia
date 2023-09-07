diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
fn _skslMain() -> vec3<i32> {
  {
    return vec3<i32>(1, 2, 3);
  }
}
@fragment fn main(_stageIn: FSIn) {
  _skslMain();
}
