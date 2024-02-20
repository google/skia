diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
  @builtin(local_invocation_id) sk_LocalInvocationID: vec3<u32>,
  @builtin(local_invocation_index) sk_LocalInvocationIndex: u32,
  @builtin(num_workgroups) sk_NumWorkgroups: vec3<u32>,
  @builtin(workgroup_id) sk_WorkgroupID: vec3<u32>,
};
struct outputs {
  outputBuffer: array<u32>,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : outputs;
fn helper_I(_stageIn: CSIn) -> u32 {
  {
    return ((_stageIn.sk_NumWorkgroups.x + _stageIn.sk_WorkgroupID.x) + _stageIn.sk_LocalInvocationID.x) + _stageIn.sk_GlobalInvocationID.x;
  }
}
fn _skslMain(_stageIn: CSIn) {
  {
    let _skTemp1 = helper_I(_stageIn);
    _storage0.outputBuffer[_stageIn.sk_LocalInvocationIndex] = _skTemp1;
  }
}
@compute @workgroup_size(64, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
