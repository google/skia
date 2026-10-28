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
    _storage1.floatOut[idx] = vec4<f32>(subgroupAdd(f), subgroupMin(f), subgroupMax(f), 0.0) + vec4<f32>(subgroupAdd(vec4<f16>(f16(f))));
    _storage2.intOut[idx] = vec4<i32>(subgroupAdd(i), subgroupMin(i), subgroupMax(i), 0) + subgroupMax(vec4<i32>(i, -i, i, -i));
    _storage3.uintOut[idx] = vec4<u32>(subgroupAdd(u), subgroupMin(u), subgroupMax(u), 0u) + subgroupMin(vec4<u32>(u));
    _storage1.floatOut[idx] = vec4<f32>((_storage1.floatOut[idx].xy + vec2<f32>(subgroupInclusiveAdd(f), subgroupExclusiveAdd(f))), _storage1.floatOut[idx].zw);
    _storage1.floatOut[idx] = vec4<f32>(_storage1.floatOut[idx].xy, (_storage1.floatOut[idx].zw + vec2<f32>(subgroupInclusiveAdd(vec2<f16>(f16(f))))));
    _storage2.intOut[idx] = vec4<i32>((_storage2.intOut[idx].xy + vec2<i32>(subgroupInclusiveAdd(i), subgroupExclusiveAdd(i))), _storage2.intOut[idx].zw);
    _storage3.uintOut[idx] = _storage3.uintOut[idx] + subgroupExclusiveAdd(vec4<u32>(u));
  }
}
@compute @workgroup_size(64, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
