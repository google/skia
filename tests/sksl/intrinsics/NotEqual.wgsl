diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: vec4<f32>,
  b: vec4<f32>,
  c: vec2<u32>,
  d: vec2<u32>,
  e: vec3<i32>,
  f: vec3<i32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var expectFFTT: vec4<bool> = vec4<bool>(false, false, true, true);
    var expectTTFF: vec4<bool> = vec4<bool>(true, true, false, false);
    (*_stageOut).sk_FragColor.x = f32(select(0, 1, (_globalUniforms.a != _globalUniforms.b).x));
    (*_stageOut).sk_FragColor.y = f32(select(0, 1, (_globalUniforms.c != _globalUniforms.d).y));
    (*_stageOut).sk_FragColor.z = f32(select(0, 1, (_globalUniforms.e != _globalUniforms.f).z));
    let _skTemp0 = any(expectTTFF);
    let _skTemp1 = any(expectFFTT);
    (*_stageOut).sk_FragColor.w = f32(select(0, 1, _skTemp0 || _skTemp1));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
