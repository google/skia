diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  pos1: vec4<f32>,
  pos2: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const expected: vec4<f32> = vec4<f32>(3.0, 3.0, 5.0, 13.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((distance(_globalUniforms.pos1.x, _globalUniforms.pos2.x) == expected.x) && (distance(_globalUniforms.pos1.xy, _globalUniforms.pos2.xy) == expected.y)) && (distance(_globalUniforms.pos1.xyz, _globalUniforms.pos2.xyz) == expected.z)) && (distance(_globalUniforms.pos1, _globalUniforms.pos2) == expected.w)) && (3.0 == expected.x)) && (3.0 == expected.y)) && (5.0 == expected.z)) && (13.0 == expected.w)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
