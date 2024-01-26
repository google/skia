diagnostic(off, derivative_uniformity);
struct IndirectDispatchArgs {
  x: i32,
  y: i32,
  z: i32,
};
var<workgroup> outX: i32;
var<workgroup> outY: i32;
var<workgroup> outZ: i32;
fn _skslMain() {
  {
    var args: IndirectDispatchArgs = IndirectDispatchArgs(1, 2, 3);
    outX = args.x;
    outY = args.y;
    outZ = args.z;
  }
}
@compute @workgroup_size(16, 16, 1) fn main() {
  _skslMain();
}
