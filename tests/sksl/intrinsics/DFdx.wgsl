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
    const expected: vec4<f16> = vec4<f16>(0.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((f16(dpdx(f32(_globalUniforms.testInputs.x))) == expected.x) && all(vec2<f16>(dpdx(vec2<f32>(_globalUniforms.testInputs.xy))) == expected.xy)) && all(vec3<f16>(dpdx(vec3<f32>(_globalUniforms.testInputs.xyz))) == expected.xyz)) && all(vec4<f16>(dpdx(vec4<f32>(_globalUniforms.testInputs))) == expected)) && all(sign(dpdx(coords.xx)) == vec2<f32>(1.0))) && all(sign(dpdx(coords.yy)) == vec2<f32>(0.0))) && all(sign(dpdx(coords)) == vec2<f32>(1.0, 0.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
