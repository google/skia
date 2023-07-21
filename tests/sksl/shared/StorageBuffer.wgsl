### Compilation failed:

error: :17:20 error: unresolved identifier 'offset'
    let _skTemp0 = offset;
                   ^^^^^^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @location(2) @interpolate(flat) bufferIndex: i32,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct SomeData {
  a: vec4<f32>,
  b: vec2<f32>,
};
fn main(_stageIn: FSIn, _skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    let _skTemp0 = offset;
    let _skTemp1 = offset;
    outputData[_skTemp0] = inputData[_skTemp1];
    return vec4<f32>(inputData[_stageIn.bufferIndex].a * inputData[_stageIn.bufferIndex].b.x);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn, _stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
