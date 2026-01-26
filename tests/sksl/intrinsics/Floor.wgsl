diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  testInputs: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const expected: vec4<f16> = vec4<f16>(-2.0h, 0.0h, 0.0h, 2.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((floor(_globalUniforms.testInputs.x) == expected.x) && all(floor(_globalUniforms.testInputs.xy) == expected.xy)) && all(floor(_globalUniforms.testInputs.xyz) == expected.xyz)) && all(floor(_globalUniforms.testInputs) == expected)) && (-2.0h == expected.x)) && all(vec2<f16>(-2.0h, 0.0h) == expected.xy)) && all(vec3<f16>(-2.0h, 0.0h, 0.0h) == expected.xyz)) && all(vec4<f16>(-2.0h, 0.0h, 0.0h, 2.0h) == expected)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
