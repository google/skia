diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  f: _skMatrix22f,
  h: _skMatrix22h,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = (vec4<f16>(_skUnpacked__globalUniforms_h[0], _skUnpacked__globalUniforms_h[1]) + vec4<f16>(vec4<f32>(_skUnpacked__globalUniforms_f[0], _skUnpacked__globalUniforms_f[1]))) + vec4<f16>(vec4<f32>(vec4<f16>(_skUnpacked__globalUniforms_h[0], _skUnpacked__globalUniforms_h[1])) + vec4<f32>(_skUnpacked__globalUniforms_f[0], _skUnpacked__globalUniforms_f[1]));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
struct _skRow2f {
  @align(16) r : vec2<f32>
};
struct _skMatrix22f {
  c : array<_skRow2f, 2>
};
var<private> _skUnpacked__globalUniforms_f: mat2x2<f32>;
struct _skRow2h {
  @align(16) r : vec2<f16>
};
struct _skMatrix22h {
  c : array<_skRow2h, 2>
};
var<private> _skUnpacked__globalUniforms_h: mat2x2<f16>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_f = mat2x2<f32>(_globalUniforms.f.c[0].r, _globalUniforms.f.c[1].r);
  _skUnpacked__globalUniforms_h = mat2x2<f16>(_globalUniforms.h.c[0].r, _globalUniforms.h.c[1].r);
}
