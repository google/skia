diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct Outputs {
  result: i32,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : Outputs;
fn _skslMain() {
  {
    _storage0.result = 1;
  }
}
@compute @workgroup_size(12, 34, 56) fn main() {
  _skslMain();
}
