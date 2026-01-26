diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct IndirectDispatchArgs {
  x: i32,
  y: i32,
  z: i32,
};
var<workgroup> outX: i32;
var<workgroup> outY: i32;
var<workgroup> outZ: i32;
fn one_i() -> i32 {
  {
    return 1;
  }
}
fn two_i() -> i32 {
  {
    return 2;
  }
}
fn three_i() -> i32 {
  {
    return 3;
  }
}
fn _skslMain() {
  {
    outX = IndirectDispatchArgs(one_i(), two_i(), three_i()).x;
    outY = IndirectDispatchArgs(one_i(), two_i(), three_i()).y;
    outZ = IndirectDispatchArgs(one_i(), two_i(), three_i()).z;
  }
}
@compute @workgroup_size(16, 16, 1) fn main() {
  _skslMain();
}
