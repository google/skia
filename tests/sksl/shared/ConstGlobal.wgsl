diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
const SEVEN: i32 = 7;
const TEN: i32 = 10;
const MATRIXFIVE: mat4x4<f16> = mat4x4<f16>(5.0h, 0.0, 0.0, 0.0, 0.0, 5.0h, 0.0, 0.0, 0.0, 0.0, 5.0h, 0.0, 0.0, 0.0, 0.0, 5.0h);
fn verify_const_globals_biih44(seven: i32, ten: i32, matrixFive: mat4x4<f16>) -> bool {
  {
    const _skTemp0 = mat4x4<f16>(5.0h, 0.0, 0.0, 0.0, 0.0, 5.0h, 0.0, 0.0, 0.0, 0.0, 5.0h, 0.0, 0.0, 0.0, 0.0, 5.0h);
    return ((seven == 7) && (ten == 10)) && (all(matrixFive[0] == _skTemp0[0]) && all(matrixFive[1] == _skTemp0[1]) && all(matrixFive[2] == _skTemp0[2]) && all(matrixFive[3] == _skTemp0[3]));
  }
}
fn _skslMain(xy: vec2<f32>) -> vec4<f16> {
  {
    var _skTemp1: vec4<f16>;
    if verify_const_globals_biih44(SEVEN, TEN, MATRIXFIVE) {
      _skTemp1 = _globalUniforms.colorGreen;
    } else {
      _skTemp1 = _globalUniforms.colorRed;
    }
    return _skTemp1;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
