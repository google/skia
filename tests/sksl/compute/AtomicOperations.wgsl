### Compilation failed:

error: :6:18 error: unresolved type 'atomicUint'
  globalCounter: atomicUint,
                 ^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct CSIn {
  @builtin(local_invocation_id) sk_LocalInvocationID: vec3<u32>,
};
struct ssbo {
  globalCounter: atomicUint,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : ssbo;
var<private> localCounter: atomicUint;
fn _skslMain(_stageIn: CSIn) {
  {
    if _stageIn.sk_LocalInvocationID.x == 0u {
      {
        atomicStore(localCounter, 0u);
      }
    }
    workgroupBarrier();
    let _skTemp1 = atomicAdd(localCounter, 1u);
    workgroupBarrier();
    if _stageIn.sk_LocalInvocationID.x == 0u {
      {
        let _skTemp2 = atomicLoad(localCounter);
        let _skTemp3 = atomicAdd(_storage0.globalCounter, _skTemp2);
      }
    }
  }
}
@compute @workgroup_size(64, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}

1 error
