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
    var rgb: array<f32, 3>;
    var a: f32;
    rgb[0] = 0.0;
    rgb[1] = 1.0;
    rgb[2] = 0.0;
    a = 1.0;
    return vec4<f32>(rgb[0], rgb[1], rgb[2], a);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
