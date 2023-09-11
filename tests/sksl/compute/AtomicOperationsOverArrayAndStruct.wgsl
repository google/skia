diagnostic(off, derivative_uniformity);
struct CSIn {
  @builtin(local_invocation_id) sk_LocalInvocationID: vec3<u32>,
};
struct ssbo {
  globalCounts: GlobalCounts,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : ssbo;
struct GlobalCounts {
  firstHalfCount: atomic<u32>,
  secondHalfCount: atomic<u32>,
};
var<workgroup> localCounts: array<atomic<u32>, 2>;
fn _skslMain(_stageIn: CSIn) {
  {
    if _stageIn.sk_LocalInvocationID.x == 0u {
      {
        atomicStore(&localCounts[0], 0u);
        atomicStore(&localCounts[1], 0u);
      }
    }
    workgroupBarrier();
    var idx: u32 = u32(select(1, 0, _stageIn.sk_LocalInvocationID.x < 128u));
    let _skTemp1 = atomicAdd(&localCounts[idx], 1u);
    workgroupBarrier();
    if _stageIn.sk_LocalInvocationID.x == 0u {
      {
        let _skTemp2 = atomicLoad(&localCounts[0]);
        let _skTemp3 = atomicAdd(&_storage0.globalCounts.firstHalfCount, _skTemp2);
        let _skTemp4 = atomicLoad(&localCounts[1]);
        let _skTemp5 = atomicAdd(&_storage0.globalCounts.secondHalfCount, _skTemp4);
      }
    }
  }
}
@compute @workgroup_size(256, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
