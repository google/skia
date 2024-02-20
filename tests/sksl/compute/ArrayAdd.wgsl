diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
struct inputBlock {
  offset: u32,
  src: array<i32>,
};
@group(0) @binding(0) var<storage, read> _storage0 : inputBlock;
struct outputBlock {
  dest: array<i32>,
};
@group(0) @binding(1) var<storage, read_write> _storage1 : outputBlock;
fn _skslMain(_stageIn: CSIn) {
  {
    let _skTemp2 = _stageIn.sk_GlobalInvocationID.x;
    let _skTemp3 = _stageIn.sk_GlobalInvocationID.x;
    let _skTemp4 = _stageIn.sk_GlobalInvocationID.x + _storage0.offset;
    _storage1.dest[_skTemp2] = _storage0.src[_skTemp3] + _storage0.src[_skTemp4];
  }
}
@compute @workgroup_size(256, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
