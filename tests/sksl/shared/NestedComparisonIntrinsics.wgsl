diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  colorGreen: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let result: vec4<bool> = (_globalUniforms.colorRed < vec4<f16>(2.0h)) == (vec4<f16>(3.0h) > _globalUniforms.colorGreen);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(result)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
