diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: i32,
  b: u32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    let b1: i32 = firstLeadingBit(_globalUniforms.a) + i32(firstLeadingBit(_globalUniforms.b));
    let b2: vec2<i32> = firstLeadingBit(vec2<i32>(_globalUniforms.a)) + vec2<i32>(firstLeadingBit(vec2<u32>(_globalUniforms.b)));
    let b3: vec3<i32> = firstLeadingBit(vec3<i32>(_globalUniforms.a)) + vec3<i32>(firstLeadingBit(vec3<u32>(_globalUniforms.b)));
    let b4: vec4<i32> = firstLeadingBit(vec4<i32>(_globalUniforms.a)) + vec4<i32>(firstLeadingBit(vec4<u32>(_globalUniforms.b)));
    (*_stageOut).sk_FragColor = vec4<f32>(((vec4<i32>(b1) + b2.xyxy) + vec4<i32>(b3, 1)) + b4);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
