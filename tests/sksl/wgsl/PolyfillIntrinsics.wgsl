diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  N: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = _globalUniforms.N.x;
    let _skTemp1 = 1e+30;
    var huge: f32 = select(-_skTemp0, _skTemp0, _skTemp1 * 1e+30 < 0);
    let _skTemp2 = 1e+30;
    huge = (1e+30 - 2 * _skTemp2 * 1e+30 * _skTemp2);
    let _skTemp3 = 0.0;
    huge = (_skTemp3 - 0.0 * floor(_skTemp3 / 0.0));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(huge > 0.0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
