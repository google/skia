diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
struct constants {
  x: i32,
};
@group(0) @binding(0) var<uniform> _uniform0 : constants;
struct outputBuffer {
  results: array<i32>,
};
@group(0) @binding(1) var<storage, read_write> _storage1 : outputBuffer;
fn _skslMain(_stageIn: CSIn) {
  {
    let _skTemp2 = _stageIn.sk_GlobalInvocationID.x;
    _storage1.results[_skTemp2] = _storage1.results[_skTemp2] * _uniform0.x;
  }
}
@compute @workgroup_size(64, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
