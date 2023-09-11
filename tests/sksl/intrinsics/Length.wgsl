diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var inputVal: vec4<f32> = _globalUniforms.testMatrix2x2 + vec4<f32>(2.0, -2.0, 1.0, 8.0);
    var expected: vec4<f32> = vec4<f32>(3.0, 3.0, 5.0, 13.0);
    const allowedDelta: f32 = 0.05;
    let _skTemp0 = length(inputVal.x);
    let _skTemp1 = abs(_skTemp0 - expected.x);
    let _skTemp2 = length(inputVal.xy);
    let _skTemp3 = abs(_skTemp2 - expected.y);
    let _skTemp4 = length(inputVal.xyz);
    let _skTemp5 = abs(_skTemp4 - expected.z);
    let _skTemp6 = length(inputVal);
    let _skTemp7 = abs(_skTemp6 - expected.w);
    let _skTemp8 = abs(3.0 - expected.x);
    let _skTemp9 = abs(3.0 - expected.y);
    let _skTemp10 = abs(5.0 - expected.z);
    let _skTemp11 = abs(13.0 - expected.w);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((_skTemp1 < allowedDelta) && (_skTemp3 < allowedDelta)) && (_skTemp5 < allowedDelta)) && (_skTemp7 < allowedDelta)) && (_skTemp8 < allowedDelta)) && (_skTemp9 < allowedDelta)) && (_skTemp10 < allowedDelta)) && (_skTemp11 < allowedDelta)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
