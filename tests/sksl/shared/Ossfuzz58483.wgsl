diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  var p = _skParam0;
  {
    p = p * 0.333333343;
    return vec4<f32>(1.0);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
