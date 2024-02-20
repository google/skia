diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
