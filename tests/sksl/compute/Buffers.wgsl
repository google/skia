diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
struct StorageBuffer {
  storage_data: array<_skArrayElement_f, 16>,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : StorageBuffer;
struct UniformBuffer {
  uniform_data: array<_skArrayElement_f, 16>,
};
@group(0) @binding(1) var<uniform> _uniform1 : UniformBuffer;
var<workgroup> scratch: array<f32, 16>;
fn _skslMain(_stageIn: CSIn) {
  {
    let id: u32 = _stageIn.sk_GlobalInvocationID.x;
    scratch[id] = _skUnpacked__uniform1_uniform_data[id];
    workgroupBarrier();
    _skUnpacked__storage0_storage_data[id] = scratch[id];
  }
}
@compute @workgroup_size(16, 1, 1) fn main(_stageIn: CSIn) {
  _skInitializePolyfilledUniforms();
  _skslMain(_stageIn);
}
struct _skArrayElement_f {
  @align(16) e : f32
};
var<private> _skUnpacked__storage0_storage_data: array<f32, 16>;
var<private> _skUnpacked__uniform1_uniform_data: array<f32, 16>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__storage0_storage_data = array<f32, 16>(_storage0.storage_data[0].e, _storage0.storage_data[1].e, _storage0.storage_data[2].e, _storage0.storage_data[3].e, _storage0.storage_data[4].e, _storage0.storage_data[5].e, _storage0.storage_data[6].e, _storage0.storage_data[7].e, _storage0.storage_data[8].e, _storage0.storage_data[9].e, _storage0.storage_data[10].e, _storage0.storage_data[11].e, _storage0.storage_data[12].e, _storage0.storage_data[13].e, _storage0.storage_data[14].e, _storage0.storage_data[15].e);
  _skUnpacked__uniform1_uniform_data = array<f32, 16>(_uniform1.uniform_data[0].e, _uniform1.uniform_data[1].e, _uniform1.uniform_data[2].e, _uniform1.uniform_data[3].e, _uniform1.uniform_data[4].e, _uniform1.uniform_data[5].e, _uniform1.uniform_data[6].e, _uniform1.uniform_data[7].e, _uniform1.uniform_data[8].e, _uniform1.uniform_data[9].e, _uniform1.uniform_data[10].e, _uniform1.uniform_data[11].e, _uniform1.uniform_data[12].e, _uniform1.uniform_data[13].e, _uniform1.uniform_data[14].e, _uniform1.uniform_data[15].e);
}
