diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
enable subgroups;
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
  @builtin(subgroup_id) sk_SubgroupID: u32,
  @builtin(subgroup_invocation_id) sk_SubgroupInvocationID: u32,
  @builtin(subgroup_size) sk_SubgroupSize: u32,
};
struct outputs {
  outputBuffer: array<u32>,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : outputs;
fn helper_I(_stageIn: CSIn) -> u32 {
  {
    return _stageIn.sk_SubgroupSize + _stageIn.sk_SubgroupID;
  }
}
fn _skslMain(_stageIn: CSIn) {
  {
    let _skTemp1 = _stageIn.sk_GlobalInvocationID.x;
    _storage0.outputBuffer[_skTemp1] = helper_I(_stageIn) + _stageIn.sk_SubgroupInvocationID;
  }
}
@compute @workgroup_size(64, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
