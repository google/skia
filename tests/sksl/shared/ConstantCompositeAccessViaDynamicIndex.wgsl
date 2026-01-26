diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
var<private> zero: i32 = 0;
const globalArray: array<f16, 2> = array<f16, 2>(1.0h, 1.0h);
const globalVector: vec2<f16> = vec2<f16>(1.0h);
const globalMatrix: mat2x2<f16> = mat2x2<f16>(1.0h, 1.0h, 1.0h, 1.0h);
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f16> {
  {
    const localArray: array<f16, 2> = array<f16, 2>(0.0h, 1.0h);
    const localVector: vec2<f16> = vec2<f16>(1.0h);
    const localMatrix: mat2x2<f16> = mat2x2<f16>(0.0h, 1.0h, 2.0h, 3.0h);
    return vec4<f16>(globalArray[zero] * localArray[zero], globalVector[zero] * localVector[zero], globalMatrix[zero] * localMatrix[zero]);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
