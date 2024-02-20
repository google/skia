diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct InputBuffer {
  args: IndirectDispatchArgs,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : InputBuffer;
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
    outX = _storage0.args.x;
    outY = _storage0.args.y;
    outZ = _storage0.args.z;
  }
}
@compute @workgroup_size(16, 16, 1) fn main() {
  _skslMain();
}
