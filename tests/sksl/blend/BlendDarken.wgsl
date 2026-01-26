diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  src: vec4<f16>,
  dst: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    var _0_a: vec4<f16> = _globalUniforms.src + (1.0h - _globalUniforms.src.w) * _globalUniforms.dst;
    let _1_b: vec3<f16> = (1.0h - _globalUniforms.dst.w) * _globalUniforms.src.xyz + _globalUniforms.dst.xyz;
    _0_a = vec4<f16>((min(_0_a.xyz, _1_b)), _0_a.w);
    (*_stageOut).sk_FragColor = _0_a;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
