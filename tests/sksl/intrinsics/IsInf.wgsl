### Compilation failed:

error: Tint compilation failed.

diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  testMatrix2x2: _skMatrix22h,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let infiniteValue: vec4<f16> = vec4<f16>(_skUnpacked__globalUniforms_testMatrix2x2[0], _skUnpacked__globalUniforms_testMatrix2x2[1]) / _globalUniforms.colorGreen.x;
    let finiteValue: vec4<f16> = vec4<f16>(_skUnpacked__globalUniforms_testMatrix2x2[0], _skUnpacked__globalUniforms_testMatrix2x2[1]) / _globalUniforms.colorGreen.y;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((isinf(infiniteValue.x) && all(isinf(infiniteValue.xy))) && all(isinf(infiniteValue.xyz))) && all(isinf(infiniteValue))) && (!isinf(finiteValue.x))) && (!any(isinf(finiteValue.xy)))) && (!any(isinf(finiteValue.xyz)))) && (!any(isinf(finiteValue)))));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
struct _skRow2h {
  @align(16) r : vec2<f16>
};
struct _skMatrix22h {
  c : array<_skRow2h, 2>
};
var<private> _skUnpacked__globalUniforms_testMatrix2x2: mat2x2<f16>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_testMatrix2x2 = mat2x2<f16>(_globalUniforms.testMatrix2x2.c[0].r, _globalUniforms.testMatrix2x2.c[1].r);
}

1 error
