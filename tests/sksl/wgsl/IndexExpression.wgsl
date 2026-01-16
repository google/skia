diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  R_array: array<vec4<f32>, 5>,
  vector: vec2<f32>,
  matrix: _skMatrix22,
  idx: i32,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain() -> vec4<f32> {
  {
    let _skTemp0 = _globalUniforms.idx + 1;
    let _skTemp1 = _globalUniforms.idx + 1;
    return vec4<f32>(f32(_globalUniforms.R_array[_globalUniforms.idx][_globalUniforms.idx]), f32(_globalUniforms.vector[_skTemp0]), f32(_skUnpacked__globalUniforms_matrix[_globalUniforms.idx][_skTemp1]), 1.0);
  }
}
@fragment fn main() -> FSOut {
  _skInitializePolyfilledUniforms();
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain();
  return _stageOut;
}
struct _skRow2 {
  @align(16) r : vec2<f32>
};
struct _skMatrix22 {
  c : array<_skRow2, 2>
};
var<private> _skUnpacked__globalUniforms_matrix: mat2x2<f32>;
fn _skInitializePolyfilledUniforms() {
  _skUnpacked__globalUniforms_matrix = mat2x2<f32>(_globalUniforms.matrix.c[0].r, _globalUniforms.matrix.c[1].r);
}
