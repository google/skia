diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn this_function_is_defined_before_use_h4h4(x: vec4<f32>) -> vec4<f32> {
  {
    return -x;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = this_function_is_defined_before_use_h4h4(-_globalUniforms.colorGreen);
    return _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
