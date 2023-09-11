diagnostic(off, derivative_uniformity);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
struct inputs {
  in_data: array<f32>,
};
@group(0) @binding(0) var<storage, read> _storage0 : inputs;
struct outputs {
  out_data: array<f32>,
};
@group(0) @binding(1) var<storage, read_write> _storage1 : outputs;
var<workgroup> shared_data: array<f32, 512>;
fn store_vIf(i: u32, value: f32) {
  {
    shared_data[i] = value;
  }
}
fn _skslMain(_stageIn: CSIn) {
  {
    var id: u32 = _stageIn.sk_GlobalInvocationID.x;
    var rd_id: u32;
    var wr_id: u32;
    var mask: u32;
    let _skTemp2 = id * 2u;
    let _skTemp3 = id * 2u;
    shared_data[_skTemp2] = _storage0.in_data[_skTemp3];
    let _skTemp4 = id * 2u + 1u;
    let _skTemp5 = id * 2u + 1u;
    shared_data[_skTemp4] = _storage0.in_data[_skTemp5];
    workgroupBarrier();
    const steps: u32 = 9u;
    {
      var _0_step: u32 = 0u;
      loop {
        {
          mask = (1u << _0_step) - 1u;
          rd_id = ((id >> _0_step) << (_0_step + 1u)) + mask;
          wr_id = (rd_id + 1u) + (id & mask);
          store_vIf(wr_id, shared_data[wr_id] + shared_data[rd_id]);
          workgroupBarrier();
        }
        continuing {
          _0_step = _0_step + u32(1);
          break if _0_step >= steps;
        }
      }
    }
    let _skTemp6 = id * 2u;
    let _skTemp7 = id * 2u;
    _storage1.out_data[_skTemp6] = shared_data[_skTemp7];
    let _skTemp8 = id * 2u + 1u;
    let _skTemp9 = id * 2u + 1u;
    _storage1.out_data[_skTemp8] = shared_data[_skTemp9];
  }
}
@compute @workgroup_size(256, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
