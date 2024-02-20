diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
    var args: array<IndirectDispatchArgs, 3>;
    outX = args[0].x;
    outY = args[1].y;
    outZ = args[2].z;
  }
}
@compute @workgroup_size(16, 16, 1) fn main() {
  _skslMain();
}
