diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct S {
  rgb: array<f16, 3>,
  a: f16,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var s: S;
    s.rgb[0] = 0.0h;
    s.rgb[1] = 1.0h;
    s.rgb[2] = 0.0h;
    s.a = 1.0h;
    return vec4<f16>(s.rgb[0], s.rgb[1], s.rgb[2], s.a);
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
