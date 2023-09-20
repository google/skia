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
    let _skTemp0 = pack2x16snorm(_globalUniforms.testInputs.xy);
    var xy: u32 = _skTemp0;
    let _skTemp1 = pack2x16snorm(_globalUniforms.testInputs.zw);
    var zw: u32 = _skTemp1;
    const tolerance: vec2<f32> = vec2<f32>(0.015625);
    let _skTemp2 = unpack2x16snorm(xy);
    let _skTemp3 = abs(_skTemp2 - vec2<f32>(-1.0, 0.0));
    let _skTemp4 = all((_skTemp3 < tolerance));
    let _skTemp5 = unpack2x16snorm(zw);
    let _skTemp6 = abs(_skTemp5 - vec2<f32>(0.75, 1.0));
    let _skTemp7 = all((_skTemp6 < tolerance));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_skTemp4 && _skTemp7));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
