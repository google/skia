diagnostic(off, derivative_uniformity);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
struct sizeBuffer {
  sizes: array<vec2<i32>>,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : sizeBuffer;
struct inputs1 {
  data1: array<f32>,
};
@group(0) @binding(1) var<storage, read> _storage1 : inputs1;
struct inputs2 {
  data2: array<f32>,
};
@group(0) @binding(2) var<storage, read> _storage2 : inputs2;
struct result {
  resultData: array<f32>,
};
@group(0) @binding(3) var<storage, read_write> _storage3 : result;
fn _skslMain(_stageIn: CSIn) {
  {
    _storage0.sizes[2] = vec2<i32>(_storage0.sizes[0].x, _storage0.sizes[1].y);
    var resultCell: vec2<i32> = vec2<i32>(i32(_stageIn.sk_GlobalInvocationID.x), i32(_stageIn.sk_GlobalInvocationID.y));
    var result: f32 = 0.0;
    {
      var i: i32 = 0;
      loop {
        if i < _storage0.sizes[0].y {
          {
            var a: i32 = i + resultCell.x * _storage0.sizes[0].y;
            var b: i32 = resultCell.y + i * _storage0.sizes[1].y;
            result = result + _storage1.data1[a] * _storage2.data2[b];
          }
        } else {
          break;
        }
        continuing {
          i = i + i32(1);
        }
      }
    }
    var index: i32 = resultCell.y + resultCell.x * _storage0.sizes[1].y;
    _storage3.resultData[index] = result;
  }
}
@compute @workgroup_size(16, 16, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
