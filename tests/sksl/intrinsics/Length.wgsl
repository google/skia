diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let inputVal: vec4<f32> = _globalUniforms.testMatrix2x2 + vec4<f32>(2.0, -2.0, 1.0, 8.0);
    const expected: vec4<f32> = vec4<f32>(3.0, 3.0, 5.0, 13.0);
    const allowedDelta: f32 = 0.05;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((abs(length(inputVal.x) - expected.x) < allowedDelta) && (abs(length(inputVal.xy) - expected.y) < allowedDelta)) && (abs(length(inputVal.xyz) - expected.z) < allowedDelta)) && (abs(length(inputVal) - expected.w) < allowedDelta)) && (abs(3.0 - expected.x) < allowedDelta)) && (abs(3.0 - expected.y) < allowedDelta)) && (abs(5.0 - expected.z) < allowedDelta)) && (abs(13.0 - expected.w) < allowedDelta)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
