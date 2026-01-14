diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testMatrix2x2: _skMatrix22,
  testMatrix3x3: mat3x3<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const testMatrix2x3: mat2x3<f32> = mat2x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    let _skTemp0 = transpose(_skUnpacked__globalUniforms_testMatrix2x2);
    const _skTemp1 = mat2x2<f32>(1.0, 3.0, 2.0, 4.0);
    let _skTemp2 = transpose(testMatrix2x3);
    const _skTemp3 = mat3x2<f32>(1.0, 4.0, 2.0, 5.0, 3.0, 6.0);
    let _skTemp4 = transpose(_globalUniforms.testMatrix3x3);
    const _skTemp5 = mat3x3<f32>(1.0, 4.0, 7.0, 2.0, 5.0, 8.0, 3.0, 6.0, 9.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((all(_skTemp0[0] == _skTemp1[0]) && all(_skTemp0[1] == _skTemp1[1])) && (all(_skTemp2[0] == _skTemp3[0]) && all(_skTemp2[1] == _skTemp3[1]) && all(_skTemp2[2] == _skTemp3[2]))) && (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1]) && all(_skTemp4[2] == _skTemp5[2]))));
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
struct _skRow2 {
  @align(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked__globalUniforms_testMatrix2x2: mat2x2<f32>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_testMatrix2x2 = mat2x2<f32>(_globalUniforms.testMatrix2x2.c[0].r, _globalUniforms.testMatrix2x2.c[1].r);
}
