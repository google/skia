diagnostic(off, derivative_uniformity);
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
    let _skTemp0 = one_i();
    let _skTemp1 = two_i();
    let _skTemp2 = three_i();
    outX = IndirectDispatchArgs(_skTemp0, _skTemp1, _skTemp2).x;
    let _skTemp3 = one_i();
    let _skTemp4 = two_i();
    let _skTemp5 = three_i();
    outY = IndirectDispatchArgs(_skTemp3, _skTemp4, _skTemp5).y;
    let _skTemp6 = one_i();
    let _skTemp7 = two_i();
    let _skTemp8 = three_i();
    outZ = IndirectDispatchArgs(_skTemp6, _skTemp7, _skTemp8).z;
  }
}
@compute @workgroup_size(16, 16, 1) fn main() {
  _skslMain();
}
