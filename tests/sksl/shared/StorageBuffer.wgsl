diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSIn {
  @location(2) @interpolate(flat) bufferIndex: i32,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct storageBuffer {
  offset: u32,
  inputData: array<SomeData>,
};
@group(0) @binding(0) var<storage, read> _storage0 : storageBuffer;
struct outputBuffer {
  outputData: array<SomeData>,
};
@group(0) @binding(1) var<storage, read_write> _storage1 : outputBuffer;
struct SomeData {
  a: vec4<f32>,
  b: vec2<f32>,
};
fn _skslMain(_stageIn: FSIn, coords: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp2 = _storage0.offset;
    let _skTemp3 = _storage0.offset;
    _storage1.outputData[_skTemp2] = _storage0.inputData[_skTemp3];
    return vec4<f32>(_storage0.inputData[_stageIn.bufferIndex].a * _storage0.inputData[_stageIn.bufferIndex].b.x);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn, /*fragcoord*/ vec2<f32>());
  return _stageOut;
}
