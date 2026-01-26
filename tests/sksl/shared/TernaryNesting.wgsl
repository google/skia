diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorWhite: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let colorBlue: vec4<f16> = vec4<f16>(0.0h, 0.0h, _globalUniforms.colorWhite.zw);
    let colorGreen: vec4<f16> = vec4<f16>(0.0h, _globalUniforms.colorWhite.y, 0.0h, _globalUniforms.colorWhite.w);
    let colorRed: vec4<f16> = vec4<f16>(_globalUniforms.colorWhite.x, 0.0h, 0.0h, _globalUniforms.colorWhite.w);
    var _skTemp0: vec4<f16>;
    if any(_globalUniforms.colorWhite != colorBlue) {
      _skTemp0 = select(colorGreen, colorRed, vec4<bool>(all(colorGreen == colorRed)));
    } else {
      _skTemp0 = select(_globalUniforms.colorWhite, colorBlue, vec4<bool>(any(colorRed != colorGreen)));
    }
    let result: vec4<f16> = _skTemp0;
    var _skTemp1: vec4<f16>;
    if all(colorRed == colorBlue) {
      _skTemp1 = _globalUniforms.colorWhite;
    } else {
      var _skTemp2: vec4<f16>;
      if any(colorRed != colorGreen) {
        _skTemp2 = result;
      } else {
        _skTemp2 = select(colorRed, colorBlue, vec4<bool>(all(colorRed == _globalUniforms.colorWhite)));
      }
      _skTemp1 = _skTemp2;
    }
    return _skTemp1;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
