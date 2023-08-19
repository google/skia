diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
var<private> zero: i32 = 0;
const globalArray: array<f32, 2> = array<f32, 2>(1.0, 1.0);
const globalVector: vec2<f32> = vec2<f32>(1.0);
const globalMatrix: mat2x2<f32> = mat2x2<f32>(1.0, 1.0, 1.0, 1.0);
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    const localArray: array<f32, 2> = array<f32, 2>(0.0, 1.0);
    const localVector: vec2<f32> = vec2<f32>(1.0);
    const localMatrix: mat2x2<f32> = mat2x2<f32>(0.0, 1.0, 2.0, 3.0);
    return vec4<f32>(globalArray[zero] * localArray[zero], globalVector[zero] * localVector[zero], globalMatrix[zero] * localMatrix[zero]);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
