### Compilation failed:

error: :15:20 error: unresolved identifier 'offset'
    let _skTemp0 = offset;
                   ^^^^^^


struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct SomeData {
  a: vec4<f32>,
  b: vec2<f32>,
};
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    let _skTemp0 = offset;
    let _skTemp1 = offset;
    outputData[_skTemp0] = inputData[_skTemp1];
    let _skTemp2 = offset;
    let _skTemp3 = offset;
    return vec4<f32>(inputData[_skTemp2].a * inputData[_skTemp3].b.x);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}

1 error
