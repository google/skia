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
    const expected: vec4<f32> = vec4<f32>(0.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((dpdy(_globalUniforms.testInputs.x) == expected.x) && all(dpdy(_globalUniforms.testInputs.xy) == expected.xy)) && all(dpdy(_globalUniforms.testInputs.xyz) == expected.xyz)) && all(dpdy(_globalUniforms.testInputs) == expected)) && all(sign(dpdy(coords.xx)) == vec2<f32>(0.0))) && all(sign(dpdy(coords.yy)) == vec2<f32>(1.0))) && all(sign(dpdy(coords)) == vec2<f32>(0.0, 1.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
