diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
enable subgroups;
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
struct inputs {
  inputBuffer: array<f32>,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : inputs;
struct outputs {
  outputBuffer: array<vec4<u32>>,
};
@group(0) @binding(1) var<storage, read_write> _storage1 : outputs;
const kLane: u32 = 3u;
fn _skslMain(_stageIn: CSIn) {
  {
    let idx: u32 = _stageIn.sk_GlobalInvocationID.x;
    let f: f32 = _storage0.inputBuffer[idx];
    let positive: bool = f > 0.0;
    var result: vec4<u32> = subgroupBallot(positive);
    result.x = result.x + u32(subgroupAll(positive));
    result.y = result.y + u32(subgroupAny(positive));
    result.z = result.z + u32(subgroupBroadcast(f, 0u));
    result.w = result.w + u32(subgroupBroadcast(f16(f), 3u));
    result = vec4<u32>((result.xy + vec2<u32>(subgroupBroadcast(vec2<i32>(i32(idx)), 2u))), result.zw);
    result = result + subgroupBroadcast(vec4<u32>(idx), 127u);
    result.z = result.z + u32(subgroupBroadcastFirst(f));
    result.w = result.w + u32(subgroupBroadcastFirst(vec3<f16>(f16(f))).x);
    result = vec4<u32>((result.xy + vec2<u32>(subgroupBroadcastFirst(vec2<i32>(i32(idx))))), result.zw);
    result = result + subgroupBroadcastFirst(vec4<u32>(idx));
    _storage1.outputBuffer[idx] = result;
  }
}
@compute @workgroup_size(64, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
