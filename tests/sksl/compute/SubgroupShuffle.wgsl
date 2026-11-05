diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
enable subgroups;
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
  @builtin(subgroup_invocation_id) sk_SubgroupInvocationID: u32,
};
struct inputs {
  inputBuffer: array<f32>,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : inputs;
struct outputs {
  floatOut: array<vec4<f32>>,
};
@group(0) @binding(1) var<storage, read_write> _storage1 : outputs;
struct intOutputs {
  intOut: array<vec4<i32>>,
};
@group(0) @binding(2) var<storage, read_write> _storage2 : intOutputs;
struct uintOutputs {
  uintOut: array<vec4<u32>>,
};
@group(0) @binding(3) var<storage, read_write> _storage3 : uintOutputs;
fn _skslMain(_stageIn: CSIn) {
  {
    let idx: u32 = _stageIn.sk_GlobalInvocationID.x;
    let f: f32 = _storage0.inputBuffer[idx];
    let i: i32 = i32(f);
    let u: u32 = idx;
    let neighbor: u32 = _stageIn.sk_SubgroupInvocationID ^ 1u;
    const kLane: u32 = 5u;
    _storage1.floatOut[idx] = vec4<f32>(subgroupShuffle(f, neighbor), subgroupShuffle(f, kLane), 0.0, 0.0) + vec4<f32>(subgroupShuffle(vec4<f16>(f16(f)), neighbor));
    _storage2.intOut[idx] = vec4<i32>(subgroupShuffle(i, neighbor), 0, 0, 0) + subgroupShuffle(vec4<i32>(i, -i, i, -i), u32(i) & 7u);
    _storage3.uintOut[idx] = vec4<u32>(subgroupShuffle(u, 63u - _stageIn.sk_SubgroupInvocationID), 0u, 0u, 0u) + subgroupShuffle(vec4<u32>(u), 0u);
  }
}
@compute @workgroup_size(64, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
