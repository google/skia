### Compilation failed:

error: Tint compilation failed.

diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let valueIsNaN: vec4<f32> = 0.0 / _globalUniforms.testInputs.yyyy;
    let valueIsNumber: vec4<f32> = 1.0 / _globalUniforms.testInputs;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((isnan(valueIsNaN.x) && all(isnan(valueIsNaN.xy))) && all(isnan(valueIsNaN.xyz))) && all(isnan(valueIsNaN))) && (!isnan(valueIsNumber.x))) && (!any(isnan(valueIsNumber.xy)))) && (!any(isnan(valueIsNumber.xyz)))) && (!any(isnan(valueIsNumber)))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}

1 error
