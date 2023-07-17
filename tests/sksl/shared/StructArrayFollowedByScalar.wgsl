diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct S {
  rgb: array<f32, 3>,
  a: f32,
};
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
  {
    var s: S;
    s.rgb[0] = 0.0;
    s.rgb[1] = 1.0;
    s.rgb[2] = 0.0;
    s.a = 1.0;
    return vec4<f32>(s.rgb[0], s.rgb[1], s.rgb[2], s.a);
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
