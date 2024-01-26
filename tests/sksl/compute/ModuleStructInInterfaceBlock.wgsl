### Compilation failed:

error: :3:9 error: unresolved type 'IndirectDispatchArgs'
  args: IndirectDispatchArgs,
        ^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct InputBuffer {
  args: IndirectDispatchArgs,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : InputBuffer;
var<workgroup> outX: i32;
var<workgroup> outY: i32;
var<workgroup> outZ: i32;
fn _skslMain() {
  {
    outX = _storage0.args.x;
    outY = _storage0.args.y;
    outZ = _storage0.args.z;
  }
}
@compute @workgroup_size(16, 16, 1) fn main() {
  _skslMain();
}

1 error
