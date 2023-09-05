diagnostic(off, derivative_uniformity);
fn _skslMain() {
  {
    workgroupBarrier();
    storageBarrier();
  }
}
@compute @workgroup_size(64, 1, 1) fn main() {
  _skslMain();
}
