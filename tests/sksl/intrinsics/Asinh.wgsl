diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  inputVal: vec4<f16>,
  expected: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((asinh(_globalUniforms.inputVal.x) == _globalUniforms.expected.x) && all(asinh(_globalUniforms.inputVal.xy) == _globalUniforms.expected.xy)) && all(asinh(_globalUniforms.inputVal.xyz) == _globalUniforms.expected.xyz)) && all(asinh(_globalUniforms.inputVal) == _globalUniforms.expected)) && (0.0h == _globalUniforms.expected.x)) && all(vec2<f16>(0.0h) == _globalUniforms.expected.xy)) && all(vec3<f16>(0.0h, 0.0h, 1.0h) == _globalUniforms.expected.xyz)) && all(vec4<f16>(0.0h, 0.0h, 1.0h, -1.0h) == _globalUniforms.expected)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
