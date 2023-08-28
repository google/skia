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
    var x: f32;
    var y: f32;
    y = 1.0;
    x = y;
    var a: f32;
    var b: f32;
    var c: f32;
    c = 0.0;
    b = c;
    a = b;
    return vec4<f32>(a * b, f32(x), c, f32(y));
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
