diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  R_array: array<vec4<f32>, 5>,
  vector: vec2<f32>,
  matrix: mat2x2<f32>,
  idx: i32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain() -> vec4<f32> {
  {
    let _skTemp0 = _globalUniforms.idx + 1;
    let _skTemp1 = _globalUniforms.idx + 1;
    return vec4<f32>(f32(_globalUniforms.R_array[_globalUniforms.idx][_globalUniforms.idx]), f32(_globalUniforms.vector[_skTemp0]), f32(_globalUniforms.matrix[_globalUniforms.idx][_skTemp1]), 1.0);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain();
  return _stageOut;
}
