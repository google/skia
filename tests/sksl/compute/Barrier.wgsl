diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
fn _skslMain() {
  {
    workgroupBarrier();
    storageBarrier();
  }
}
@compute @workgroup_size(64, 1, 1) fn main() {
  _skslMain();
}
