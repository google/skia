### Compilation failed:

error: :13:20 error: unresolved call target 'packHalf2x16'
    let _skTemp0 = packHalf2x16(_globalUniforms.testInputs.xy);
                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
  testInputs: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = packHalf2x16(_globalUniforms.testInputs.xy);
    var xy: u32 = _skTemp0;
    let _skTemp1 = packHalf2x16(_globalUniforms.testInputs.zw);
    var zw: u32 = _skTemp1;
    let _skTemp2 = unpackHalf2x16(xy);
    let _skTemp3 = unpackHalf2x16(zw);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(_skTemp2 == vec2<f32>(-1.25, 0.0)) && all(_skTemp3 == vec2<f32>(0.75, 2.25))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
