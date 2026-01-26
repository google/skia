diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
  testMatrix2x2: _skMatrix22f,
  testMatrix3x3: _skMatrix33h,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const smallM22: mat2x2<f16> = mat2x2<f16>(1000.0h, 1000.0h, 1000.0h, 1000.0h);
    let _skTemp0 = smallM22;
    var h22: mat2x2<f16> = mat2x2<f16>(_skTemp0[0] * smallM22[0], _skTemp0[1] * smallM22[1]);
    const hugeM22: mat2x2<f32> = mat2x2<f32>(1e+30, 1e+30, 1e+30, 1e+30);
    let _skTemp1 = hugeM22;
    h22 = mat2x2<f16>(mat2x2<f32>(_skTemp1[0] * hugeM22[0], _skTemp1[1] * hugeM22[1]));
    h22 = mat2x2<f16>(0.0h, 5.0h, 10.0h, 15.0h);
    const _skTemp2 = mat2x2<f32>(1.0, 0.0, 0.0, 1.0);
    let f22: mat2x2<f32> = mat2x2<f32>(_skUnpacked__globalUniforms_testMatrix2x2[0] * _skTemp2[0], _skUnpacked__globalUniforms_testMatrix2x2[1] * _skTemp2[1]);
    const _skTemp3 = mat3x3<f16>(2.0h, 2.0h, 2.0h, 2.0h, 2.0h, 2.0h, 2.0h, 2.0h, 2.0h);
    let h33: mat3x3<f16> = mat3x3<f16>(_skUnpacked__globalUniforms_testMatrix3x3[0] * _skTemp3[0], _skUnpacked__globalUniforms_testMatrix3x3[1] * _skTemp3[1], _skUnpacked__globalUniforms_testMatrix3x3[2] * _skTemp3[2]);
    const _skTemp4 = mat2x2<f16>(0.0h, 5.0h, 10.0h, 15.0h);
    const _skTemp5 = mat2x2<f32>(1.0, 0.0, 0.0, 4.0);
    const _skTemp6 = mat3x3<f16>(2.0h, 4.0h, 6.0h, 8.0h, 10.0h, 12.0h, 14.0h, 16.0h, 18.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((all(h22[0] == _skTemp4[0]) && all(h22[1] == _skTemp4[1])) && (all(f22[0] == _skTemp5[0]) && all(f22[1] == _skTemp5[1]))) && (all(h33[0] == _skTemp6[0]) && all(h33[1] == _skTemp6[1]) && all(h33[2] == _skTemp6[2]))));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
struct _skRow2f {
  @align(16) r : vec2<f32>
};
struct _skMatrix22f {
  c : array<_skRow2f, 2>
};
var<private> _skUnpacked__globalUniforms_testMatrix2x2: mat2x2<f32>;
struct _skRow3h {
  @align(16) r : vec3<f16>
};
struct _skMatrix33h {
  c : array<_skRow3h, 3>
};
var<private> _skUnpacked__globalUniforms_testMatrix3x3: mat3x3<f16>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_testMatrix2x2 = mat2x2<f32>(_globalUniforms.testMatrix2x2.c[0].r, _globalUniforms.testMatrix2x2.c[1].r);
  _skUnpacked__globalUniforms_testMatrix3x3 = mat3x3<f16>(_globalUniforms.testMatrix3x3.c[0].r, _globalUniforms.testMatrix3x3.c[1].r, _globalUniforms.testMatrix3x3.c[2].r);
}
