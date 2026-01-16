diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  inputVal: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const expectedVec: vec4<f32> = vec4<f32>(1.0, 0.0, 0.0, 0.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((sign(_globalUniforms.inputVal.x) == expectedVec.x) && all(normalize(_globalUniforms.inputVal.xy) == expectedVec.xy)) && all(normalize(_globalUniforms.inputVal.xyz) == expectedVec.xyz)) && all(normalize(_globalUniforms.inputVal) == expectedVec)) && (1.0 == expectedVec.x)) && all(vec2<f32>(0.0, 1.0) == expectedVec.yx)) && all(vec3<f32>(0.0, 1.0, 0.0) == expectedVec.zxy)) && all(vec4<f32>(1.0, 0.0, 0.0, 0.0) == expectedVec)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
