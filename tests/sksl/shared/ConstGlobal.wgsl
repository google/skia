diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
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
fn verify_const_globals_biih44(_skParam0: i32, _skParam1: i32, _skParam2: mat4x4<f32>) -> bool {
  let seven = _skParam0;
  let ten = _skParam1;
  let matrixFive = _skParam2;
  {
    let _skTemp0 = mat4x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0);
    return ((seven == 7) && (ten == 10)) && (all(matrixFive[0] == _skTemp0[0]) && all(matrixFive[1] == _skTemp0[1]) && all(matrixFive[2] == _skTemp0[2]) && all(matrixFive[3] == _skTemp0[3]));
  }
}
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let xy = _skParam0;
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
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
