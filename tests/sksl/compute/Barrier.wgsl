diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
fn _skslMain() {
  {
    workgroupBarrier();
    storageBarrier();
  }
}
@compute @workgroup_size(64, 1, 1) fn main() {
  _skslMain();
}
