diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let inputVal: vec4<bool> = vec4<bool>(_globalUniforms.colorGreen.xxyz);
    let expected: vec4<bool> = vec4<bool>(_globalUniforms.colorGreen.xyyw);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((any(inputVal.xy) == expected.x) && (any(inputVal.xyz) == expected.y)) && (any(inputVal) == expected.z)) && (false == expected.x)) && expected.y) && expected.z));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
