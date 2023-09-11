diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
const SEVEN: i32 = 7;
const TEN: i32 = 10;
const MATRIXFIVE: mat4x4<f32> = mat4x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0);
fn verify_const_globals_biih44(seven: i32, ten: i32, matrixFive: mat4x4<f32>) -> bool {
  {
    let _skTemp0 = mat4x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0);
    return ((seven == 7) && (ten == 10)) && (all(matrixFive[0] == _skTemp0[0]) && all(matrixFive[1] == _skTemp0[1]) && all(matrixFive[2] == _skTemp0[2]) && all(matrixFive[3] == _skTemp0[3]));
  }
}
fn _skslMain(xy: vec2<f32>) -> vec4<f32> {
  {
    var _skTemp1: vec4<f32>;
    let _skTemp2 = verify_const_globals_biih44(SEVEN, TEN, MATRIXFIVE);
    if _skTemp2 {
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
