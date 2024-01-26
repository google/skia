### Compilation failed:

error: :7:21 error: unresolved type 'IndirectDispatchArgs'
    var args: array<IndirectDispatchArgs, 3>;
                    ^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
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

1 error
