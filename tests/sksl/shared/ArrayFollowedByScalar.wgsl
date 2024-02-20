diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
