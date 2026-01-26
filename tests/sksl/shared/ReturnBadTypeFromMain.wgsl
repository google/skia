diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
fn _skslMain() -> vec3<i32> {
  {
    return vec3<i32>(1, 2, 3);
  }
}
@fragment fn main() {
  _skslMain();
}
