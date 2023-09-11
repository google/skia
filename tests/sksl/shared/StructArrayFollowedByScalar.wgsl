diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct S {
  rgb: array<f32, 3>,
  a: f32,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var s: S;
    s.rgb[0] = 0.0;
    s.rgb[1] = 1.0;
    s.rgb[2] = 0.0;
    s.a = 1.0;
    return vec4<f32>(s.rgb[0], s.rgb[1], s.rgb[2], s.a);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
