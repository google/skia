diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var rgb: array<f16, 3>;
    var a: f16;
    rgb[0] = 0.0h;
    rgb[1] = 1.0h;
    rgb[2] = 0.0h;
    a = 1.0h;
    return vec4<f16>(rgb[0], rgb[1], rgb[2], a);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
