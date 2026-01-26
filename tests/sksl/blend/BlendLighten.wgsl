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
    var _0_result: vec4<f16> = _globalUniforms.src + (1.0h - _globalUniforms.src.w) * _globalUniforms.dst;
    _0_result = vec4<f16>((max(_0_result.xyz, (1.0h - _globalUniforms.dst.w) * _globalUniforms.src.xyz + _globalUniforms.dst.xyz)), _0_result.w);
    (*_stageOut).sk_FragColor = _0_result;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
