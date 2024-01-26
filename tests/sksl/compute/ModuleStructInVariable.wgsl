### Compilation failed:

error: :7:15 error: unresolved type 'IndirectDispatchArgs'
    var args: IndirectDispatchArgs = IndirectDispatchArgs(1, 2, 3);
              ^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
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

1 error
