diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var x: mat2x2<f32> = mat2x2<f32>(0.0, 1.0, 2.0, 3.0);
    var y: vec2<f32> = vec2<f32>(vec4<f32>(x[0], x[1]).xy);
    return vec4<f32>(vec4<f32>(y, 0.0, 1.0));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
