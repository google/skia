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
    let one: f32 = _globalUniforms.testArray[0];
    let two: f32 = _globalUniforms.testArray[1];
    let three: f32 = _globalUniforms.testArray[2];
    let four: f32 = f32(_globalUniforms.testArray[3]);
    let five: f32 = f32(_globalUniforms.testArray[4]);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((fma(one, two, three) == 5.0) && (fma(f32(three), four, five) == 17.0)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
