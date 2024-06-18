### Compilation failed:

error: Tint compilation failed.

diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testArray: array<f32, 5>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var one: f32 = _globalUniforms.testArray[0];
    var two: f32 = _globalUniforms.testArray[1];
    var three: f32 = _globalUniforms.testArray[2];
    var four: f32 = f32(_globalUniforms.testArray[3]);
    var five: f32 = f32(_globalUniforms.testArray[4]);
    let _skTemp0 = fma(one, two, three);
    let _skTemp1 = fma(f32(three), four, five);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((_skTemp0 == 5.0) && (_skTemp1 == 17.0)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
