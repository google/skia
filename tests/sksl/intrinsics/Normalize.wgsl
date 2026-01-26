diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  inputVal: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const expectedVec: vec4<f16> = vec4<f16>(1.0h, 0.0h, 0.0h, 0.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((sign(_globalUniforms.inputVal.x) == expectedVec.x) && all(normalize(_globalUniforms.inputVal.xy) == expectedVec.xy)) && all(normalize(_globalUniforms.inputVal.xyz) == expectedVec.xyz)) && all(normalize(_globalUniforms.inputVal) == expectedVec)) && (1.0h == expectedVec.x)) && all(vec2<f16>(0.0h, 1.0h) == expectedVec.yx)) && all(vec3<f16>(0.0h, 1.0h, 0.0h) == expectedVec.zxy)) && all(vec4<f16>(1.0h, 0.0h, 0.0h, 0.0h) == expectedVec)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
